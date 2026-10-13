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

#include <DSP/SummingStereoConvolver.h>
#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    SummingStereoConvolver::~SummingStereoConvolver()
    {
        Release();
    }

    bool SummingStereoConvolver::Init(
        std::span<const std::shared_ptr<const ConvolverKernel>> left, std::span<const std::shared_ptr<const ConvolverKernel>> right)
    {
        Release();

        if (left.size() != right.size() || left.empty() || left.size() > kMaxInputs)
            return false;

        AmSize blockSize = 0;
        AmSize complexSize = 0;
        AmSize ringSize = 0;

        for (const auto& kernels : { left, right })
        {
            for (const auto& kernel : kernels)
            {
                if (kernel == nullptr)
                    return false;

                if (kernel->GetSegmentCount() == 0)
                    continue;

                if (blockSize == 0)
                {
                    blockSize = kernel->GetBlockSize();
                    complexSize = kernel->GetComplexSize();
                }
                else if (kernel->GetBlockSize() != blockSize)
                {
                    return false;
                }

                ringSize = std::max(ringSize, kernel->GetSegmentCount());
            }
        }

        _kernels[0].assign(left.begin(), left.end());
        _kernels[1].assign(right.begin(), right.end());
        _inputCount = left.size();

        // Every kernel is empty: Process adds nothing.
        if (ringSize == 0)
            return true;

        _blockSize = blockSize;
        _complexSize = complexSize;
        _ringSize = ringSize;

        _fft.Initialize(2 * _blockSize);
        _fftBuffer.Resize(static_cast<AmUInt32>(2 * _blockSize));

        // One history ring per input, sized for the longest kernel; each kernel reads only its own segments.
        _history.reserve(_inputCount * _ringSize);
        for (AmSize i = 0; i < _inputCount * _ringSize; ++i)
        {
            auto* segment = ampoolnew(eMemoryPoolKind_Filtering, SplitComplex, _complexSize);
            segment->Clear();
            _history.push_back(segment);
        }

        _inputBuffer.Resize(static_cast<AmUInt32>(_inputCount * _blockSize));

        for (AmSize ear = 0; ear < 2; ++ear)
        {
            _preMultiplied[ear].Resize(_complexSize, true);
            _conv[ear].Resize(_complexSize, true);
            _overlap[ear].Resize(static_cast<AmUInt32>(_blockSize));
        }

        _current = 0;
        _inputBufferFill = 0;

        return true;
    }

    void SummingStereoConvolver::Process(const AmAudioSample* const* inputs, AmAudioSample* outL, AmAudioSample* outR, AmSize len)
    {
        if (_blockSize == 0)
            return;

        AmAudioSample* outputs[2] = { outL, outR };

        AmSize processed = 0;
        while (processed < len)
        {
            const bool inputBufferWasEmpty = _inputBufferFill == 0;
            const AmSize processing = std::min(len - processed, _blockSize - _inputBufferFill);
            const AmSize inputBufferPos = _inputBufferFill;

            // Forward FFT of each input's whole block into its history ring
            for (AmSize c = 0; c < _inputCount; ++c)
            {
                AmAudioSample* block = _inputBuffer.GetBuffer() + c * _blockSize;
                std::memcpy(block + inputBufferPos, inputs[c] + processed, processing * sizeof(AmAudioSample));
                CopyAndPad(_fftBuffer, block, _blockSize);
                _fft.Forward(_fftBuffer.GetBuffer(), History(c, _current));
            }

            for (AmSize ear = 0; ear < 2; ++ear)
            {
                // The older segments only change when a new block starts
                if (inputBufferWasEmpty)
                {
                    _preMultiplied[ear].Clear();
                    for (AmSize c = 0; c < _inputCount; ++c)
                    {
                        const auto& kernel = _kernels[ear][c];
                        for (AmSize i = 1; i < kernel->GetSegmentCount(); ++i)
                            ComplexMultiplyAccumulate(_preMultiplied[ear], kernel->GetSegment(i), History(c, (_current + i) % _ringSize));
                    }
                }

                _conv[ear].CopyFrom(_preMultiplied[ear]);
                for (AmSize c = 0; c < _inputCount; ++c)
                {
                    const auto& kernel = _kernels[ear][c];
                    if (kernel->GetSegmentCount() > 0)
                        ComplexMultiplyAccumulate(_conv[ear], kernel->GetSegment(0), History(c, _current));
                }

                _fft.Backward(_fftBuffer.GetBuffer(), _conv[ear]);

                const AmAudioSample* result = _fftBuffer.GetBuffer() + inputBufferPos;
                const AmAudioSample* overlap = _overlap[ear].GetBuffer() + inputBufferPos;
                AmAudioSample* output = outputs[ear] + processed;
                for (AmSize i = 0; i < processing; ++i)
                    output[i] += result[i] + overlap[i];

                if (inputBufferPos + processing == _blockSize)
                    std::memcpy(_overlap[ear].GetBuffer(), _fftBuffer.GetBuffer() + _blockSize, _blockSize * sizeof(AmAudioSample));
            }

            _inputBufferFill += processing;
            if (_inputBufferFill == _blockSize)
            {
                _inputBuffer.Clear();
                _inputBufferFill = 0;
                _current = _current > 0 ? _current - 1 : _ringSize - 1;
            }

            processed += processing;
        }
    }

    void SummingStereoConvolver::Reset()
    {
        for (auto* segment : _history)
            segment->Clear();

        _inputBuffer.Clear();

        for (AmSize ear = 0; ear < 2; ++ear)
        {
            _preMultiplied[ear].Clear();
            _conv[ear].Clear();
            _overlap[ear].Clear();
        }

        _current = 0;
        _inputBufferFill = 0;
    }

    AmSize SummingStereoConvolver::GetInputCount() const
    {
        return _inputCount;
    }

    void SummingStereoConvolver::Release()
    {
        for (auto* segment : _history)
            ampooldelete(eMemoryPoolKind_Filtering, SplitComplex, segment);

        _history.clear();
        _kernels[0].clear();
        _kernels[1].clear();
        _inputCount = 0;
        _blockSize = 0;
        _complexSize = 0;
        _ringSize = 0;
        _fftBuffer.Release();
        _fft.Initialize(0);
        _inputBuffer.Release();

        for (AmSize ear = 0; ear < 2; ++ear)
        {
            _preMultiplied[ear].Release();
            _conv[ear].Release();
            _overlap[ear].Release();
        }

        _current = 0;
        _inputBufferFill = 0;
    }

    SplitComplex& SummingStereoConvolver::History(AmSize input, AmSize slot)
    {
        return *_history[input * _ringSize + slot];
    }
} // namespace SparkyStudios::Audio::Amplitude
