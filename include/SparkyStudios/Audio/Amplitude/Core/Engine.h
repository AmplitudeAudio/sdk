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

#ifndef SS_AMPLITUDE_AUDIO_ENGINE_H
#define SS_AMPLITUDE_AUDIO_ENGINE_H

#include <string>

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

#include <SparkyStudios/Audio/Amplitude/Core/Bus.h>
#include <SparkyStudios/Audio/Amplitude/Core/Channel.h>
#include <SparkyStudios/Audio/Amplitude/Core/Driver.h>
#include <SparkyStudios/Audio/Amplitude/Core/Entity.h>
#include <SparkyStudios/Audio/Amplitude/Core/Event.h>
#include <SparkyStudios/Audio/Amplitude/Core/Listener.h>
#include <SparkyStudios/Audio/Amplitude/Core/Version.h>

#include <SparkyStudios/Audio/Amplitude/Sound/Attenuation.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Collection.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Effect.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Rtpc.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Sound.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Switch.h>
#include <SparkyStudios/Audio/Amplitude/Sound/SwitchContainer.h>

namespace SparkyStudios::Audio::Amplitude
{
    struct EngineConfigDefinition;

    struct EngineInternalState;

    typedef SwitchContainer* SwitchContainerHandle;
    typedef Collection* CollectionHandle;
    typedef Sound* SoundHandle;
    typedef Event* EventHandle;
    typedef Attenuation* AttenuationHandle;
    typedef Switch* SwitchHandle;
    typedef Rtpc* RtpcHandle;
    typedef Effect* EffectHandle;

    /**
     * @brief The central class of  the library that manages the Listeners, Entities,
     * Sounds, Collections, Channels, and tracks all of the internal state.
     */
    class Engine
    {
    private:
        /**
         * @brief Construct an uninitialized Engine.
         */
        Engine();

    public:
        ~Engine();

        /**
         * @brief Initialize the audio engine.
         *
         * @param configFile The path to the file containing an EngineConfigDefinition flatbuffer binary.
         *
         * @return Whether initialization was successful.
         */
        bool Initialize(AmOsString configFile);

        /**
         * @brief Initialize the audio engine.
         *
         * @param config A pointer to a loaded EngineConfigDefinition object.
         *
         * @return Whether initialization was successful.
         */
        bool Initialize(const EngineConfigDefinition* config);

        /**
         * @brief Deinitialize the audio engine.
         *
         * @return Whether the engine was successfully deinitialized.
         */
        bool Deinitialize();

        /**
         * @brief Checks if the audio engine has been initialized.
         *
         * @return Whether the audio engine is initialized.
         */
        bool IsInitialized();

        /**
         * @brief Set the File Loader used by the engine.
         *
         * @param loader A FileLoader implementation.
         */
        void SetFileLoader(const FileLoader& loader);

        /**
         * @brief Get the File Loader used by the engine.
         *
         * @return The file loader.
         */
        [[nodiscard]] const FileLoader* GetFileLoader() const;

        /**
         * @brief Update audio volume per channel each frame.
         *
         * @param delta The number of elapsed seconds since the last frame.
         */
        void AdvanceFrame(AmTime delta);

        /**
         * @brief Gets the total elapsed time since the start of the game.
         *
         * @return The total elapsed time since the start of the game.
         */
        [[nodiscard]] AmTime GetTotalTime() const;

        /**
         * @brief Load a sound bank from a file. Queue the sound files in that sound
         *        bank for loading. Call StartLoadingSoundFiles() to trigger loading
         *        of the sound files on a separate thread.
         *
         * @param filename The file containing the SoundBank flatbuffer binary data.
         *
         * @return true Returns true on success
         */
        bool LoadSoundBank(AmOsString filename);

        /**
         * @brief Load a sound bank from a file. Queue the sound files in that sound
         *        bank for loading. Call StartLoadingSoundFiles() to trigger loading
         *        of the sound files on a separate thread.
         *
         * @param filename The file containing the SoundBank flatbuffer binary data.
         * @param outID The ID of the loaded soundbank.
         *
         * @return true Returns true on success
         */
        bool LoadSoundBank(AmOsString filename, AmBankID& outID);

        /**
         * @brief Load a sound bank from memory. Queue the sound files in that sound
         *        bank for loading. Call StartLoadingSoundFiles() to trigger loading
         *        of the sound files on a separate thread.
         *
         * @param fileData The SoundBank flatbuffer binary data.
         *
         * @return true Returns true on success
         */
        bool LoadSoundBankFromMemory(const char* fileData);

