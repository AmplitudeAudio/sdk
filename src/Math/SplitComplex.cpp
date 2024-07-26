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

#include <SparkyStudios/Audio/Amplitude/Math/SplitComplex.h>

namespace SparkyStudios::Audio::Amplitude
{
    SplitComplex::SplitComplex(const AmSize initialSize)
        : _size(0)
        , _re()
        , _im()
    {
        Resize(initialSize, false);
    }

    SplitComplex::~SplitComplex()
    {
        Release();
    }

    void SplitComplex::Release()
    {
        _re.Release();
        _im.Release();
        _size = 0;
    }

    void SplitComplex::Resize(const AmSize newSize, const bool clear)
    {
        if (newSize == _size)
            return;

        _re.Resize(newSize, clear);
        _im.Resize(newSize, clear);
        _size = newSize;
    }

    void SplitComplex::Clear() const
    {
        _re.Clear();
        _im.Clear();
    }

    void SplitComplex::CopyFrom(const SplitComplex& other) const
    {
        _re.CopyFrom(other._re);
        _im.CopyFrom(other._im);
    }

    AmAudioSample* SplitComplex::re()
    {
        return _re.GetBuffer();
    }

    const AmAudioSample* SplitComplex::re() const
    {
        return _re.GetBuffer();
    }

    AmAudioSample* SplitComplex::im()
    {
        return _im.GetBuffer();
    }

    const AmAudioSample* SplitComplex::im() const
    {
        return _im.GetBuffer();
    }

    std::complex<AmAudioSample> SplitComplex::operator[](AmSize index) const
    {
        AMPLITUDE_ASSERT(index < _size);
        return { _re[index], _im[index] };
    }
} // namespace SparkyStudios::Audio::Amplitude