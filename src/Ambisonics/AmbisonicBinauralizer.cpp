// Copyright (c) 2024-present Sparky Studios. All rights reserved.
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

#include <Ambisonics/AmbisonicBinauralizer.h>

namespace SparkyStudios::Audio::Amplitude
{
    bool AmbisonicBinauralizer::Configure(
        AmUInt32 order, bool is3D, AmUInt32 maxBlockSize, AmUInt32 sampleRate, const HRIRSphere* hrirSphere)
    {
        if (hrirSphere == nullptr)
            return false;

        return Configure(order, is3D, maxBlockSize, sampleRate, BinauralDecodingFilters::Build(order, is3D, *hrirSphere));
    }

    bool AmbisonicBinauralizer::Configure(
        AmUInt32 order, bool is3D, AmUInt32 maxBlockSize, AmUInt32 sampleRate, std::shared_ptr<const BinauralDecodingFilters> filters)
    {
        if (filters == nullptr || filters->GetOrder() != order || filters->Is3D() != is3D)
            return false;

        if (!AmbisonicComponent::Configure(order, is3D))
            return false;

        if (!_shelfFilter.Configure(order, is3D, maxBlockSize, sampleRate))
            return false;

        if (!_convolver.Init(filters->GetKernels(0), filters->GetKernels(1)))
            return false;

        _filters = std::move(filters);
        return true;
    }

    void AmbisonicBinauralizer::Reset()
    {
        _shelfFilter.Reset();
    }

    void AmbisonicBinauralizer::Refresh()
    {}

    void AmbisonicBinauralizer::Process(BFormat* input, AmUInt32 samples, AudioBuffer& output)
    {
        _shelfFilter.Process(input, samples);

        if (_filters == nullptr)
            return;

        const AmAudioSample* inputs[SummingStereoConvolver::kMaxInputs];
        for (AmUInt32 c = 0; c < m_channelCount; ++c)
            inputs[c] = input->GetBufferChannel(c).begin();

        _convolver.Process(inputs, output[0].begin(), output[1].begin(), samples);
    }

    const std::shared_ptr<const BinauralDecodingFilters>& AmbisonicBinauralizer::GetFilters() const
    {
        return _filters;
    }
} // namespace SparkyStudios::Audio::Amplitude