        /**
         * @brief Load a sound bank from memory. Queue the sound files in that sound
         *        bank for loading. Call StartLoadingSoundFiles() to trigger loading
         *        of the sound files on a separate thread.
         *
         * @param fileData The SoundBank flatbuffer binary data.
         * @param outID The ID of the loaded soundbank.
         *
         * @return true Returns true on success
         */
        bool LoadSoundBankFromMemory(const char* fileData, AmBankID& outID);

        /**
         * @brief Load a sound bank from memory. Queue the sound files in that sound
         *        bank for loading. Call StartLoadingSoundFiles() to trigger loading
         *        of the sound files on a separate thread.
         *
         * @param fileData The SoundBank flatbuffer binary data.
         *
         * @return true Returns true on success
         */
        bool LoadSoundBankFromMemoryView(void* ptr, AmSize size);

        /**
         * @brief Load a sound bank from memory. Queue the sound files in that sound
         *        bank for loading. Call StartLoadingSoundFiles() to trigger loading
         *        of the sound files on a separate thread.
         *
         * @param fileData The SoundBank flatbuffer binary data.
         * @param outID The ID of the loaded soundbank.
         *
         * @return true Returns true on success
         */
        bool LoadSoundBankFromMemoryView(void* ptr, AmSize size, AmBankID& outID);

        /**
         * @brief Unload a sound bank.
         *
         * @param filename The file to unload.
         */
        void UnloadSoundBank(AmOsString filename);

        /**
         * @brief Unload a sound bank.
         *
         * @param id The soundbank id to unload.
         */
        void UnloadSoundBank(AmBankID id);

        /**
         * @brief Unload all loaded sound banks
         */
        void UnloadSoundBanks();

        /**
         * @brief Kick off loading thread to load all sound files queued with
         *        LoadSoundBank().
         */
        void StartLoadingSoundFiles();

        /**
         * @brief Return true if all sound files have been loaded. Must call
         *        StartLoadingSoundFiles() first.
         */
        bool TryFinalizeLoadingSoundFiles();

        /**
         * @brief Get a SwitchContainerHandle given its name as defined in its JSON data.
         *
         * @param name The unique name as defined in the JSON data.
         */
        [[nodiscard]] SwitchContainerHandle GetSwitchContainerHandle(const std::string& name) const;

        /**
         * @brief Get a SwitchContainerHandle given its ID as defined in its JSON data.
         *
         * @param id The unique ID as defined in the JSON data.
         */
        [[nodiscard]] SwitchContainerHandle GetSwitchContainerHandle(AmSwitchContainerID id) const;

        /**
         * @brief Get a SwitchContainerHandle given its SwitchContainerDefinition filename.
         *
         * @param filename The filename containing the flatbuffer binary data.
         */
        [[nodiscard]] SwitchContainerHandle GetSwitchContainerHandleFromFile(AmOsString filename) const;

        /**
         * @brief Get a CollectionHandle given its name as defined in its JSON data.
         *
         * @param name The unique name as defined in the JSON data.
         */
        [[nodiscard]] CollectionHandle GetCollectionHandle(const std::string& name) const;

        /**
         * @brief Get a CollectionHandle given its ID as defined in its JSON data.
         *
         * @param id The unique ID as defined in the JSON data.
         */
        [[nodiscard]] CollectionHandle GetCollectionHandle(AmCollectionID id) const;

        /**
         * @brief Get a CollectionHandle given its CollectionDefinition filename.
         *
         * @param filename The filename containing the flatbuffer binary data.
         */
        [[nodiscard]] CollectionHandle GetCollectionHandleFromFile(AmOsString filename) const;

        /**
         * @brief Get a SoundHandle given its name as defined in its JSON data.
         *
         * @param name The unique name as defined in the JSON data.
         */
        [[nodiscard]] SoundHandle GetSoundHandle(const std::string& name) const;

        /**
         * @brief Get a SoundHandle given its ID as defined in its JSON data.
         *
         * @param id The unique ID as defined in the JSON data.
         */
        [[nodiscard]] SoundHandle GetSoundHandle(AmSoundID id) const;

