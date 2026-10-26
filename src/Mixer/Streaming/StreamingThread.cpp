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

#include <algorithm>
#include <chrono>

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>
#include <SparkyStudios/Audio/Amplitude/IO/Log.h>

#include <Mixer/Streaming/StreamingThread.h>
#include <Mixer/Voice/VoiceTypes.h>

namespace SparkyStudios::Audio::Amplitude
{
    namespace
    {
        constexpr AmReal64 kRateSmoothing = 0.25;
        constexpr AmReal64 kMinRateFactor = 0.25;
        constexpr AmReal64 kMinRateWindow = 0.001;
        constexpr AmUInt32 kIdleWaitMs = 5;
        constexpr AmUInt32 kInitialSources = 64;

        AmInt64 NowTicks()
        {
            return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
        }
    } // namespace

    StreamingThread::StreamingThread(const StreamingSettings& settings, ThreadFactory createThread)
        : _settings(settings)
        , _createThread(createThread)
    {
        _settings.chunkFrames = std::max<AmUInt64>(_settings.chunkFrames, 1);
        _sources.reserve(kInitialSources);
        _retireRequests.reserve(kInitialSources);
        _acknowledged.reserve(kInitialSources);
        _active.reserve(kInitialSources);
        _candidates.reserve(kInitialSources);
    }

    StreamingThread::~StreamingThread()
    {
        Stop();
        CollectRetired();

        std::vector<Entry> remaining;
        {
            std::lock_guard lock(_mutex);
            remaining.swap(_sources);
        }

        for (const auto& entry : remaining)
            Free(entry.source);
    }

    void StreamingThread::Free(StreamSource* source)
    {
        ampooldelete(eMemoryPoolKind_Amplimix, StreamSource, source);
    }

    void StreamingThread::Start()
    {
        if (_settings.synchronous || _running.load(std::memory_order_acquire))
            return;

        {
            // The previous producer, if any, is gone: let the new worker take the sources over.
            std::lock_guard lock(_mutex);
            for (const auto& entry : _sources)
                entry.source->ReleaseProducer();
            _wakePending = false;
        }

        _lastPassTicks = NowTicks();
        _running.store(true, std::memory_order_release);
        _thread = _createThread != nullptr ? _createThread(&StreamingThread::Run, this) : nullptr;
        if (_thread == nullptr)
        {
            // Without a worker nothing would refill the streams past their prefill: their consumers fill them instead.
            _running.store(false, std::memory_order_release);
            _settings.synchronous = true;
            amLogError("Cannot start the streaming thread: streamed sounds are decoded on the audio thread instead.");
        }
    }

    void StreamingThread::Stop()
    {
        if (_thread == nullptr)
        {
            _running.store(false, std::memory_order_release);
            return;
        }

        {
            std::lock_guard lock(_mutex);
            _stopping = true;
            _running.store(false, std::memory_order_release);
        }
        _wake.notify_all();

        // Blocks until the worker leaves the decoder call it may be in.
        Thread::Wait(_thread);

        // The worker is gone: nothing can reach the pending requests, so they are acknowledged now.
        std::lock_guard lock(_mutex);
        _stopping = false;
        _acknowledged.insert(_acknowledged.end(), _retireRequests.begin(), _retireRequests.end());
        _retireRequests.clear();
    }

    bool StreamingThread::IsRunning() const
    {
        return _running.load(std::memory_order_acquire);
    }

    StreamSource* StreamingThread::Create(
        std::shared_ptr<Codec::Decoder> decoder, const StreamPlan& plan, AmUInt16 channels, AmUInt32 sampleRate, AmUInt64 minCapacity)
    {
        return Make(std::move(decoder), nullptr, plan, channels, sampleRate, minCapacity);
    }

    StreamSource* StreamingThread::CreateDeferred(
        StreamDecoderOpener opener, const StreamPlan& plan, AmUInt16 channels, AmUInt32 sampleRate, AmUInt64 minCapacity)
    {
        return Make(nullptr, std::move(opener), plan, channels, sampleRate, minCapacity);
    }

