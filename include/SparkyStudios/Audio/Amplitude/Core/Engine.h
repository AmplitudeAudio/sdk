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
#include <SparkyStudios/Audio/Amplitude/Core/Device.h>
#include <SparkyStudios/Audio/Amplitude/Core/Driver.h>
#include <SparkyStudios/Audio/Amplitude/Core/Entity.h>
#include <SparkyStudios/Audio/Amplitude/Core/Environment.h>
#include <SparkyStudios/Audio/Amplitude/Core/Event.h>
#include <SparkyStudios/Audio/Amplitude/Core/Listener.h>
#include <SparkyStudios/Audio/Amplitude/Core/Version.h>

#include <SparkyStudios/Audio/Amplitude/Sound/Attenuation.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Collection.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Effect.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Rtpc.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Sound.h>
#include <SparkyStudios/Audio/Amplitude/Sound/SoundObject.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Switch.h>
#include <SparkyStudios/Audio/Amplitude/Sound/SwitchContainer.h>

/**
 * @brief Macro to get the current Amplitude engine instance.
 */
#define amEngine SparkyStudios::Audio::Amplitude::Engine::GetInstance()

namespace SparkyStudios::Audio::Amplitude
{
    struct EngineConfigDefinition;

    struct EngineInternalState;

    typedef SwitchContainer* SwitchContainerHandle;
    typedef Collection* CollectionHandle;
    typedef Sound* SoundHandle;
    typedef SoundObject* SoundObjectHandle;
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
        bool Initialize(const AmOsString& configFile);

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
        [[nodiscard]] bool IsInitialized() const;

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
        void AdvanceFrame(AmTime delta) const;

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
        bool LoadSoundBank(const AmOsString& filename);

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
        bool LoadSoundBank(const AmOsString& filename, AmBankID& outID);

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
        void UnloadSoundBank(const AmOsString& filename);

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
        [[nodiscard]] SwitchContainerHandle GetSwitchContainerHandle(const AmString& name) const;

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
        [[nodiscard]] SwitchContainerHandle GetSwitchContainerHandleFromFile(const AmOsString& filename) const;

        /**
         * @brief Get a CollectionHandle given its name as defined in its JSON data.
         *
         * @param name The unique name as defined in the JSON data.
         */
        [[nodiscard]] CollectionHandle GetCollectionHandle(const AmString& name) const;

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
        [[nodiscard]] CollectionHandle GetCollectionHandleFromFile(const AmOsString& filename) const;

        /**
         * @brief Get a SoundHandle given its name as defined in its JSON data.
         *
         * @param name The unique name as defined in the JSON data.
         */
        [[nodiscard]] SoundHandle GetSoundHandle(const AmString& name) const;

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
        [[nodiscard]] SoundHandle GetSoundHandleFromFile(const AmOsString& filename) const;

        /**
         * @brief Get a SoundObjectHandle given its name as defined in its JSON data.
         *
         * Can be a SwitchContainerHandle, CollectionHandle, or SoundHandle.
         *
         * @param name The unique name as defined in the JSON data.
         */
        [[nodiscard]] SoundObjectHandle GetSoundObjectHandle(const AmString& name) const;

        /**
         * @brief Get a SoundObjectHandle given its ID as defined in its JSON data.
         *
         * Can be a SwitchContainerHandle, CollectionHandle, or SoundHandle.
         *
         * @param id The unique ID as defined in the JSON data.
         */
        [[nodiscard]] SoundObjectHandle GetSoundObjectHandle(AmSoundID id) const;

        /**
         * @brief Get a SoundObjectHandle given its definition filename.
         *
         * Can be a SwitchContainerHandle, CollectionHandle, or SoundHandle.
         *
         * @param filename The filename containing the flatbuffer binary data.
         */
        [[nodiscard]] SoundObjectHandle GetSoundObjectHandleFromFile(const AmOsString& filename) const;

        /**
         * @brief Get an EventHandle given its name as defined in its JSON data.
         *
         * @param name The unique name as defined in the JSON data.
         */
        [[nodiscard]] EventHandle GetEventHandle(const AmString& name) const;

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
        [[nodiscard]] EventHandle GetEventHandleFromFile(const AmOsString& filename) const;

        /**
         * @brief Get an AttenuationHandle given its name as defined in its JSON data.
         *
         * @param name The unique name as defined in the JSON data.
         */
        [[nodiscard]] AttenuationHandle GetAttenuationHandle(const AmString& name) const;

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
        [[nodiscard]] AttenuationHandle GetAttenuationHandleFromFile(const AmOsString& filename) const;

