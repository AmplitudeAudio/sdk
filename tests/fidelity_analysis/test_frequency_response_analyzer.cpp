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

#include <complex>
#include <numbers>

#include <Fidelity/Analysis/FrequencyResponse.h>
#include <Fidelity/Targets.h>

#include "PureUnitTestCase.h"
#include "TestRegistry.h"

namespace SparkyStudios::Audio::Amplitude::Tests
{
    using namespace SparkyStudios::Audio::Amplitude::Fidelity;

    namespace
    {
        constexpr double kRate = 48000.0;

        struct Biquad
        {
            double b0, b1, b2, a1, a2;

            // RBJ low-pass, Q = 1/sqrt(2): a 2nd-order Butterworth with -3 dB at `cutoff`.
            static Biquad LowPass(double cutoff)
            {
                const double w0 = 2.0 * std::numbers::pi * cutoff / kRate;
                const double alpha = std::sin(w0) / (2.0 / std::numbers::sqrt2);
                const double cosine = std::cos(w0);
                const double a0 = 1.0 + alpha;
                return { (1.0 - cosine) / 2.0 / a0, (1.0 - cosine) / a0, (1.0 - cosine) / 2.0 / a0, -2.0 * cosine / a0,
                         (1.0 - alpha) / a0 };
            }

            [[nodiscard]] Signal Apply(const Signal& x) const
            {
                Signal y(x.size(), 0.0);
                for (std::size_t i = 0; i < x.size(); ++i)
                {
                    const double x1 = i >= 1 ? x[i - 1] : 0.0;
                    const double x2 = i >= 2 ? x[i - 2] : 0.0;
                    const double y1 = i >= 1 ? y[i - 1] : 0.0;
                    const double y2 = i >= 2 ? y[i - 2] : 0.0;
                    y[i] = b0 * x[i] + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
                }

                return y;
            }

            [[nodiscard]] double MagnitudeDb(double frequency) const
            {
                const std::complex<double> z = std::polar(1.0, -2.0 * std::numbers::pi * frequency / kRate);
                const std::complex<double> h = (b0 + b1 * z + b2 * z * z) / (1.0 + a1 * z + a2 * z * z);
                return 20.0 * std::log10(std::abs(h));
            }
        };

        double MagnitudeAt(const ResponseResult& r, double frequency)
        {
            std::size_t best = 0;
            for (std::size_t i = 1; i < r.frequencyHz.size(); ++i)
                if (std::abs(r.frequencyHz[i] - frequency) < std::abs(r.frequencyHz[best] - frequency))
                    best = i;

            return r.magnitudeDb[best];
        }

        Signal Delayed(const Signal& x, std::size_t delay)
        {
            Signal out(delay, 0.0);
            out.insert(out.end(), x.begin(), x.end());
            out.insert(out.end(), delay, 0.0);
            return out;
        }
    } // namespace

    AM_TEST_CASE(PureUnitTestCase, fidelity_analysis, frequency_response_analyzer)
    {
    public:
        void Run() override
        {
            const SweepModel model{ 10.0, 0.95 * kRate / 2.0, 2.0, 0.5, 0.01 };
            const Signal sweep = RenderLogSweep(model, kRate, model.f2Hz);

            ResponseOptions options;
            options.passbandHighHz = 20000.0;

            // Identity (with latency): flat.
            const ResponseResult identity = AnalyzeFrequencyResponse(Delayed(sweep, 1000), kRate, model, options);
            AM_EXPECT(identity.rippleDb <= Floors::kRippleDb);
            AM_EXPECT(identity.minus3dBHz >= 20000.0);

            // A Butterworth low-pass at 8 kHz is measured against its exact response.
            const Biquad filter = Biquad::LowPass(8000.0);
            const ResponseResult filtered = AnalyzeFrequencyResponse(Delayed(filter.Apply(sweep), 1000), kRate, model, options);
            const double reference = filter.MagnitudeDb(1000.0);
            for (const double frequency : { 4000.0, 8000.0, 12000.0 })
                AM_EXPECT(std::abs(MagnitudeAt(filtered, frequency) - (filter.MagnitudeDb(frequency) - reference)) <= 0.2);

            AM_EXPECT(std::abs(filtered.minus3dBHz - 8000.0) <= 100.0);
        }
    };

    AM_REGISTER_TEST(fidelity_analysis, frequency_response_analyzer);
} // namespace SparkyStudios::Audio::Amplitude::Tests