    StreamSource* StreamingThread::Make(
        std::shared_ptr<Codec::Decoder> decoder,
        StreamDecoderOpener opener,
        const StreamPlan& plan,
        AmUInt16 channels,
        AmUInt32 sampleRate,
        AmUInt64 minCapacity)
    {
        const AmUInt64 prefill = GetPrefillFrames(sampleRate);
        const AmUInt64 configured = std::max<AmUInt64>(MillisecondsToFrames(_settings.bufferMs, sampleRate), 2 * prefill);

        // A ring smaller than one consumer block underruns on every block, whatever the producer does.
        if (configured < minCapacity && !_warnedSmallBuffer)
        {
            _warnedSmallBuffer = true;
            amLogWarning(
                "The streaming buffer (%llu frames) is smaller than one mixer block reads (%llu frames): it is enlarged. Raise "
                "streaming.buffer_ms to silence this warning.",
                static_cast<unsigned long long>(configured), static_cast<unsigned long long>(minCapacity));
        }

        const AmUInt64 capacity = std::max<AmUInt64>({ configured, minCapacity, 1 });

        auto* source = ampoolnew(
            eMemoryPoolKind_Amplimix, StreamSource, std::move(decoder), plan, capacity, channels, _settings.chunkFrames, sampleRate,
            _settings.synchronous);

        if (opener)
            source->DeferDecoder(std::move(opener));

        if (_settings.synchronous || !IsRunning())
        {
            // Nobody else touches the source yet: fill it on this thread, then hand the producer role over.
            source->Fill(prefill);
            if (!_settings.synchronous)
                source->ReleaseProducer();
        }

        {
            std::lock_guard lock(_mutex);
            Entry entry;
            entry.source = source;
            entry.lastRelease = source->GetRing().GetReleaseIndex();
            entry.rate = static_cast<AmReal64>(sampleRate);
            _sources.push_back(entry);
            _wakePending = true;
        }
        _wake.notify_one();

        return source;
    }

    bool StreamingThread::IsReady(const StreamSource* source) const
    {
        if (source == nullptr)
            return false;

        return source->IsFilled() || source->GetBufferedFrames() >= GetPrefillFrames(source->GetSampleRate());
    }

    void StreamingThread::Retire(StreamSource* source)
    {
        if (source == nullptr)
            return;

        {
            std::lock_guard lock(_mutex);
            const auto it = std::find_if(
                _sources.begin(), _sources.end(),
                [source](const Entry& e)
                {
                    return e.source == source;
                });
            if (it == _sources.end())
                return; // not registered, or already retired: never freed twice

            _sources.erase(it);

            // While Stop() is joining, the worker may still be filling the source: queue it for the acknowledgment.
            if (_stopping || _running.load(std::memory_order_acquire))
            {
                _retireRequests.push_back(source);
                _wake.notify_one();
                return;
            }
        }

        // The worker is not running: nothing else can reach the source.
        Free(source);
    }

    void StreamingThread::CollectRetired()
    {
        std::vector<StreamSource*> freed;
        {
            std::lock_guard lock(_mutex);
            freed.swap(_acknowledged);
        }

        for (auto* source : freed)
            Free(source);
    }

    void StreamingThread::FillInline(StreamSource* source, AmUInt64 frames)
    {
        AMPLITUDE_ASSERT(_settings.synchronous);
        if (source != nullptr)
            source->Fill(frames);
    }

    bool StreamingThread::IsSynchronous() const
    {
        return _settings.synchronous;
    }

    AmUInt64 StreamingThread::GetPrefillFrames(AmUInt32 sampleRate) const
    {
        return std::max<AmUInt64>(MillisecondsToFrames(_settings.prefillMs, sampleRate), 1);
    }

    void StreamingThread::RecordUnderrun()
    {
        _underruns.fetch_add(1, std::memory_order_relaxed);
    }

    AmUInt64 StreamingThread::GetUnderrunCount() const
    {
        return _underruns.load(std::memory_order_relaxed);
    }

    void StreamingThread::WakeUp()
    {
        {
            std::lock_guard lock(_mutex);
            _wakePending = true;
        }
        _wake.notify_one();
    }

