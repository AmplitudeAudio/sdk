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

#include <Ambisonics/AmbisonicComponent.h>
#include <Utils/Utils.h>

namespace SparkyStudios::Audio::Amplitude
{
    AmbisonicComponent::AmbisonicComponent()
        : m_order(0)
        , m_is3D(false)
        , m_channelCount(0)
    {}

    bool AmbisonicComponent::Configure(AmUInt32 order, bool is3D)
    {
        m_order = order;
        m_is3D = is3D;
        m_channelCount = OrderToComponents(order, is3D);

        return true;
    }
} // namespace SparkyStudios::Audio::Amplitude