        /**
         * @brief Get a SwitchHandle given its name as defined in its JSON data.
         *
         * @param name The unique name as defined in the JSON data.
         */
        [[nodiscard]] SwitchHandle GetSwitchHandle(const AmString& name) const;

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
        [[nodiscard]] SwitchHandle GetSwitchHandleFromFile(const AmOsString& filename) const;

        /**
         * @brief Get a RtpcHandle given its name as defined in its JSON data.
         *
         * @param name The unique name as defined in the JSON data.
         */
        [[nodiscard]] RtpcHandle GetRtpcHandle(const AmString& name) const;

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
        [[nodiscard]] RtpcHandle GetRtpcHandleFromFile(const AmOsString& filename) const;

        /**
         * @brief Get a EffectHandle given its name as defined in its JSON data.
         *
         * @param name The unique name as defined in the JSON data.
         */
        [[nodiscard]] EffectHandle GetEffectHandle(const AmString& name) const;

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
        [[nodiscard]] EffectHandle GetEffectHandleFromFile(const AmOsString& filename) const;

        /**
         * @brief Adjusts the gain on the master bus.
         *
         * @param gain the gain to apply to all buses.
         */
        void SetMasterGain(float gain) const;

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
        void SetMute(bool mute) const;

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
        void Pause(bool pause) const;

        /**
         * @brief Whether the Engine is currently paused.
         */
        [[nodiscard]] bool IsPaused() const;

        /**
         * @brief Sets the default sound listener.
         *
         * @param listener A valid and initialized Listener instance.
         */
        void SetDefaultListener(const Listener* listener);

        /**
         * @brief Sets the default sound listener.
         *
         * @param id A valid Listener ID.
         */
        void SetDefaultListener(AmListenerID id);

        /**
         * @brief Returns a Listener object storing the state of the default
         * audio listener.
         *
         * @return An initialized Listener object if a default listener was set,
         * otherwise an unitialized Listener object.
         */
        [[nodiscard]] Listener GetDefaultListener() const;

        /**
         * @brief Initialize and return a Listener.
         *
         * @param id The game Listener ID.
         *
         * @return An initialized Listener.
         */
        [[nodiscard]] Listener AddListener(AmListenerID id) const;

        /**
         * @brief Return the Listener with the given ID.
         *
         * @param id The game Listener ID.
         *
         * @return An initialized Listener if that one has been registered before,
         * otherwise an unitialized Listener.
         */
        [[nodiscard]] Listener GetListener(AmListenerID id) const;

        /**
         * @brief Remove a Listener given its ID.
         *
         * @param id The ID of the Listener to be removed.
         */
        void RemoveListener(AmListenerID id) const;

        /**
         * @brief Remove a Listener.
         *
         * @param listener The Listener to be removed.
         */
        void RemoveListener(const Listener* listener) const;

        /**
         * @brief Initialize and return an Entity.
         *
         * @param id The game Entity ID.
         *
         * @return An initialized Entity.
         */
        [[nodiscard]] Entity AddEntity(AmEntityID id) const;

        /**
         * @brief Return the Entity with the given ID.
         *
         * @param id The game Entity ID.
         *
         * @return An initialized Entity if that one has been registered before,
         * otherwise an uninitialized Entity.
         */
        [[nodiscard]] Entity GetEntity(AmEntityID id) const;

        /**
         * @brief Remove an Entity.
         *
         * @param entity The Entity to be removed.
         */
        void RemoveEntity(const Entity* entity) const;

        /**
         * @brief Remove an Entity given its ID.
         *
         * @param id The ID of the Entity to be removed.
         */
        void RemoveEntity(AmEntityID id) const;

        /**
         * @brief Initialize and return an Environment.
         *
         * @param id The game Environment ID.
         *
         * @return An initialized Environment.
         */
        [[nodiscard]] Environment AddEnvironment(AmEnvironmentID id) const;

        /**
         * @brief Return the Environment with the given ID.
         *
         * @param id The game Environment ID.
         *
         * @return An initialized Environment if that one has been registered before,
         * otherwise an uninitialized Environment.
         */
        [[nodiscard]] Environment GetEnvironment(AmEnvironmentID id) const;

        /**
         * @brief Remove an Environment.
         *
         * @param Environment The Environment to be removed.
         */
        void RemoveEnvironment(const Environment* Environment) const;

        /**
         * @brief Remove an Environment given its ID.
         *
         * @param id The ID of the Environment to be removed.
         */
        void RemoveEnvironment(AmEnvironmentID id) const;

