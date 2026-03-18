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

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>

#include <DSP/Filters/CompressorFilter.h>
#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    // dB-to-linear: 10^(x/20) = exp(ln(10)/20 * x)
    static inline AmReal32 dBToLinear(AmReal32 x)
    {
        return std::exp(2.302585092994f * x);
    }

    // linear-to-dB: 20 * log10(x) = 20 * log2(x) / log2(10)
    static inline AmReal32 linearToDb(AmReal32 x)
    {
        return std::log2(x) * (20.0f * 0.3010299956639812f);
    }

    CompressorFilter::CompressorFilter()
        : Filter("CompressorFilter")
        , _threshold(-20.0f)
        , _ratio(4.0f)
        , _attack(5.0f)
        , _release(200.0f)
        , _knee(0.0f)
        , _makeupGain(0.0f)
    {
    }

    AmResult CompressorFilter::Initialize(
        AmReal32 threshold, AmReal32 ratio, AmReal32 attack, AmReal32 release, AmReal32 knee, AmReal32 makeupGain)
    {
        _threshold = std::clamp(threshold, -60.0f, 0.0f);
        _ratio = std::clamp(ratio, 1.0f, 20.0f);
        _attack = std::clamp(attack, 0.1f, 100.0f);
        _release = std::clamp(release, 10.0f, 1000.0f);
        _knee = std::clamp(knee, 0.0f, 12.0f);
        _makeupGain = std::clamp(makeupGain, 0.0f, 24.0f);

        return eErrorCode_Success;
    }

    AmUInt32 CompressorFilter::GetParameterCount() const
    {
        return ATTRIBUTE_LAST;
    }

    AmString CompressorFilter::GetParameterName(AmUInt32 index) const
    {
        if (index >= ATTRIBUTE_LAST)
            return "";

        static const AmString names[ATTRIBUTE_LAST] = { "Wet", "Threshold", "Ratio", "Attack", "Release", "Knee", "MakeupGain" };
        return names[index];
    }

    eParameterType CompressorFilter::GetParameterType(AmUInt32 index) const
    {
        return eParameterType_Float;
    }

    AmReal32 CompressorFilter::GetParameterMax(AmUInt32 index) const
    {
        if (index >= ATTRIBUTE_LAST)
            return 0.0f;

        static const AmReal32 maxValues[ATTRIBUTE_LAST] = { 1.0f, 0.0f, 20.0f, 100.0f, 1000.0f, 12.0f, 24.0f };
        return maxValues[index];
    }

    AmReal32 CompressorFilter::GetParameterMin(AmUInt32 index) const
    {
        if (index >= ATTRIBUTE_LAST)
            return 0.0f;

        static const AmReal32 minValues[ATTRIBUTE_LAST] = { 0.0f, -60.0f, 1.0f, 0.1f, 10.0f, 0.0f, 0.0f };
        return minValues[index];
    }

    std::shared_ptr<FilterInstance> CompressorFilter::CreateInstance()
    {
        return ampoolshared(eMemoryPoolKind_Filtering, CompressorFilterInstance, this);
    }

    CompressorFilterInstance::CompressorFilterInstance(CompressorFilter* parent)
        : FilterInstance(parent)
        , _lastSampleRate(0)
        , _parameterVersion(1)
        , _lastParameterVersion(0)
        , _attackCoeff(0.0f)
        , _releaseCoeff(0.0f)
        , _levelLpCoeff(0.0f)
    {
        Initialize(parent->GetParameterCount());

        for (AmUInt16 i = 0; i < kAmMaxSupportedChannelCount; ++i)
        {
            _prevEnvelopeLinear[i] = 1e-4f;
            _prevGainDb[i] = 0.0f;
        }

        m_parameters[CompressorFilter::ATTRIBUTE_WET] = 1.0f;
        m_parameters[CompressorFilter::ATTRIBUTE_THRESHOLD] = parent->_threshold;
        m_parameters[CompressorFilter::ATTRIBUTE_RATIO] = parent->_ratio;
        m_parameters[CompressorFilter::ATTRIBUTE_ATTACK] = parent->_attack;
        m_parameters[CompressorFilter::ATTRIBUTE_RELEASE] = parent->_release;
        m_parameters[CompressorFilter::ATTRIBUTE_KNEE] = parent->_knee;
        m_parameters[CompressorFilter::ATTRIBUTE_MAKEUP_GAIN] = parent->_makeupGain;
    }

    void CompressorFilterInstance::SetParameter(AmUInt32 index, AmReal32 value)
    {
        static const AmReal32 minValues[CompressorFilter::ATTRIBUTE_LAST] = { 0.0f, -60.0f, 1.0f, 0.1f, 10.0f, 0.0f, 0.0f };
        static const AmReal32 maxValues[CompressorFilter::ATTRIBUTE_LAST] = { 1.0f, 0.0f, 20.0f, 100.0f, 1000.0f, 12.0f, 24.0f };

        if (index >= CompressorFilter::ATTRIBUTE_LAST)
            return;

        value = std::clamp(value, minValues[index], maxValues[index]);
        FilterInstance::SetParameter(index, value);

        // Invalidate cached coefficients when timing parameters change
        if (index == CompressorFilter::ATTRIBUTE_ATTACK || index == CompressorFilter::ATTRIBUTE_RELEASE)
            ++_parameterVersion;
    }

    void CompressorFilterInstance::UpdateCoefficients(AmUInt32 sampleRate)
    {
        if (_lastSampleRate == sampleRate && _lastParameterVersion == _parameterVersion)
            return;

        _lastSampleRate = sampleRate;
        _lastParameterVersion = _parameterVersion;

        const AmReal32 attackSec = m_parameters[CompressorFilter::ATTRIBUTE_ATTACK] / 1000.0f;
        const AmReal32 releaseSec = m_parameters[CompressorFilter::ATTRIBUTE_RELEASE] / 1000.0f;

        AmReal32 levelTimeConst = std::min(attackSec, releaseSec) / 5.0f;
        levelTimeConst = std::max(levelTimeConst, 0.002f);

        _levelLpCoeff = std::exp(-1.0f / (levelTimeConst * static_cast<AmReal32>(sampleRate)));
        _attackCoeff = std::exp(-1.0f / (attackSec * static_cast<AmReal32>(sampleRate)));
        _releaseCoeff = std::exp(-1.0f / (releaseSec * static_cast<AmReal32>(sampleRate)));
    }

    // Compute gain reduction in dB using the standard soft-knee formula from
    // Giannoulis, Massberg, Reiss - "Digital Dynamic Range Compressor Design" (JAES 2012).
    AmReal32 CompressorFilterInstance::ComputeGainReduction(AmReal32 levelDb) const
    {
        const AmReal32 threshold = m_parameters[CompressorFilter::ATTRIBUTE_THRESHOLD];
        const AmReal32 ratio = m_parameters[CompressorFilter::ATTRIBUTE_RATIO];
        const AmReal32 knee = m_parameters[CompressorFilter::ATTRIBUTE_KNEE];

        const AmReal32 slope = (1.0f / ratio) - 1.0f;
        const AmReal32 kneeHalf = knee / 2.0f;
        const AmReal32 overshoot = levelDb - threshold;

        if (overshoot <= -kneeHalf)
        {
            return 0.0f;
        }

        if (overshoot < kneeHalf && knee > 0.0f)
        {
            const AmReal32 x = overshoot + kneeHalf;
            return 0.5f * slope * x * x / knee;
        }

        return slope * overshoot;
    }

    AmAudioSample CompressorFilterInstance::ProcessSample(AmAudioSample sample, AmUInt16 channel, AmUInt32 sampleRate)
    {
        UpdateCoefficients(sampleRate);

        const AmReal32 inputLevel = std::abs(sample);

        AmReal32 envelope = _levelLpCoeff * _prevEnvelopeLinear[channel] + (1.0f - _levelLpCoeff) * inputLevel;
        envelope = std::max(envelope, 1e-13f);
        _prevEnvelopeLinear[channel] = envelope;

        const AmReal32 levelDb = linearToDb(envelope);

        const AmReal32 targetGainDb = ComputeGainReduction(levelDb);

        AmReal32 smoothedGainDb;
        if (targetGainDb < _prevGainDb[channel])
        {
            smoothedGainDb = _attackCoeff * _prevGainDb[channel] + (1.0f - _attackCoeff) * targetGainDb;
        }
        else
        {
            smoothedGainDb = _releaseCoeff * _prevGainDb[channel] + (1.0f - _releaseCoeff) * targetGainDb;
        }
        _prevGainDb[channel] = smoothedGainDb;

        const AmReal32 makeupGainDb = m_parameters[CompressorFilter::ATTRIBUTE_MAKEUP_GAIN];
        const AmReal32 totalGainDb = smoothedGainDb + makeupGainDb;
        const AmReal32 gainLinear = dBToLinear(totalGainDb / 20.0f);

        const AmReal32 output = sample * gainLinear;

        // Wet/dry mix implements parallel compression (a.k.a. "New York compression"):
        // wet=1.0 is fully compressed, wet=0.0 is fully dry, intermediate values
        // blend compressed and uncompressed signals.
        const AmReal32 wet = m_parameters[CompressorFilter::ATTRIBUTE_WET];
        return static_cast<AmAudioSample>(sample + (output - sample) * wet);
    }
} // namespace SparkyStudios::Audio::Amplitude