        /**
         * @brief Get a SoundHandle given its SoundDefinition filename.
         *
         * @param filename The filename containing the flatbuffer binary data.
         */
        [[nodiscard]] SoundHandle GetSoundHandleFromFile(AmOsString filename) const;

        /**
         * @brief Get an EventHandle given its name as defined in its JSON data.
         *
         * @param name The unique name as defined in the JSON data.
         */
        [[nodiscard]] EventHandle GetEventHandle(const std::string& name) const;

        /**
         * @brief Get an EventHandle given its ID as defined in its JSON data.
         *
         * @param id The unique ID as defined in the JSON data.
         */
        [[nodiscard]] EventHandle GetEventHandle(AmEventID id) const;

        /**
         * @brief Get an EventHandle given its EventDefinition filename.
         *
         * @param filename The filename containing the flatbuffer binary data.
         */
        [[nodiscard]] EventHandle GetEventHandleFromFile(AmOsString filename) const;

        /**
         * @brief Get an AttenuationHandle given its name as defined in its JSON data.
         *
         * @param name The unique name as defined in the JSON data.
         */
        [[nodiscard]] AttenuationHandle GetAttenuationHandle(const std::string& name) const;

        /**
         * @brief Get an AttenuationHandle given its ID as defined in its JSON data.
         *
         * @param id The unique ID as defined in the JSON data.
         */
        [[nodiscard]] AttenuationHandle GetAttenuationHandle(AmAttenuationID id) const;

        /**
         * @brief Get an AttenuationHandle given its AttenuationDefinition filename.
         *
         * @param filename The filename containing the flatbuffer binary data.
         */
        [[nodiscard]] AttenuationHandle GetAttenuationHandleFromFile(AmOsString filename) const;

        /**
         * @brief Get a SwitchHandle given its name as defined in its JSON data.
         *
         * @param name The unique name as defined in the JSON data.
         */
        [[nodiscard]] SwitchHandle GetSwitchHandle(const std::string& name) const;

        /**
         * @brief Get an SwitchHandle given its ID as defined in its JSON data.
         *
         * @param id The unique ID as defined in the JSON data.
         */
        [[nodiscard]] SwitchHandle GetSwitchHandle(AmSwitchID id) const;

        /**
         * @brief Get an SwitchHandle given its SwitchDefinition filename.
         *
         * @param filename The filename containing the flatbuffer binary data.
         */
        [[nodiscard]] SwitchHandle GetSwitchHandleFromFile(AmOsString filename) const;

        /**
         * @brief Get a RtpcHandle given its name as defined in its JSON data.
         *
         * @param name The unique name as defined in the JSON data.
         */
        [[nodiscard]] RtpcHandle GetRtpcHandle(const std::string& name) const;

        /**
         * @brief Get an RtpcHandle given its ID as defined in its JSON data.
         *
         * @param id The unique ID as defined in the JSON data.
         */
        [[nodiscard]] RtpcHandle GetRtpcHandle(AmRtpcID id) const;

        /**
         * @brief Get an RtpcHandle given its RtpcDefinition filename.
         *
         * @param filename The filename containing the flatbuffer binary data.
         */
        [[nodiscard]] RtpcHandle GetRtpcHandleFromFile(AmOsString filename) const;

        /**
         * @brief Get a EffectHandle given its name as defined in its JSON data.
         *
         * @param name The unique name as defined in the JSON data.
         */
        [[nodiscard]] EffectHandle GetEffectHandle(const std::string& name) const;

        /**
         * @brief Get an EffectHandle given its ID as defined in its JSON data.
         *
         * @param id The unique ID as defined in the JSON data.
         */
        [[nodiscard]] EffectHandle GetEffectHandle(AmEffectID id) const;

        /**
         * @brief Get an EffectHandle given its EffectDefinition filename.
         *
         * @param filename The filename containing the flatbuffer binary data.
         */
        [[nodiscard]] EffectHandle GetEffectHandleFromFile(AmOsString filename) const;

        /**
         * @brief Adjusts the gain on the master bus.
         *
         * @param gain the gain to apply to all buses.
         */
        void SetMasterGain(float gain);

        /**
         * @brief Get the master bus's current gain.
         *
         * @return the master bus's current gain.
         */
        [[nodiscard]] float GetMasterGain() const;

        /**
         * @brief Mutes the Engine completely.
         *
         * @param mute whether to mute or unmute the Engine.
         */
        void SetMute(bool mute);