    void StreamingThread::Run(AmVoidPtr param)
    {
        auto* self = static_cast<StreamingThread*>(param);
        AM_UNUSED(Thread::SetCurrentThreadPriority(eThreadPriority::AboveNormal));

        while (self->_running.load(std::memory_order_acquire))
        {
            self->Pass();

            std::unique_lock lock(self->_mutex);
            self->_wake.wait_for(
                lock, std::chrono::milliseconds(kIdleWaitMs),
                [self]()
                {
                    return !self->_running.load(std::memory_order_acquire) || self->_wakePending || !self->_retireRequests.empty();
                });
            self->_wakePending = false;
        }
    }

    bool StreamingThread::UpdateRates(std::vector<Entry>& entries, AmReal64 elapsedSeconds)
    {
        if (elapsedSeconds < kMinRateWindow)
            return false;

        for (auto& entry : entries)
        {
            const AmUInt64 release = entry.source->GetRing().GetReleaseIndex();
            if (release >= entry.lastRelease)
            {
                const AmReal64 instant = static_cast<AmReal64>(release - entry.lastRelease) / elapsedSeconds;
                entry.rate += kRateSmoothing * (instant - entry.rate);
            }
            entry.lastRelease = release;
        }

        return true;
    }

    bool StreamingThread::HasPendingWork()
    {
        // A wake covers new, unsuspended and woken sources; a retire request must be acknowledged promptly.
        std::lock_guard lock(_mutex);
        return _wakePending || !_retireRequests.empty();
    }

    void StreamingThread::Pass()
    {
        // 1. Acknowledge the retire requests and take a snapshot of the live sources. After this point the worker no
        // longer fills an acknowledged source, so the game thread may free it.
        {
            std::lock_guard lock(_mutex);
            _acknowledged.insert(_acknowledged.end(), _retireRequests.begin(), _retireRequests.end());
            _retireRequests.clear();
            _active = _sources;
            _wakePending = false; // everything signalled so far is covered by the snapshot
        }

        if (_active.empty())
        {
            _lastPassTicks = NowTicks();
            return;
        }

        // 2. Estimate how long each source's buffered audio lasts.
        const AmInt64 now = NowTicks();
        if (UpdateRates(_active, static_cast<AmReal64>(now - _lastPassTicks) / 1.0e6))
            _lastPassTicks = now;

        _candidates.clear();
        AmUInt64 maxCapacity = 0;
        for (const auto& entry : _active)
        {
            StreamSource* source = entry.source;
            if (source->IsSuspended() || source->IsFilled())
                continue;

            const AmReal64 rate = std::max(entry.rate, kMinRateFactor * static_cast<AmReal64>(source->GetSampleRate()));
            _candidates.push_back({ source, static_cast<AmReal64>(source->GetBufferedFrames()) / rate });
            maxCapacity = std::max(maxCapacity, source->GetRing().GetCapacity());
        }

        // Write the rates back; the entries may have been retired meanwhile, in which case nothing matches.
        {
            std::lock_guard lock(_mutex);
            for (const auto& updated : _active)
            {
                const auto it = std::find_if(
                    _sources.begin(), _sources.end(),
                    [&](const Entry& e)
                    {
                        return e.source == updated.source;
                    });
                if (it != _sources.end())
                {
                    it->lastRelease = updated.lastRelease;
                    it->rate = updated.rate;
                }
            }
        }

        // 3. Most starved first; one chunk per source per round so a full-speed stream cannot starve the others.
        std::stable_sort(
            _candidates.begin(), _candidates.end(),
            [](const Candidate& a, const Candidate& b)
            {
                return a.timeLeft < b.timeLeft;
            });

        const AmUInt64 maxRounds = maxCapacity / _settings.chunkFrames + 1;
        for (AmUInt64 round = 0; round < maxRounds && !_candidates.empty(); ++round)
        {
            bool wroteAny = false;

            for (auto it = _candidates.begin(); it != _candidates.end();)
            {
                // Between two fills: end the pass when new work arrived, the next one re-snapshots and re-prioritizes.
                // A source retired meanwhile is acknowledged by that next pass, so it is never filled after it.
                if (!_running.load(std::memory_order_acquire) || HasPendingWork())
                    return;

                const AmUInt64 written = it->source->Fill(_settings.chunkFrames);
                if (written == 0 || it->source->IsFilled())
                    it = _candidates.erase(it);
                else
                    ++it;

                wroteAny = wroteAny || written > 0;
            }

            if (!wroteAny)
                break;
        }
    }
} // namespace SparkyStudios::Audio::Amplitude