        /**
         * @brief Returns the bus with the specified name.
         *
         * @param name The name of the bus.
         *
         * @return A valid bus if found, otherwise an invalid bus.
         */
        [[nodiscard]] Bus FindBus(const AmString& name) const;

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
        Channel Play(SwitchContainerHandle handle) const;

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
        Channel Play(SwitchContainerHandle handle, const AmVec3& location) const;

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
        Channel Play(SwitchContainerHandle handle, const AmVec3& location, float userGain) const;

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
        Channel Play(SwitchContainerHandle handle, const Entity& entity) const;

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
        Channel Play(SwitchContainerHandle handle, const Entity& entity, float userGain) const;

        /**
         * @brief Play a collection associated with the given handle in the
         *        World scope.
         *
         * @param handle A handle to the collection to play.
         *
         * @return The channel the collection is played on. If the collection could not be
         *         played, an invalid Channel is returned.
         */
        Channel Play(CollectionHandle handle) const;

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
        Channel Play(CollectionHandle handle, const AmVec3& location) const;

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
        Channel Play(CollectionHandle handle, const AmVec3& location, float userGain) const;

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
        Channel Play(CollectionHandle handle, const Entity& entity) const;

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
        Channel Play(CollectionHandle handle, const Entity& entity, float userGain) const;

        /**
         * @brief Play a sound associated with the given handle in the
         *        World scope.
         *
         * @param handle A handle to the sound to play.
         *
         * @return The channel the sound is played on. If the sound could not be
         *         played, an invalid Channel is returned.
         */
        Channel Play(SoundHandle handle) const;

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
        Channel Play(SoundHandle handle, const AmVec3& location) const;

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
        Channel Play(SoundHandle handle, const AmVec3& location, float userGain) const;

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
        Channel Play(SoundHandle handle, const Entity& entity) const;

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
        Channel Play(SoundHandle handle, const Entity& entity, float userGain) const;

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
        [[nodiscard]] Channel Play(const AmString& name) const;

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
        [[nodiscard]] Channel Play(const AmString& name, const AmVec3& location) const;

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
        [[nodiscard]] Channel Play(const AmString& name, const AmVec3& location, float userGain) const;

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
        [[nodiscard]] Channel Play(const AmString& name, const Entity& entity) const;

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
        [[nodiscard]] Channel Play(const AmString& name, const Entity& entity, float userGain) const;

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
        [[nodiscard]] Channel Play(AmObjectID id) const;

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
        [[nodiscard]] Channel Play(AmObjectID id, const AmVec3& location) const;

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
        [[nodiscard]] Channel Play(AmObjectID id, const AmVec3& location, float userGain) const;

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
        [[nodiscard]] Channel Play(AmObjectID id, const Entity& entity) const;

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
        [[nodiscard]] Channel Play(AmObjectID id, const Entity& entity, float userGain) const;

        /**
         * @brief Stops all playing sound objects.
         */
        void StopAll() const;

        /**
         * @brief Trigger the event associated with the given sound handle at the
         *        given location.
         *
         * @param handle A handle of the event to trigger.
         * @param entity The which trigger the event.
         *
         * @return The triggered event.
         */
        [[nodiscard]] EventCanceler Trigger(EventHandle handle, const Entity& entity) const;

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
        [[nodiscard]] EventCanceler Trigger(const AmString& name, const Entity& entity) const;

        /**
         * @brief Set the active state of the defined Switch.
         *
         * @param handle The handle of the Switch.
         * @param stateId The ID of the active state to set.
         */
        void SetSwitchState(SwitchHandle handle, AmObjectID stateId) const;

        /**
         * @brief Set the active state of the defined Switch.
         *
         * @param handle The handle of the Switch.
         * @param stateName The name of the active state to set.
         */
        void SetSwitchState(SwitchHandle handle, const AmString& stateName) const;

        /**
         * @brief Set the active state of the defined Switch.
         *
         * @param handle The handle of the Switch.
         * @param state The active state to set.
         */
        void SetSwitchState(SwitchHandle handle, const SwitchState& state) const;

        /**
         * @brief Set the active state of the defined Switch.
         *
         * @param id The ID of the Switch to update.
         * @param stateId The ID of the active state to set.
         */
        void SetSwitchState(AmSwitchID id, AmObjectID stateId) const;

        /**
         * @brief Set the active state of the defined Switch.
         *
         * @param id The ID of the Switch to update.
         * @param stateName The name of the active state to set.
         */
        void SetSwitchState(AmSwitchID id, const AmString& stateName) const;

