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
#include <cmath>

#include <SparkyStudios/Audio/Amplitude/Math/Utils.h>

#include <DSP/Resamplers/BandlimitedKernel.h>

namespace SparkyStudios::Audio::Amplitude
{
    AmReal64 BesselI0(AmReal64 x)
    {
        const AmReal64 q = x * x / 4.0;
        AmReal64 term = 1.0;
        AmReal64 sum = 1.0;

        for (AmUInt32 k = 1; k < 500; ++k)
        {
            term *= q / (static_cast<AmReal64>(k) * static_cast<AmReal64>(k));
            sum += term;
            if (term < sum * 1e-17)
                break;
        }

        return sum;
    }

    BandlimitedKernel::BandlimitedKernel(const BandlimitedKernelSpec& spec)
        : _spec(spec)
    {
        AMPLITUDE_ASSERT(spec.zeroCrossings > 0 && spec.phases > 0);

        const AmUInt64 size = GetTableSize();
        _table.assign(size + 2, 0.0f);

        const AmReal64 norm = BesselI0(spec.beta);
        for (AmUInt64 i = 0; i < size; ++i)
        {
            // The sinc is exactly 0 at every whole frame but the centre.
            if (i > 0 && i % spec.phases == 0)
                continue;

            const AmReal64 x = static_cast<AmReal64>(i) / static_cast<AmReal64>(spec.phases);
            const AmReal64 sinc = i == 0 ? 1.0 : std::sin(AM_PI * x) / (AM_PI * x);
            const AmReal64 t = x / static_cast<AmReal64>(spec.zeroCrossings);
            const AmReal64 window = BesselI0(spec.beta * std::sqrt(std::max(0.0, 1.0 - t * t))) / norm;

            _table[i] = static_cast<AmReal32>(sinc * window);
        }

        _deltas.assign(size + 2, 0.0f);
        for (AmUInt64 i = 0; i + 1 < _table.size(); ++i)
            _deltas[i] = _table[i + 1] - _table[i];
    }

    AmReal32 BandlimitedKernel::Evaluate(AmReal64 distance) const
    {
        const AmReal64 p = std::abs(distance) * static_cast<AmReal64>(_spec.phases);
        if (p >= static_cast<AmReal64>(GetTableSize()))
            return 0.0f;

        const auto i = static_cast<AmUInt64>(p);
        const auto f = static_cast<AmReal32>(p - static_cast<AmReal64>(i));
        return _table[i] + f * _deltas[i];
    }
} // namespace SparkyStudios::Audio::Amplitude
