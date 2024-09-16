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

#ifndef _AM_IMPLEMENTATION_AMBISONICS_AMBISONIC_ORIENTATION_PROCESSOR_H
#define _AM_IMPLEMENTATION_AMBISONICS_AMBISONIC_ORIENTATION_PROCESSOR_H

#include <SparkyStudios/Audio/Amplitude/Math/Orientation.h>

#include <Ambisonics/AmbisonicComponent.h>

namespace SparkyStudios::Audio::Amplitude
{
    class BFormat;

    class AmbisonicOrientationProcessor : public AmbisonicComponent
    {
    public:
        AmbisonicOrientationProcessor();

        ~AmbisonicOrientationProcessor() override;

        /**
         * @copydoc AmbisonicComponent::Configure
         */
        bool Configure(AmUInt32 order, bool is3D) override;

        /**
         * @copydoc AmbisonicComponent::Refresh
         */
        void Refresh() override;

        /**
         * @copydoc AmbisonicComponent::Reset
         */
        void Reset() override;

        AM_INLINE void SetOrientation(const Orientation& orientation)
        {
            _orientation = orientation;
            Refresh();
        }

        [[nodiscard]] AM_INLINE const Orientation& GetOrientation() const
        {
            return _orientation;
        }

        void Process(BFormat* input, AmUInt32 samples);

    private:
        void ProcessOrder1(BFormat* input, AmUInt32 samples);
        void ProcessOrder2(BFormat* input, AmUInt32 samples);
        void ProcessOrder3(BFormat* input, AmUInt32 samples);

        Orientation _orientation;
        AmAlignedReal32Buffer _tempSamples;

        AmReal32 _cosAlpha;
        AmReal32 _sinAlpha;
        AmReal32 _cosBeta;
        AmReal32 _sinBeta;
        AmReal32 _cosGamma;
        AmReal32 _sinGamma;

        AmReal32 _cos2Alpha;
        AmReal32 _sin2Alpha;
        AmReal32 _cos2Beta;
        AmReal32 _sin2Beta;
        AmReal32 _cos2Gamma;
        AmReal32 _sin2Gamma;

        AmReal32 _cos3Alpha;
        AmReal32 _sin3Alpha;
        AmReal32 _cos3Beta;
        AmReal32 _sin3Beta;
        AmReal32 _cos3Gamma;
        AmReal32 _sin3Gamma;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IMPLEMENTATION_AMBISONICS_AMBISONIC_ORIENTATION_PROCESSOR_H