        /**
         * @brief Whether the Engine is currently muted.
         *
         * @return Whether the Engine is currently muted.
         */
        [[nodiscard]] bool GetMute() const;

        /**
         * @brief Pauses all playing sounds and streams.
         *
         * @param pause Whether to pause or resume the Engine.
         */
        void Pause(bool pause);

        /**
         * @brief Initialize and return a Listener.
         *
         * @param id The game Listener ID.
         *
         * @return An initialized Listener.
         */
        Listener AddListener(AmListenerID id);

        /**
         * @brief Remove a Listener given its ID.
         *
         * @param id The ID of the Listener to be removed.
         */
        void RemoveListener(AmListenerID id);

        /**
         * @brief Remove a Listener.
         *
         * @param listener The Listener to be removed.
         */
        void RemoveListener(const Listener* listener);

        /**
         * @brief Initialize and return an Entity.
         *
         * @param id The game Entity ID.
         *
         * @return An initialized Entity.
         */
        Entity AddEntity(AmEntityID id);

        /**
         * @brief Remove an Entity.
         *
         * @param entity The Entity to be removed.
         */
        void RemoveEntity(const Entity* entity);

        /**
         * @brief Remove an Entity given its ID.
         *
         * @param id The ID of the Entity to be removed.
         */
        void RemoveEntity(AmEntityID id);

        /**
         * @brief Returns the bus with the specified name.
         *
         * @param name The name of the bus.
         *
         * @return A valid bus if found, otherwise an invalid bus.
         */
        [[nodiscard]] Bus FindBus(AmString name) const;

        /**
         * @brief Returns the bus with the given ID.
         *
         * @param id The ID of the bus.
         *
         * @return A valid bus if found, otherwise an invalid bus.
         */
        [[nodiscard]] Bus FindBus(AmBusID id) const;

        /**
         * @brief Play a switch container associated with the given handle in the
         *        World scope.
         *
         * @param handle A handle to the switch container to play.
         *
         * @return The channel the switch container is played on. If the switch container could not be
         *         played, an invalid Channel is returned.
         */
        Channel Play(SwitchContainerHandle handle);

        /**
         * @brief Play a switch container associated with the given handle in the
         *        World scope at the given location.
         *
         * @param handle A handle to the switch container to play.
         * @param location The location on which play the switch container.
         *
         * @return The channel the switch container is played on. If the switch container could not be
         *         played, an invalid Channel is returned.
         */
        Channel Play(SwitchContainerHandle handle, const hmm_vec3& location);

        /**
         * @brief Play a switch container associated with the given handle in the
         *        location with the given gain.
         *
         * @param handle A handle to the switch container to play.
         * @param location The location on which play the switch container.
         * @param userGain The gain of the sound.
         *
         * @return The channel the switch container is played on. If the switch container could not be
         *         played, an invalid Channel is returned.
         */
        Channel Play(SwitchContainerHandle handle, const hmm_vec3& location, float userGain);

        /**
         * @brief Play a switch container associated with the given handle in the
         *        Entity scope.
         *
         * @param handle A handle to the switch container to play.
         * @param entity The entity which is playing the switch container.
         *
         * @return The channel the switch container is played on. If the switch container could not be
         *         played, an invalid Channel is returned.
         */
        Channel Play(SwitchContainerHandle handle, const Entity& entity);

        /**
         * @brief Play a switch container associated with the given handle in an
         *        Entity scope with the given gain.
         *
         * @param handle A handle to the switch container to play.
         * @param entity The entity which is playing the switch container.
         * @param userGain The gain of the sound.
         *
         * @return The channel the switch container is played on. If the switch container could not be
         *         played, an invalid Channel is returned.
         */
        Channel Play(SwitchContainerHandle handle, const Entity& entity, float userGain);

        /**
         * @brief Play a collection associated with the given handle in the
         *        World scope.
         *
         * @param handle A handle to the collection to play.
         *
         * @return The channel the collection is played on. If the collection could not be
         *         played, an invalid Channel is returned.
         */
        Channel Play(CollectionHandle handle);

        /**
         * @brief Play a collection associated with the given handle in the
         *        World scope at the given location.
         *
         * @param handle A handle to the collection to play.
         * @param location The location on which play the collection.
         *
         * @return The channel the collection is played on. If the collection could not be
         *         played, an invalid Channel is returned.
         */
        Channel Play(CollectionHandle handle, const hmm_vec3& location);

