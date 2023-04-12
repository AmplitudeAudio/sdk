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

#pragma once

#ifndef SS_AMPLITUDE_AUDIO_ATTENUATION_H
#define SS_AMPLITUDE_AUDIO_ATTENUATION_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

#include <SparkyStudios/Audio/Amplitude/Core/Entity.h>
#include <SparkyStudios/Audio/Amplitude/Core/Listener.h>
#include <SparkyStudios/Audio/Amplitude/Core/RefCounter.h>

#include <SparkyStudios/Audio/Amplitude/Math/Curve.h>
#include <SparkyStudios/Audio/Amplitude/Math/Shape.h>

namespace SparkyStudios::Audio::Amplitude
{
    struct AttenuationDefinition;
    struct AttenuationShapeDefinition;

    class Attenuation;

    /**
     * @brief The propagation shape for positional sounds.
     *
     * This allows to increase the attenuation according to the shape of
     * the sound propagation.
     */
    class AttenuationZone
    {
    public:
        /**
         * @brief Returns the attenuation factor.
         *
         * This method is used only for position based sound sources.
         *
         * @param attenuation The Attenuator object to use for distance attenuation.
         * @param soundLocation The location of the sound source.
         * @param listener The listener for which compute the attenuation.
         *
         * @return The attenuation factor.
         */
        virtual float GetAttenuationFactor(const Attenuation* attenuation, const AmVec3& soundLocation, const Listener& listener);

        /**
         * @brief Returns the attenuation factor.
         *
         * This method is used by position and orientation based sound sources.
         *
         * @param attenuation The Attenuator object to use for distance attenuation.
         * @param entity The entity which emits the sound.
         * @param listener The listener for which compute the attenuation.
         *
         * @return The attenuation factor.
         */
        virtual float GetAttenuationFactor(const Attenuation* attenuation, const Entity& entity, const Listener& listener);

        /**
         * @brief Creates an AttenuationZone object from the definition.
         *
         * @param definition The attenuation shape definition.
         *
         * @return An AttenuationZone object.
         */
        static AttenuationZone* Create(const AttenuationShapeDefinition* definition);

    protected:
        AttenuationZone();

        /**
         * @brief The maximum attenuation factor to apply to the sound gain.
         */
        float m_maxAttenuationFactor;
    };

    /**
     * @brief An Attenuation materialize how the sound volume and other distance-based
     * parameters are calculated following the distance of the sound source to the listener.
     *
     * The Attenuation is an shared object between sound sources. They are used only
     * when the sound need to lower his volume due to the distance of from the listener,
     * and many other parameters.
     */
    class Attenuation
    {
    public:
        /**
         * @brief Creates an uninitialized Attenuation.
         *
         * An uninitialized Attenuation instance cannot compute gain nor provide
         * attenuation configuration data.
         */
        Attenuation();

        bool LoadAttenuationDefinition(const std::string& attenuation);
        bool LoadAttenuationDefinitionFromFile(const AmOsString& filename);

        /**
         * @brief Returns the gain of the sound from the given distance to the listener.
         *
         * @param soundLocation The location of the sound source.
         * @param listener The listener which is hearing the sound.
         *
         * @return The computed gain value fom the curve.
         */
        float GetGain(const AmVec3& soundLocation, const Listener& listener) const;

        /**
         * @brief Returns the gain of the sound from the given distance to the listener.
         *
         * @param entity The entity which emits the sound.
         * @param listener The listener which is hearing the sound.
         *
         * @return The computed gain value fom the curve.
         */
        float GetGain(const Entity& entity, const Listener& listener) const;

        /**
         * @brief Returns the unique ID of this Attenuation.
         *
         * @return The Attenuation ID.
         */
        [[nodiscard]] AmAttenuationID GetId() const;

        /**
         * @brief Returns the name of this Attenuation.
         *
         * @return The Attenuation name.
         */
        [[nodiscard]] const std::string& GetName() const;

        /**
         * @brief Returns the shape object of this Attenuation.
         *
         * @return The Attenuation shape.
         */
        [[nodiscard]] AttenuationZone* GetShape() const;

        /**
         * @brief Returns the gain curve attached to this Attenuation.
         *
         * @return The attenuation's gain curve.
         */
        [[nodiscard]] const Curve& GetGainCurve() const;

        /**
         * @brief Returns the maximum distance for a fully attenuated sound
         *
         * @return The maximum sound attenuation distance.
         */
        [[nodiscard]] double GetMaxDistance() const;

        /**
         * @brief Get the attenuation definition which generated this attenuation.
         *
         * @return The attenuation definition.
         */
        [[nodiscard]] const AttenuationDefinition* GetAttenuationDefinition() const;

        RefCounter* GetRefCounter();

    private:
        std::string _source;

        AmAttenuationID _id;
        std::string _name;

        double _maxDistance;

        AttenuationZone* _shape;

        Curve _gainCurve;

        RefCounter _refCounter;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_ATTENUATION_H
