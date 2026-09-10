// Copyright (c) 2021-present Sparky Studios. All rights reserved.
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

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>

#include <DSP/Filters/DelayFilter.h>
#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    DelayFilter::DelayFilter()
        : Filter("Delay")
        , _delay(0.3f)
        , _decay(0.7f)
        , _delayStart(0.0f)
    {}

    AmResult DelayFilter::Initialize(AmReal32 delay, AmReal32 decay, AmReal32 delayStart)
    {
        if (delay <= 0 || decay <= 0)
            return eErrorCode_InvalidParameter;

        _delay = delay;
        _decay = std::clamp(decay, 0.0f, 0.999f);
        _delayStart = delayStart;

        return eErrorCode_Success;
    }

    AmUInt32 DelayFilter::GetParameterCount() const
    {
        return ATTRIBUTE_LAST;
    }

    AmString DelayFilter::GetParameterName(AmUInt32 index) const
    {
        if (index >= ATTRIBUTE_LAST)
            return "Unknown";

        static const AmString names[ATTRIBUTE_LAST] = { "Wet", "Delay", "Decay", "Delay Start" };

        return names[index];
    }

    eParameterType DelayFilter::GetParameterType(AmUInt32 index) const
    {
        if (index == ATTRIBUTE_DELAY_START)
            return eParameterType_Bool;

        return eParameterType_Float;
    }

    AmReal32 DelayFilter::GetParameterMax(AmUInt32 index) const
    {
        if (index >= ATTRIBUTE_LAST)
            return 0.0f;

        static const AmReal32 values[ATTRIBUTE_LAST] = { 1.0f, 1.0f, 1.0f, 1.0f };

        return values[index];
    }

    AmReal32 DelayFilter::GetParameterMin(AmUInt32 index) const
    {
        if (index >= ATTRIBUTE_LAST)
            return 0.0f;

        static const AmReal32 values[ATTRIBUTE_LAST] = { 0.0f, 0.0f, 0.0f, 0.0f };

        return values[index];
    }

    std::shared_ptr<FilterInstance> DelayFilter::CreateInstance()
    {
        return ampoolshared(eMemoryPoolKind_Filtering, DelayFilterInstance, this);
    }

    DelayFilterInstance::DelayFilterInstance(DelayFilter* parent)
        : FilterInstance(parent)
    {
        _buffer = nullptr;
        _bufferLength = 0;
        _bufferOffset = 0;
        _bufferMaxLength = 0;
        _offset = 0;

        Initialize(parent->GetParameterCount());

        m_parameters[DelayFilter::ATTRIBUTE_DELAY] = parent->_delay;
        m_parameters[DelayFilter::ATTRIBUTE_DECAY] = std::clamp(parent->_decay, 0.0f, 0.999f);
        m_parameters[DelayFilter::ATTRIBUTE_DELAY_START] = parent->_delayStart;
    }

    DelayFilterInstance::~DelayFilterInstance()
    {
        if (_buffer == nullptr)
            return;

        ampoolfree(eMemoryPoolKind_Filtering, _buffer);
    }

    void DelayFilterInstance::SetParameter(AmUInt32 parameterIndex, AmReal32 value)
    {
        if (parameterIndex == DelayFilter::ATTRIBUTE_DECAY)
            value = std::clamp(value, 0.0f, 0.999f);

        FilterInstance::SetParameter(parameterIndex, value);
    }

    void DelayFilterInstance::Process(const AudioBuffer& in, AudioBuffer& out, AmUInt64 frames, AmUInt32 sampleRate)
    {
        const AmUInt16 channels = in.GetChannelCount();

        InitializeBuffer(channels, sampleRate);

        if (m_parameters[DelayFilter::ATTRIBUTE_DELAY] < (1.0f / static_cast<AmReal32>(sampleRate)))
        {
            for (AmUInt16 c = 0; c < channels; c++)
                for (AmUInt64 f = 0; f < frames; f++)
                    out[c][f] = in[c][f];
            return;
        }

        const AmUInt32 startOffset = _offset;

        for (AmUInt16 c = 0; c < channels; c++)
        {
            const auto& inChannel = in[c];
            auto& outChannel = out[c];

            AmUInt32 offset = startOffset;

            for (AmUInt64 f = 0; f < frames; f++)
            {
                _bufferOffset = c * _bufferLength + offset;

                outChannel[f] = ProcessSample(inChannel[f], c, sampleRate);
                offset = (offset + 1) % _bufferLength;
            }
        }

        _offset = (startOffset + static_cast<AmUInt32>(frames)) % _bufferLength;
    }

    AmAudioSample DelayFilterInstance::ProcessSample(AmAudioSample sample, AmUInt16 channel, AmUInt32 sampleRate)
    {
        const AmUInt32 o = _bufferOffset;

        const AmReal32 x = sample;
        /* */ AmReal32 y;

        if (m_parameters[DelayFilter::ATTRIBUTE_DELAY_START] != 0.0f)
        {
            // Read first
            y = _buffer[o] * m_parameters[DelayFilter::ATTRIBUTE_WET];

            // Produce feedback
            _buffer[o] = _buffer[o] * m_parameters[DelayFilter::ATTRIBUTE_DECAY] + x;
        }
        else
        {
            // Produce feedback first
            _buffer[o] = _buffer[o] * m_parameters[DelayFilter::ATTRIBUTE_DECAY] + x;

            // Read
            y = _buffer[o] * m_parameters[DelayFilter::ATTRIBUTE_WET];
        }

        return static_cast<AmAudioSample>(y);
    }

    void DelayFilterInstance::InitializeBuffer(AmUInt16 channels, AmUInt32 sampleRate)
    {
        const auto maxSamples =
            std::max(1u, static_cast<AmUInt32>(std::ceil(m_parameters[DelayFilter::ATTRIBUTE_DELAY] * static_cast<AmReal32>(sampleRate))));

        if (_buffer == nullptr || maxSamples > _bufferMaxLength)
        {
            if (_buffer != nullptr)
                ampoolfree(eMemoryPoolKind_Filtering, _buffer);

            _offset = 0;
            _bufferOffset = 0;

            _bufferMaxLength = maxSamples;
            const AmUInt32 size = _bufferMaxLength * channels * sizeof(AmReal32);

            _buffer = static_cast<AmReal32Buffer>(ampoolmalloc(eMemoryPoolKind_Filtering, size));
            std::memset(_buffer, 0, size);
        }

        _bufferLength = maxSamples;
    }

} // namespace SparkyStudios::Audio::Amplitude