        /**
         * @brief Play a collection associated with the given handle in the
         *        location with the given gain.
         *
         * @param handle A handle to the collection to play.
         * @param location The location on which play the collection.
         * @param userGain The gain of the sound.
         *
         * @return The channel the collection is played on. If the collection could not be
         *         played, an invalid Channel is returned.
         */
        Channel Play(CollectionHandle handle, const hmm_vec3& location, float userGain);

        /**
         * @brief Play a collection associated with the given handle in the
         *        Entity scope.
         *
         * @param handle A handle to the collection to play.
         * @param entity The entity which is playing the collection.
         *
         * @return The channel the collection is played on. If the collection could not be
         *         played, an invalid Channel is returned.
         */
        Channel Play(CollectionHandle handle, const Entity& entity);

        /**
         * @brief Play a collection associated with the given handle in an
         *        Entity scope with the given gain.
         *
         * @param handle A handle to the collection to play.
         * @param entity The entity which is playing the collection.
         * @param userGain The gain of the sound.
         *
         * @return The channel the collection is played on. If the collection could not be
         *         played, an invalid Channel is returned.
         */
        Channel Play(CollectionHandle handle, const Entity& entity, float userGain);

        /**
         * @brief Play a sound associated with the given handle in the
         *        World scope.
         *
         * @param handle A handle to the sound to play.
         *
         * @return The channel the sound is played on. If the sound could not be
         *         played, an invalid Channel is returned.
         */
        Channel Play(SoundHandle handle);

        /**
         * @brief Play a sound associated with the given handle in the
         *        World scope at the given location.
         *
         * @param handle A handle to the sound to play.
         * @param location The location on which play the sound.
         *
         * @return The channel the sound is played on. If the sound could not be
         *         played, an invalid Channel is returned.
         */
        Channel Play(SoundHandle handle, const hmm_vec3& location);

        /**
         * @brief Play a sound associated with the given handle in the
         *        World scope at the given location with the given gain.
         *
         * @param handle A handle to the sound to play.
         * @param location The location on which play the sound.
         * @param userGain The gain of the sound.
         *
         * @return The channel the sound is played on. If the sound could not be
         *         played, an invalid Channel is returned.
         */
        Channel Play(SoundHandle handle, const hmm_vec3& location, float userGain);

        /**
         * @brief Play a sound associated with the given sound handle in an
         *        Entity scope.
         *
         * @param handle A handle to the sound to play.
         * @param entity The entity which is playing the sound.
         *
         * @return The channel the sound is played on. If the sound could not be
         *         played, an invalid Channel is returned.
         */
        Channel Play(SoundHandle handle, const Entity& entity);

        /**
         * @brief Play a sound associated with the given sound handle in an
         *        Entity scope with the given gain.
         *
         * @param handle A handle to the sound to play.
         * @param entity The entity which is playing the sound.
         * @param userGain The gain of the sound.
         *
         * @return The channel the sound is played on. If the sound could not be
         *         played, an invalid Channel is returned.
         */
        Channel Play(SoundHandle handle, const Entity& entity, float userGain);

        /**
         * @brief Play a sound or a collection associated with the given name.
         *
         * Sound objects will have the priority on collection objects.
         *
         * Note: Playing an object with its handle is faster than using the
         * name as using the name requires a map lookup internally.
         *
         * @param name The name of the object to play.
         *
         * @return The channel the sound is played on. If the object could not be
         *         played, an invalid Channel is returned.
         */
        Channel Play(const std::string& name);

        /**
         * @brief Play a sound or a collection associated with the given name.
         *
         * Sound objects will have the priority on collection objects.
         *
         * Note: Playing an object with its handle is faster than using the
         * name as using the name requires a map lookup internally.
         *
         * @param name The name of the object to play.
         * @param location The location of the sound.
         *
         * @return The channel the sound is played on. If the object could not be
         *         played, an invalid Channel is returned.
         */
        Channel Play(const std::string& name, const hmm_vec3& location);

        /**
         * @brief Play a sound or a collection associated with the given name.
         *
         * Sound objects will have the priority on collection objects.
         *
         * Note: Playing an object with its handle is faster than using the
         * name as using the name requires a map lookup internally.
         *
         * @param name The name of the object to play.
         * @param location The location of the sound.
         * @param userGain The gain of the sound.
         *
         * @return The channel the sound is played on. If the object could not be
         *         played, an invalid Channel is returned.
         */
        Channel Play(const std::string& name, const hmm_vec3& location, float userGain);

