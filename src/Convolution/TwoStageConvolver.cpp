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
//
// Based on the code from https://github.com/HiFi-LoFi/FFTConvolver
// Copyright (c) 2017 HiFi-LoFi, MIT License

#include <algorithm>
#include <cmath>

#include <SparkyStudios/Audio/Amplitude/Convolution/TwoStageConvolver.h>

#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude::Convolution
{

    TwoStageConvolver::TwoStageConvolver()
        : _headBlockSize(0)
        , _tailBlockSize(0)
        , _headConvolver()
        , _tailConvolver0()
        , _tailOutput0()
        , _tailPrecalculated0()
        , _tailConvolver()
        , _tailOutput()
        , _tailPrecalculated()
        , _tailInput()
        , _tailInputFill(0)
        , _precalculatedPos(0)
        , _backgroundProcessingInput()
    {}

    TwoStageConvolver::~TwoStageConvolver()
    {
        Reset();
    }

    void TwoStageConvolver::Reset()
    {
        _headBlockSize = 0;
        _tailBlockSize = 0;
        _headConvolver.Reset();
        _tailConvolver0.Reset();
        _tailOutput0.Release();
        _tailPrecalculated0.Release();
        _tailConvolver.Reset();
        _tailOutput.Release();
        _tailPrecalculated.Release();
        _tailInput.Release();
        _tailInputFill = 0;
        _tailInputFill = 0;
        _precalculatedPos = 0;
        _backgroundProcessingInput.Release();
    }

    bool TwoStageConvolver::Init(AmSize headBlockSize, AmSize tailBlockSize, const AmAudioSample* ir, AmSize irLen)
    {
        Reset();

        if (headBlockSize == 0 || tailBlockSize == 0)
            return false;

        headBlockSize = std::max(static_cast<AmSize>(1), headBlockSize);
        if (headBlockSize > tailBlockSize)
        {
            AMPLITUDE_ASSERT(false);
            std::swap(headBlockSize, tailBlockSize);
        }

        // Ignore zeros at the end of the impulse response because they only waste computation time
        while (irLen > 0 && ::fabs(ir[irLen - 1]) < 0.000001f)
            --irLen;

        if (irLen == 0)
            return true;

        _headBlockSize = NextPowerOf2(headBlockSize);
        _tailBlockSize = NextPowerOf2(tailBlockSize);

        const AmSize headIrLen = std::min(irLen, _tailBlockSize);
        _headConvolver.Init(_headBlockSize, ir, headIrLen);

        if (irLen > _tailBlockSize)
        {
            const AmSize conv1IrLen = std::min(irLen - _tailBlockSize, _tailBlockSize);
            _tailConvolver0.Init(_headBlockSize, ir + _tailBlockSize, conv1IrLen);
            _tailOutput0.Resize(_tailBlockSize);
            _tailPrecalculated0.Resize(_tailBlockSize);
        }

        if (irLen > 2 * _tailBlockSize)
        {
            const AmSize tailIrLen = irLen - (2 * _tailBlockSize);
            _tailConvolver.Init(_tailBlockSize, ir + (2 * _tailBlockSize), tailIrLen);
            _tailOutput.Resize(_tailBlockSize);
            _tailPrecalculated.Resize(_tailBlockSize);
            _backgroundProcessingInput.Resize(_tailBlockSize);
        }

        if (_tailPrecalculated0.GetSize() > 0 || _tailPrecalculated.GetSize() > 0)
            _tailInput.Resize(_tailBlockSize);

        _tailInputFill = 0;
        _precalculatedPos = 0;

        return true;
    }

    void TwoStageConvolver::Process(const AmAudioSample* input, AmAudioSample* output, AmSize len)
    {
        // Head
        _headConvolver.Process(input, output, len);

        // Tail
        if (_tailInput.GetSize() > 0)
        {
            AmSize processed = 0;
            while (processed < len)
            {
                const AmSize remaining = len - processed;
                const AmSize processing = std::min(remaining, _headBlockSize - (_tailInputFill % _headBlockSize));
                AMPLITUDE_ASSERT(_tailInputFill + processing <= _tailBlockSize);

                // Sum head and tail
                const AmSize sumBegin = processed;
                const AmSize sumEnd = processed + processing;
                {
                    // Sum: 1st tail block
                    if (_tailPrecalculated0.GetSize() > 0)
                    {
                        AmSize precalculatedPos = _precalculatedPos;
                        for (AmSize i = sumBegin; i < sumEnd; ++i)
                        {
                            output[i] += _tailPrecalculated0[precalculatedPos];
                            ++precalculatedPos;
                        }
                    }

                    // Sum: 2nd-Nth tail block
                    if (_tailPrecalculated.GetSize() > 0)
                    {
                        AmSize precalculatedPos = _precalculatedPos;
                        for (AmSize i = sumBegin; i < sumEnd; ++i)
                        {
                            output[i] += _tailPrecalculated[precalculatedPos];
                            ++precalculatedPos;
                        }
                    }

                    _precalculatedPos += processing;
                }

                // Fill input buffer for tail convolution
                std::memcpy(_tailInput.GetBuffer() + _tailInputFill, input + processed, processing * sizeof(AmAudioSample));
                _tailInputFill += processing;
                AMPLITUDE_ASSERT(_tailInputFill <= _tailBlockSize);

                // Convolution: 1st tail block
                if (_tailPrecalculated0.GetSize() > 0 && _tailInputFill % _headBlockSize == 0)
                {
                    AMPLITUDE_ASSERT(_tailInputFill >= _headBlockSize);
                    const AmSize blockOffset = _tailInputFill - _headBlockSize;
                    _tailConvolver0.Process(_tailInput.GetBuffer() + blockOffset, _tailOutput0.GetBuffer() + blockOffset, _headBlockSize);
                    if (_tailInputFill == _tailBlockSize)
                    {
                        AmAlignedReal32Buffer::Swap(_tailPrecalculated0, _tailOutput0);
                    }
                }

                // Convolution: 2nd-Nth tail block (might be done in some background thread)
                if (_tailPrecalculated.GetSize() > 0 && _tailInputFill == _tailBlockSize &&
                    _backgroundProcessingInput.GetSize() == _tailBlockSize && _tailOutput.GetSize() == _tailBlockSize)
                {
                    WaitForBackgroundProcessing();
                    AmAlignedReal32Buffer::Swap(_tailPrecalculated, _tailOutput);
                    _backgroundProcessingInput.CopyFrom(_tailInput);
                    StartBackgroundProcessing();
                }

                if (_tailInputFill == _tailBlockSize)
                {
                    _tailInputFill = 0;
                    _precalculatedPos = 0;
                }

                processed += processing;
            }
        }
    }

    void TwoStageConvolver::StartBackgroundProcessing()
    {
        DoBackgroundProcessing();
    }

    void TwoStageConvolver::WaitForBackgroundProcessing()
    {}

    void TwoStageConvolver::DoBackgroundProcessing()
    {
        _tailConvolver.Process(_backgroundProcessingInput.GetBuffer(), _tailOutput.GetBuffer(), _tailBlockSize);
    }
} // namespace SparkyStudios::Audio::Amplitude::Convolution
