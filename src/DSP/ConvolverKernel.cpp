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
#include <SparkyStudios/Audio/Amplitude/DSP/FFT.h>
#include <SparkyStudios/Audio/Amplitude/Math/Utils.h>

#include <DSP/ConvolverKernel.h>
#include <Utils/Utils.h>

#include <algorithm>
#include <cmath>

namespace SparkyStudios::Audio::Amplitude
{
    std::shared_ptr<const ConvolverKernel> ConvolverKernel::Create(AmSize blockSize, const AmAudioSample* ir, AmSize irLen)
    {
        if (blockSize == 0)
            return nullptr;

        // Zeros at the end of the impulse response only waste computation time
        while (irLen > 0 && std::fabs(ir[irLen - 1]) < 0.000001f)
            --irLen;

        auto kernel = ampoolshared(eMemoryPoolKind_Filtering, ConvolverKernel);
        kernel->_blockSize = NextPowerOf2(blockSize);
        kernel->_segmentSize = 2 * kernel->_blockSize;
        kernel->_complexSize = FFT::GetOutputSize(kernel->_segmentSize);

        if (irLen == 0)
            return kernel;

        const AmSize segmentCount = (irLen + kernel->_blockSize - 1) / kernel->_blockSize;

        FFT fft;
        fft.Initialize(kernel->_segmentSize);

        AmAlignedReal32Buffer padded;
        padded.Resize(static_cast<AmUInt32>(kernel->_segmentSize));

        kernel->_segments.reserve(segmentCount);
        for (AmSize i = 0; i < segmentCount; ++i)
        {
            auto* segment = ampoolnew(eMemoryPoolKind_Filtering, SplitComplex, kernel->_complexSize);
            const AmSize remaining = irLen - i * kernel->_blockSize;
            CopyAndPad(padded, &ir[i * kernel->_blockSize], std::min(remaining, kernel->_blockSize));
            fft.Forward(padded.GetBuffer(), *segment);
            kernel->_segments.push_back(segment);
        }

        return kernel;
    }

    ConvolverKernel::~ConvolverKernel()
    {
        for (auto* segment : _segments)
            ampooldelete(eMemoryPoolKind_Filtering, SplitComplex, segment);

        _segments.clear();
    }

    AmSize ConvolverKernel::GetBlockSize() const
    {
        return _blockSize;
    }

    AmSize ConvolverKernel::GetSegmentSize() const
    {
        return _segmentSize;
    }

    AmSize ConvolverKernel::GetSegmentCount() const
    {
        return _segments.size();
    }

    AmSize ConvolverKernel::GetComplexSize() const
    {
        return _complexSize;
    }

    const SplitComplex& ConvolverKernel::GetSegment(AmSize index) const
    {
        AMPLITUDE_ASSERT(index < _segments.size());
        return *_segments[index];
    }
} // namespace SparkyStudios::Audio::Amplitude