        /**
         * @brief Play a sound or a collection associated with the given name.
         *
         * Sound objects will have the priority on collection objects.
         *
         * Note: Playing an object with its handle is faster than using the
         * name as using the name requires a map lookup internally.
         *
         * @param name The name of the object to play.
         * @param entity The entity which is playing the sound.
         *
         * @return The channel the sound is played on. If the object could not be
         *         played, an invalid Channel is returned.
         */
        Channel Play(const std::string& name, const Entity& entity);

        /**
         * @brief Play a sound or a collection associated with the given name.
         *
         * Sound objects will have the priority on collection objects.
         *
         * Note: Playing an object with its handle is faster than using the
         * name as using the name requires a map lookup internally.
         *
         * @param name The name of the object to play.
         * @param entity The entity which is playing the sound.
         * @param userGain The gain of the sound.
         *
         * @return The channel the sound is played on. If the object could not be
         *         played, an invalid Channel is returned.
         */
        Channel Play(const std::string& name, const Entity& entity, float userGain);

        /**
         * @brief Play a sound or a collection associated with the given ID in the
         * World scope, at the origin of the world.
         *
         * Sound objects will have the priority on collection objects.
         *
         * Note: Playing an object with its handle is faster than using the
         * ID as using the ID requires a map lookup internally.
         *
         * @param id The ID of the object to play.
         *
         * @return The channel the sound is played on. If the object could not be
         *         played, an invalid Channel is returned.
         */
        Channel Play(AmObjectID id);

        /**
         * @brief Play a sound or a collection associated with the given ID in the
         * World scope, at the given location.
         *
         * Sound objects will have the priority on collection objects.
         *
         * Note: Playing an object with its handle is faster than using the
         * ID as using the ID requires a map lookup internally.
         *
         * @param id The ID of the object to play.
         * @param location The location of the sound.
         *
         * @return The channel the sound is played on. If the sound could not be
         *         played, an invalid Channel is returned.
         */
        Channel Play(AmObjectID id, const hmm_vec3& location);

        /**
         * @brief Play a sound or a collection associated with the given ID in the
         * World scope, at the given location, and with the given gain.
         *
         * Sound objects will have the priority on collection objects.
         *
         * Note: Playing an object with its handle is faster than using the
         * ID as using the ID requires a map lookup internally.
         *
         * @param id The ID of the object to play.
         * @param location The location of the sound.
         * @param userGain The gain of the sound.
         *
         * @return The channel the sound is played on. If the sound could not be
         *         played, an invalid Channel is returned.
         */
        Channel Play(AmObjectID id, const hmm_vec3& location, float userGain);

        /**
         * @brief Play a sound or a collection associated with the given ID in an
         * Entity scope.
         *
         * Sound objects will have the priority on collection objects.
         *
         * Note: Playing an object with its handle is faster than using the
         * ID as using the ID requires a map lookup internally.
         *
         * @param id The ID of the object to play.
         * @param entity The entity which is playing the sound.
         *
         * @return The channel the sound is played on. If the sound could not be
         *         played, an invalid Channel is returned.
         */
        Channel Play(AmObjectID id, const Entity& entity);

        /**
         * @brief Play a sound or a collection associated with the given ID in an
         * Entity scope with the given gain.
         *
         * Sound objects will have the priority on collection objects.
         *
         * Note: Playing an object with its handle is faster than using the
         * ID as using the ID requires a map lookup internally.
         *
         * @param id The ID of the object to play.
         * @param entity The entity which is playing the sound.
         * @param userGain The gain of the sound.
         *
         * @return The channel the sound is played on. If the sound could not be
         *         played, an invalid Channel is returned.
         */
        Channel Play(AmObjectID id, const Entity& entity, float userGain);

        /**
         * @brief Stops all playing sound objects.
         */
        void StopAll();

        /**
         * @brief Trigger the event associated with the given sound handle at the
         *        given location.
         *
         * @param handle A handle of the event to trigger.
         * @param entity The which trigger the event.
         *
         * @return The triggered event.
         */
        EventCanceler Trigger(EventHandle handle, const Entity& entity);

