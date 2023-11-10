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
#include <SparkyStudios/Audio/Amplitude/Math/FFT.h>

#include <Utils/Audio/FFT/AudioFFT.h>

using namespace SparkyStudios::Audio::Amplitude::FFT;

namespace SparkyStudios::Audio::Amplitude
{
    AmUInt64 FFT::GetOutputSize(SparkyStudios::Audio::Amplitude::AmUInt64 inputSize)
    {
        return AudioFFT::ComplexSize(inputSize);
    }

    FFT::FFT()
    {
        _implementation = amnew(AudioFFT);
    }

    FFT::~FFT()
    {
        amdelete(AudioFFT, (AudioFFT*)_implementation);
    }

    void FFT::Initialize(AmUInt64 size)
    {
        ((AudioFFT*)_implementation)->init(size);
    }

    void FFT::Forward(AmConstAudioSampleBuffer input, AmReal32Buffer re, AmReal32Buffer im)
    {
        ((AudioFFT*)_implementation)->fft(input, re, im);
    }

    void FFT::Backward(AmAudioSampleBuffer output, AmConstReal32Buffer re, AmConstReal32Buffer im)
    {
        ((AudioFFT*)_implementation)->ifft(output, re, im);
    }
} // namespace SparkyStudios::Audio::Amplitude