        /**
         * @brief Set the active state of the defined Switch.
         *
         * @param id The ID of the Switch to update.
         * @param state The active state to set.
         */
        void SetSwitchState(AmSwitchID id, const SwitchState& state) const;

        /**
         * @brief Set the active state of the defined Switch.
         *
         * @param name The name of the Switch to update.
         * @param stateId The ID of the active state to set.
         */
        void SetSwitchState(const AmString& name, AmObjectID stateId) const;

        /**
         * @brief Set the active state of the defined Switch.
         *
         * @param name The name of the Switch to update.
         * @param stateName The name of the active state to set.
         */
        void SetSwitchState(const AmString& name, const AmString& stateName) const;

        /**
         * @brief Set the active state of the defined Switch.
         *
         * @param name The name of the Switch to update.
         * @param state The active state to set.
         */
        void SetSwitchState(const AmString& name, const SwitchState& state) const;

        /**
         * @brief Set the value of a RTPC.
         *
         * @param handle The RTPC handle to update.
         * @param value The value to set to the RTPC.
         */
        void SetRtpcValue(RtpcHandle handle, double value) const;

        /**
         * @brief Set the value of a RTPC.
         *
         * @param id The ID of the RTPC to update.
         * @param value The value to set to the RTPC.
         */
        void SetRtpcValue(AmRtpcID id, double value) const;

        /**
         * @brief Set the value of a RTPC.
         *
         * @param name THe name of the RTPC to update.
         * @param value The value to set to the RTPC.
         */
        void SetRtpcValue(const AmString& name, double value) const;

        /**
         * @brief Gets the version structure.
         *
         * @return The version string structure
         */
        [[nodiscard]] const struct Version* Version() const;

        [[nodiscard]] const EngineConfigDefinition* GetEngineConfigDefinition() const;

        /**
         * @brief Gets the audio driver used by this Engine.
         *
         * @return The audio driver.
         */
        [[nodiscard]] Driver* GetDriver() const;

#pragma region Amplimix

        /**
         * @brief Gets the mixer instance.
         *
         * @return The Amplimix mixer instance.
         */
        [[nodiscard]] Mixer* GetMixer() const;

#pragma endregion

#pragma region Engine State

        /**
         * @brief Gets the current state of this Engine.
         *
         * @return The current state of this Engine.
         */
        [[nodiscard]] EngineInternalState* GetState() const;

        /**
         * @brief Get the current speed of sound.
         *
         * @return The speed of sound.
         */
        [[nodiscard]] AmReal32 GetSoundSpeed() const;

        /**
         * @brief Get the engine Doppler factor.
         *
         * @return The Doppler factor.
         */
        [[nodiscard]] AmReal32 GetDopplerFactor() const;

        /**
         * @brief Get the number of samples to process in one stream.
         *
         * @return The number of samples per stream.
         */
        [[nodiscard]] AmUInt32 GetSamplesPerStream() const;

        /**
         * @brief Checks whether the game is tracking environment amounts
         * himself. This is useful for engines like O3DE.
         *
         * @return Whether the game is tracking environment amounts.
         */
        [[nodiscard]] bool IsGameTrackingEnvironmentAmounts() const;

        /**
         * @brief Gets the maximum number of listeners handled by the engine.
         *
         * @return The maximum number of listeners.
         */
        [[nodiscard]] AmUInt32 GetMaxListenersCount() const;

        /**
         * @brief Gets the maximum number of game entities handled by the engine. This value
         * does not reflect the maximum number of simultaneous sound handled by the engine.
         *
         * @return The maximum number of game entities.
         */
        [[nodiscard]] AmUInt32 GetMaxEntitiesCount() const;

#pragma endregion

        /**
         * @brief Returns an unique instance of the Amplitude Engine.
         */
        [[nodiscard]] static Engine* GetInstance();

    private:
        Channel PlayScopedSwitchContainer(SwitchContainerHandle handle, const Entity& entity, const AmVec3& location, float userGain) const;
        Channel PlayScopedCollection(CollectionHandle handle, const Entity& entity, const AmVec3& location, float userGain) const;
        Channel PlayScopedSound(SoundHandle handle, const Entity& entity, const AmVec3& location, float userGain) const;

        // Hold the engine config file contents.
        AmString _configSrc;

        // The current state of the engine.
        EngineInternalState* _state;

        // The default audio listener
        ListenerInternalState* _defaultListener;

        // The file loader implementation
        FileLoader _loader;

        // The audio driver used by the engine.
        Driver* _audioDriver;
    };

} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_ENGINE_H