        /**
         * @brief Trigger the event associated with the specified event name at
         *        the given location.
         *
         * Note: Triggering an event with its EventHandle is faster than using the
         * event name as using the name requires a map lookup internally.
         *
         * @param name The name of event to trigger.
         * @param entity The which trigger the event.
         *
         * @return The triggered event.
         */
        EventCanceler Trigger(const std::string& name, const Entity& entity);

        /**
         * @brief Set the active state of the defined Switch.
         *
         * @param handle The handle of the Switch.
         * @param stateId The ID of the active state to set.
         */
        void SetSwitchState(SwitchHandle handle, AmObjectID stateId);

        /**
         * @brief Set the active state of the defined Switch.
         *
         * @param handle The handle of the Switch.
         * @param stateName The name of the active state to set.
         */
        void SetSwitchState(SwitchHandle handle, const std::string& stateName);

        /**
         * @brief Set the active state of the defined Switch.
         *
         * @param handle The handle of the Switch.
         * @param state The active state to set.
         */
        void SetSwitchState(SwitchHandle handle, const SwitchState& state);

        /**
         * @brief Set the active state of the defined Switch.
         *
         * @param id The ID of the Switch to update.
         * @param stateId The ID of the active state to set.
         */
        void SetSwitchState(AmSwitchID id, AmObjectID stateId);

        /**
         * @brief Set the active state of the defined Switch.
         *
         * @param id The ID of the Switch to update.
         * @param stateName The name of the active state to set.
         */
        void SetSwitchState(AmSwitchID id, const std::string& stateName);

        /**
         * @brief Set the active state of the defined Switch.
         *
         * @param id The ID of the Switch to update.
         * @param state The active state to set.
         */
        void SetSwitchState(AmSwitchID id, const SwitchState& state);

        /**
         * @brief Set the active state of the defined Switch.
         *
         * @param name The name of the Switch to update.
         * @param stateId The ID of the active state to set.
         */
        void SetSwitchState(const std::string name, AmObjectID stateId);

        /**
         * @brief Set the active state of the defined Switch.
         *
         * @param name The name of the Switch to update.
         * @param stateName The name of the active state to set.
         */
        void SetSwitchState(const std::string name, const std::string& stateName);

        /**
         * @brief Set the active state of the defined Switch.
         *
         * @param name The name of the Switch to update.
         * @param state The active state to set.
         */
        void SetSwitchState(const std::string name, const SwitchState& state);

        /**
         * @brief Set the value of a RTPC.
         *
         * @param handle The RTPC handle to update.
         * @param value The value to set to the RTPC.
         */
        void SetRtpcValue(RtpcHandle handle, double value);

        /**
         * @brief Set the value of a RTPC.
         *
         * @param id The ID of the RTPC to update.
         * @param value The value to set to the RTPC.
         */
        void SetRtpcValue(AmRtpcID id, double value);

        /**
         * @brief Set the value of a RTPC.
         *
         * @param name THe name of the RTPC to update.
         * @param value The value to set to the RTPC.
         */
        void SetRtpcValue(const std::string& name, double value);

        /**
         * @brief Gets the version structure.
         *
         * @return The version string structure
         */
        [[nodiscard]] const struct Version* Version() const;

        /**
         * @brief Gets the current state of this Engine.
         *
         * @return The current state of this Engine.
         */
        [[nodiscard]] EngineInternalState* GetState() const;

        [[nodiscard]] const EngineConfigDefinition* GetEngineConfigDefinition() const;

        /**
         * @brief Gets the audio driver used by this Engine.
         *
         * @return The audio driver.
         */
        [[nodiscard]] Driver* GetDriver() const;

        /**
         * @brief Returns an unique instance of the Amplitude Engine.
         */
        [[nodiscard]] static Engine* GetInstance();

    private:
        Channel PlayScopedSwitchContainer(
            SwitchContainerHandle handle, const Entity& entity, const hmm_vec3& location, float userGain) const;
        Channel PlayScopedCollection(CollectionHandle handle, const Entity& entity, const hmm_vec3& location, float userGain) const;
        Channel PlayScopedSound(SoundHandle handle, const Entity& entity, const hmm_vec3& location, float userGain) const;

        // Hold the engine config file contents.
        std::string _configSrc;

        // The current state of the engine.
        EngineInternalState* _state;

        // The file loader implementation
        FileLoader _loader;

        // The audio driver used by the engine.
        Driver* _audioDriver;
    };

} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_ENGINE_H
