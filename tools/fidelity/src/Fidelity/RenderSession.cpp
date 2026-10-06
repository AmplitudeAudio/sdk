// Copyright (c) 2026-present Sparky Studios. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <Fidelity/RenderSession.h>

#include <algorithm>
#include <cmath>
#include <exception>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>
#include <SparkyStudios/Audio/Amplitude/DSP/Resampler.h>
#include <SparkyStudios/Audio/Amplitude/IO/DiskFileSystem.h>

#include <Utils/ScopedDenormalFlush.h>

#include <Fidelity/OfflineDriver.h>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    namespace
    {
        /// Registered as "default" so the engine picks it up whatever its config names; forwards to another resampler.
        class ResamplerAlias final : public Resampler
        {
        public:
            explicit ResamplerAlias(std::string target)
                : Resampler("default")
                , _target(std::move(target))
            {}

            std::shared_ptr<ResamplerInstance> CreateInstance() override
            {
                return Resampler::Construct(_target);
            }

        private:
            std::string _target;
        };

        RenderOutcome RunLockStep(const RenderSettings& settings, std::vector<TimedAction> actions)
        {
            RenderOutcome outcome;

            amEngine->EnsureSoundBankLoaded(AM_OS_STRING("fidelity.ambank"));
            amEngine->StartLoadSoundFiles();
            while (!amEngine->TryFinalizeLoadSoundFiles())
                Thread::Sleep(1);

            AM_UNUSED(amEngine->AddListener(1));
            amEngine->SetDefaultListener(1);

            Amplimix* mixer = amEngine->GetMixer();
            const DeviceDescription& device = mixer->GetDeviceDescription();
            const std::uint32_t sampleRate = device.mRequestedOutputSampleRate;
            const auto channels = static_cast<std::size_t>(device.mRequestedOutputChannels);

            if (device.mOutputBufferSize != settings.blockSize * channels)
            {
                outcome.error = "config buffer_size " + std::to_string(device.mOutputBufferSize) + " does not match blockSize " +
                    std::to_string(settings.blockSize) + " x " + std::to_string(channels) + " channels";
                return outcome;
            }

            Capture& capture = outcome.capture;
            capture.sampleRate = sampleRate;
            capture.channels.assign(channels, {});
            for (auto& channel : capture.channels)
                channel.reserve(settings.durationSamples);

            std::stable_sort(
                actions.begin(), actions.end(),
                [](const TimedAction& a, const TimedAction& b)
                {
                    return a.sample < b.sample;
                });

            LockStepSchedule schedule(settings, sampleRate);
            std::size_t nextAction = 0;
            std::uint64_t captured = 0;

            while (captured < settings.durationSamples)
            {
                const ScheduleEvent event = schedule.Next();

                if (event.kind == MarkerKind::Frame)
                {
                    const auto frameSample = static_cast<std::uint64_t>(std::floor(event.sample));
                    capture.markers.push_back({ frameSample, MarkerKind::Frame, {} });

                    while (nextAction < actions.size() && static_cast<double>(actions[nextAction].sample) <= event.sample)
                    {
                        capture.markers.push_back({ frameSample, MarkerKind::Action, actions[nextAction].label });
                        actions[nextAction].run();
                        ++nextAction;
                    }

                    amEngine->AdvanceFrame(event.deltaMs);
                    continue;
                }

                capture.markers.push_back({ static_cast<std::uint64_t>(event.sample), MarkerKind::Block, {} });

                AudioBuffer* out = nullptr;
                AmUInt64 mixed = 0;
                {
                    ScopedDenormalFlush flush;
                    mixed = mixer->Mix(&out, event.frames);
                }

                const std::uint64_t keep = std::min<std::uint64_t>(event.frames, settings.durationSamples - captured);
                for (std::size_t c = 0; c < channels; ++c)
                {
                    for (std::uint64_t i = 0; i < keep; ++i)
                    {
                        const bool hasSample = out != nullptr && i < mixed;
                        capture.channels[c].push_back(hasSample ? out->GetChannel(c)[i] : 0.0f);
                    }
                }

                captured += keep;
            }

            return outcome;
        }
    } // namespace

    LockStepSchedule::LockStepSchedule(const RenderSettings& settings, std::uint32_t sampleRate)
        : _sampleRate(static_cast<double>(sampleRate))
        , _blockSize(settings.blockSize)
        , _sequence(settings.blockSequence)
        , _period(static_cast<double>(sampleRate) / settings.fps)
        , _jitter(settings.jitter)
        , _jitterAmount(settings.jitterAmount)
        , _random(settings.jitterSeed)
        , _lastFrame(-static_cast<double>(sampleRate) / settings.fps)
    {}

    ScheduleEvent LockStepSchedule::Next()
    {
        if (_nextFrame <= static_cast<double>(_nextBlock))
        {
            const ScheduleEvent event{ MarkerKind::Frame, _nextFrame, 0, (_nextFrame - _lastFrame) / _sampleRate * 1000.0 };
            _lastFrame = _nextFrame;
            _nextFrame += NextPeriod();
            return event;
        }

        const std::uint32_t frames = _sequence.empty() ? _blockSize : _sequence[_sequenceIndex++ % _sequence.size()];
        const ScheduleEvent event{ MarkerKind::Block, static_cast<double>(_nextBlock), frames, 0.0 };
        _nextBlock += frames;
        return event;
    }

    double LockStepSchedule::NextPeriod()
    {
        if (!_jitter)
            return _period;

        if (_hasPending)
        {
            _hasPending = false;
            return _period * (1.0 - _pending);
        }

        _pending = _jitterAmount * _random.Symmetric();
        _hasPending = true;
        return _period * (1.0 + _pending);
    }

    RenderOutcome Render(const std::filesystem::path& assets, const RenderSettings& settings, std::vector<TimedAction> actions)
    {
        const bool ownsMemory = !MemoryManager::IsInitialized();
        if (ownsMemory)
            MemoryManager::Initialize();

        auto fileSystem = ampoolshared(eMemoryPoolKind_IO, DiskFileSystem);
        fileSystem->SetBasePath(AmOsString(assets.native()));
        amEngine->SetFileSystem(fileSystem);
        amEngine->StartOpenFileSystem();
        while (!amEngine->TryFinalizeOpenFileSystem())
            Thread::Sleep(1);

        Engine::RegisterDefaultExtensions();
        Driver::Unregister(Driver::Find("miniaudio"));
        auto driver = Engine::RegisterExtension<OfflineDriver>();

        std::shared_ptr<ResamplerAlias> alias;
        if (!settings.resampler.empty() && settings.resampler != "default")
        {
            if (Resampler::Find(settings.resampler) == nullptr)
            {
                RenderOutcome failed;
                failed.error = "unknown resampler '" + settings.resampler + "'";
                Engine::UnregisterDefaultExtensions();
                Engine::UnregisterExtension(driver);
                amEngine->DestroyInstance();
                fileSystem.reset();
                if (ownsMemory)
                    MemoryManager::Deinitialize();

                return failed;
            }

            Resampler::Unregister(Resampler::Find("default"));
            alias = Engine::RegisterExtension<ResamplerAlias>(settings.resampler);
        }

        RenderOutcome outcome;
        if (amEngine->Initialize(AmOsString(std::filesystem::path(settings.configFile).native())))
        {
            // Whatever an action throws, the engine below must still be torn down: the next render needs a fresh one.
            try
            {
                outcome = RunLockStep(settings, std::move(actions));
            } catch (const std::exception& exception)
            {
                outcome = {};
                outcome.error = std::string("render aborted: ") + exception.what();
            } catch (...)
            {
                outcome = {};
                outcome.error = "render aborted: unknown exception";
            }
        }
        else
        {
            outcome.error = "engine initialization failed with config '" + settings.configFile + "'";
        }

        amEngine->Deinitialize();
        if (alias != nullptr)
            Engine::UnregisterExtension(alias);

        Engine::UnregisterDefaultExtensions();
        Engine::UnregisterExtension(driver);
        amEngine->DestroyInstance();
        fileSystem.reset();

        if (ownsMemory)
            MemoryManager::Deinitialize();

        return outcome;
    }
} // namespace SparkyStudios::Audio::Amplitude::Fidelity
