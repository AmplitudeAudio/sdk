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
#include <SparkyStudios/Audio/Amplitude/Core/Thread.h>
#include <SparkyStudios/Audio/Amplitude/Core/Version.h>

#include <SparkyStudios/Audio/Amplitude/Sound/Attenuation.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Collection.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Effect.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Rtpc.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Sound.h>
#include <SparkyStudios/Audio/Amplitude/Sound/SoundObject.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Switch.h>
#include <SparkyStudios/Audio/Amplitude/Sound/SwitchContainer.h>

#ifndef AM_BUILDSYSTEM_BUILDING_PLUGIN

/**
 * @brief Macro to get the current Amplitude engine instance.
 */
#define amEngine SparkyStudios::Audio::Amplitude::Engine::GetInstance()

#endif // AM_BUILDSYSTEM_BUILDING_PLUGIN

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
    class AM_API_PUBLIC Engine
    {
    private:
        /**
         * @brief Construct an uninitialized Engine.
         */
        Engine();

    public:
        ~Engine();

        /**
         * @brief Initializes the Amplitude engine.
         *
         * @param configFile The path to the configuration file.
         *
         * @return Whether the engine has been successfully initialized.
         */
        bool Initialize(const AmOsString& configFile);

        /**
         * @brief Initializes the Amplitude engine.
         *
         * @param config A pointer to a loaded EngineConfigDefinition object.
         *
         * @return Whether the engine has been successfully initialized.
         */
        bool Initialize(const EngineConfigDefinition* config);

        /**
         * @brief Deinitializes the Amplitude engine.
         *
         * @return \c true when the engine has been successfully deinitialized, \c false otherwise.
         */
        bool Deinitialize();

        /**
         * @brief Checks if the Amplitude engine has been successfully initialized.
         *
         * @return @c true if the engine has been successfully initialized, @c false otherwise.
         */
        [[nodiscard]] bool IsInitialized() const;

        /**
         * @brief Set a file system implementation to be used by the engine.
         *
         * @param fs The file system implementation.
         */
        void SetFileSystem(FileSystem* fs);

        /**
         * @brief Gets the file system implementation used by the engine.
         *
         * @return The file system implementation used by the engine.
         */
        [[nodiscard]] const FileSystem* GetFileSystem() const;

        /**
         * @brief Updates the engine state for the given number of milliseconds.
         *
         * @param delta The number of milliseconds since the last frame.
         */
        void AdvanceFrame(AmTime delta) const;

        /**
         * @brief Gets the total elapsed time in milliseconds since the start of the engine.
         *
         * @return The total elapsed time in milliseconds since the start of the engine.
         */
        [[nodiscard]] AmTime GetTotalTime() const;

        /**
         * @brief Loads a sound bank from a file. Queue the sound files in that sound
         *        bank for loading. Call StartLoadSoundFiles() to trigger loading
         *        of the sound files on a separate thread.
         *
         * @param filename The path to the sound bank file.
         *
         * @return @c true when the sound bank is successfully loaded, @c false otherwise.
         */
        bool LoadSoundBank(const AmOsString& filename);

        /**
         * @brief Loads a sound bank from a file. Queue the sound files in that sound
         *        bank for loading. Call StartLoadSoundFiles() to trigger loading
         *        of the sound files on a separate thread.
         *
         * @param filename The path to the sound bank file.
         * @param outID The ID of the loaded sound bank.
         *
         * @return @c true when the sound bank is successfully loaded, @c false otherwise.
         */
        bool LoadSoundBank(const AmOsString& filename, AmBankID& outID);

        /**
         * @brief Loads a sound bank from memory. Queue the sound files in that sound
         *        bank for loading. Call StartLoadSoundFiles() to trigger loading
         *        of the sound files on a separate thread.
         *
         * @param fileData The sound bank data to be loaded.
         *
         * @return @c true when the sound bank is successfully loaded, @c false otherwise.
         */
        bool LoadSoundBankFromMemory(const char* fileData);

        /**
         * @brief Loads a sound bank from memory. Queue the sound files in that sound
         *        bank for loading. Call StartLoadSoundFiles() to trigger loading
         *        of the sound files on a separate thread.
         *
         * @param fileData The sound bank data to be loaded.
         * @param outID The ID of the loaded sound bank.
         *
         * @return @c true when the sound bank is successfully loaded, @c false otherwise.
         */
        bool LoadSoundBankFromMemory(const char* fileData, AmBankID& outID);

        /**
         * @brief Loads a sound bank from memory. Queue the sound files in that sound
         *        bank for loading. Call StartLoadSoundFiles() to trigger loading
         *        of the sound files on a separate thread.
         *
         * @param ptr The pointer to the sound bank data to be loaded.
         * @param size The size of the memory to read.
         *
         * @return @c true when the sound bank is successfully loaded, @c false otherwise.
         */
        bool LoadSoundBankFromMemoryView(void* ptr, AmSize size);

        /**
         * @brief Loads a sound bank from memory. Queue the sound files in that sound
         *        bank for loading. Call StartLoadSoundFiles() to trigger loading
         *        of the sound files on a separate thread.
         *
         * @param ptr The pointer to the sound bank data to be loaded.
         * @param size The size of the memory to read.
         * @param outID The ID of the loaded sound bank.
         *
         * @return @c true when the sound bank is successfully loaded, @c false otherwise.
         */
        bool LoadSoundBankFromMemoryView(void* ptr, AmSize size, AmBankID& outID);

        /**
         * @brief Unloads a sound bank.
         *
         * @param filename The file to unload.
         */
        void UnloadSoundBank(const AmOsString& filename);

        /**
         * @brief Unloads a sound bank.
         *
         * @param id The sound bank id to unload.
         */
        void UnloadSoundBank(AmBankID id);

        /**
         * @brief Unloads all the loaded sound banks.
         */
        void UnloadSoundBanks();

        /**
         * @brief Opens the file system, usually in a separate thread.
         */
        void StartOpenFileSystem();

        /**
         * @brief Returns @c true if the file system has been fully loaded. Must call
         *        StartOpenFileSystem() first.
         */
        bool TryFinalizeOpenFileSystem();

        /**
         * @brief Closes the file system, usually in a separate thread.
         */
        void StartCloseFileSystem();

        /**
         * @brief Return @c true if the file system has been fully closed. Must call
         *        StartCloseFileSystem() first.
         */
        bool TryFinalizeCloseFileSystem();

        /**
         * @brief Starts the loading of sound files referenced in loaded sound banks.
         *
         * This process will run in another thread. You must call TryFinalizeLoadSoundFiles() to
         * know when the loading has completed, and to release used resources.
         */
        void StartLoadSoundFiles();

        /**
         * @brief Checks if the loading of sound files has been completed, and releases used resources.
         *
         * @return @c true when the sound files have been successfully loaded, @c false otherwise.
         */
        bool TryFinalizeLoadSoundFiles();

        /**
         * @brief Gets a @c SwitchContainerHandle given its name as defined in its JSON data.
         *
         * @param name The unique name as defined in the JSON data.
         */
        [[nodiscard]] SwitchContainerHandle GetSwitchContainerHandle(const AmString& name) const;

        /**
         * @brief Gets a @C SwitchContainerHandle given its ID as defined in its JSON data.
         *
         * @param id The unique ID as defined in the JSON data.
         */
        [[nodiscard]] SwitchContainerHandle GetSwitchContainerHandle(AmSwitchContainerID id) const;

        /**
         * @brief Gets a @c SwitchContainerHandle given its SwitchContainerDefinition filename.
         *
         * @param filename The filename containing the flatbuffer binary data.
         */
        [[nodiscard]] SwitchContainerHandle GetSwitchContainerHandleFromFile(const AmOsString& filename) const;

        /**
         * @brief Gets a @c CollectionHandle given its name as defined in its JSON data.
         *
         * @param name The unique name as defined in the JSON data.
         */
        [[nodiscard]] CollectionHandle GetCollectionHandle(const AmString& name) const;

        /**
         * @brief Gets a @c CollectionHandle given its ID as defined in its JSON data.
         *
         * @param id The unique ID as defined in the JSON data.
         */
        [[nodiscard]] CollectionHandle GetCollectionHandle(AmCollectionID id) const;

        /**
         * @brief Gets a @c CollectionHandle given its CollectionDefinition filename.
         *
         * @param filename The filename containing the flatbuffer binary data.
         */
        [[nodiscard]] CollectionHandle GetCollectionHandleFromFile(const AmOsString& filename) const;

        /**
         * @brief Gets a @c SoundHandle given its name as defined in its JSON data.
         *
         * @param name The unique name as defined in the JSON data.
         */
        [[nodiscard]] SoundHandle GetSoundHandle(const AmString& name) const;

        /**
         * @brief Gets a @c SoundHandle given its ID as defined in its JSON data.
         *
         * @param id The unique ID as defined in the JSON data.
         */
        [[nodiscard]] SoundHandle GetSoundHandle(AmSoundID id) const;

        /**
         * @brief Gets a @c SoundHandle given its SoundDefinition filename.
         *
         * @param filename The filename containing the flatbuffer binary data.
         */
        [[nodiscard]] SoundHandle GetSoundHandleFromFile(const AmOsString& filename) const;

        /**
         * @brief Gets a @c SoundObjectHandle given its name as defined in its JSON data.
         *
         * Can be a @c SwitchContainerHandle, @c CollectionHandle, or @c SoundHandle.
         *
         * @param name The unique name as defined in the JSON data.
         */
        [[nodiscard]] SoundObjectHandle GetSoundObjectHandle(const AmString& name) const;

        /**
         * @brief Gets a @c SoundObjectHandle given its ID as defined in its JSON data.
         *
         * Can be a @c SwitchContainerHandle, @c CollectionHandle, or @c SoundHandle.
         *
         * @param id The unique ID as defined in the JSON data.
         */
        [[nodiscard]] SoundObjectHandle GetSoundObjectHandle(AmSoundID id) const;

        /**
         * @brief Gets a @c SoundObjectHandle given its definition filename.
         *
         * Can be a @c SwitchContainerHandle, @c CollectionHandle, or @c SoundHandle.
         *
         * @param filename The filename containing the flatbuffer binary data.
         */
        [[nodiscard]] SoundObjectHandle GetSoundObjectHandleFromFile(const AmOsString& filename) const;

        /**
         * @brief Gets an @c EventHandle given its name as defined in its JSON data.
         *
         * @param name The unique name as defined in the JSON data.
         */
        [[nodiscard]] EventHandle GetEventHandle(const AmString& name) const;

        /**
         * @brief Gets an @c EventHandle given its ID as defined in its JSON data.
         *
         * @param id The unique ID as defined in the JSON data.
         */
        [[nodiscard]] EventHandle GetEventHandle(AmEventID id) const;

        /**
         * @brief Gets an @c EventHandle given its EventDefinition filename.
         *
         * @param filename The filename containing the flatbuffer binary data.
         */
        [[nodiscard]] EventHandle GetEventHandleFromFile(const AmOsString& filename) const;

        /**
         * @brief Gets an @c AttenuationHandle given its name as defined in its JSON data.
         *
         * @param name The unique name as defined in the JSON data.
         */
        [[nodiscard]] AttenuationHandle GetAttenuationHandle(const AmString& name) const;

        /**
         * @brief Gets an @c AttenuationHandle given its ID as defined in its JSON data.
         *
         * @param id The unique ID as defined in the JSON data.
         */
        [[nodiscard]] AttenuationHandle GetAttenuationHandle(AmAttenuationID id) const;

        /**
         * @brief Gets an @c AttenuationHandle given its AttenuationDefinition filename.
         *
         * @param filename The filename containing the flatbuffer binary data.
         */
        [[nodiscard]] AttenuationHandle GetAttenuationHandleFromFile(const AmOsString& filename) const;

        /**
         * @brief Gets a @c SwitchHandle given its name as defined in its JSON data.
         *
         * @param name The unique name as defined in the JSON data.
         */
        [[nodiscard]] SwitchHandle GetSwitchHandle(const AmString& name) const;

        /**
         * @brief Gets a @c SwitchHandle given its ID as defined in its JSON data.
         *
         * @param id The unique ID as defined in the JSON data.
         */
        [[nodiscard]] SwitchHandle GetSwitchHandle(AmSwitchID id) const;

        /**
         * @brief Gets a @c SwitchHandle given its SwitchDefinition filename.
         *
         * @param filename The filename containing the flatbuffer binary data.
         */
        [[nodiscard]] SwitchHandle GetSwitchHandleFromFile(const AmOsString& filename) const;

        /**
         * @brief Gets a @c RtpcHandle given its name as defined in its JSON data.
         *
         * @param name The unique name as defined in the JSON data.
         */
        [[nodiscard]] RtpcHandle GetRtpcHandle(const AmString& name) const;

        /**
         * @brief Gets an @c RtpcHandle given its ID as defined in its JSON data.
         *
         * @param id The unique ID as defined in the JSON data.
         */
        [[nodiscard]] RtpcHandle GetRtpcHandle(AmRtpcID id) const;

        /**
         * @brief Gets an @c RtpcHandle given its RtpcDefinition filename.
         *
         * @param filename The filename containing the flatbuffer binary data.
         */
        [[nodiscard]] RtpcHandle GetRtpcHandleFromFile(const AmOsString& filename) const;

        /**
         * @brief Gets an @c EffectHandle given its name as defined in its JSON data.
         *
         * @param name The unique name as defined in the JSON data.
         */
        [[nodiscard]] EffectHandle GetEffectHandle(const AmString& name) const;

        /**
         * @brief Gets an @c EffectHandle given its ID as defined in its JSON data.
         *
         * @param id The unique ID as defined in the JSON data.
         */
        [[nodiscard]] EffectHandle GetEffectHandle(AmEffectID id) const;

        /**
         * @brief Gets an @c EffectHandle given its EffectDefinition filename.
         *
         * @param filename The filename containing the flatbuffer binary data.
         */
        [[nodiscard]] EffectHandle GetEffectHandleFromFile(const AmOsString& filename) const;

        /**
         * @brief Adjusts the master gain of the mixer.
         *
         * @param gain The master gain.
         */
        void SetMasterGain(AmReal32 gain) const;

        /**
         * @brief Gets the mixer master gain.
         *
         * @return The mixer master gain.
         */
        [[nodiscard]] AmReal32 GetMasterGain() const;

        /**
         * @brief Mutes the engine, but keep processing audio.
         *
         * @param mute Whether to mute or unmute the engine.
         */
        void SetMute(bool mute) const;

        /**
         * @brief Whether the engine is currently muted.
         *
         * @return @c true if the engine is muted, @c false otherwise.
         */
        [[nodiscard]] bool IsMuted() const;

        /**
         * @brief Pauses or resumes all playing sounds and streams.
         *
         * @param pause Whether to pause or resume the engine.
         */
        void Pause(bool pause) const;

        /**
         * @brief Whether the engine is currently paused.
         *
         * @return @c true if the engine is currently paused, @c false otherwise.
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
         * @brief Returns a @c Listener object storing the state of the default
         * audio listener.
         *
         * @return An initialized Listener object if a default listener was set,
         * otherwise an unitialized Listener object.
         */
        [[nodiscard]] Listener GetDefaultListener() const;

        /**
         * @brief Initializes and returns a @c Listener.
         *
         * @param id The sound listener ID.
         *
         * @return An initialized Listener.
         */
        [[nodiscard]] Listener AddListener(AmListenerID id) const;

        /**
         * @brief Returns the @c Listener with the given ID.
         *
         * @param id The sound listener ID.
         *
         * @return An initialized Listener if that one has been registered before,
         * otherwise an unitialized Listener.
         */
        [[nodiscard]] Listener GetListener(AmListenerID id) const;

        /**
         * @brief Removes a @c Listener given its ID.
         *
         * @param id The ID of the Listener to be removed.
         */
        void RemoveListener(AmListenerID id) const;

        /**
         * @brief Removes a @c Listener given its handle.
         *
         * @param listener The Listener to be removed.
         */
        void RemoveListener(const Listener* listener) const;

        /**
         * @brief Initializes and returns an @c Entity.
         *
         * @param id The game entity ID.
         *
         * @return An initialized Entity.
         */
        [[nodiscard]] Entity AddEntity(AmEntityID id) const;

        /**
         * @brief Returns the @c Entity with the given ID.
         *
         * @param id The game entity ID.
         *
         * @return An initialized Entity if that one has been registered before,
         * otherwise an uninitialized Entity.
         */
        [[nodiscard]] Entity GetEntity(AmEntityID id) const;

        /**
         * @brief Removes an @c Entity.
         *
         * @param entity The game entity to be removed.
         */
        void RemoveEntity(const Entity* entity) const;

        /**
         * @brief Removes an @c Entity given its ID.
         *
         * @param id The ID of the game entity to be removed.
         */
        void RemoveEntity(AmEntityID id) const;

        /**
         * @brief Initializes and return an @c Environment.
         *
         * @param id The game environment ID.
         *
         * @return An initialized Environment.
         */
        [[nodiscard]] Environment AddEnvironment(AmEnvironmentID id) const;

        /**
         * @brief Returns the @c Environment with the given ID.
         *
         * @param id The game environment ID.
         *
         * @return An initialized Environment if that one has been registered before,
         * otherwise an uninitialized Environment.
         */
        [[nodiscard]] Environment GetEnvironment(AmEnvironmentID id) const;

        /**
         * @brief Removes an @c Environment.
         *
         * @param Environment The game environment to be removed.
         */
        void RemoveEnvironment(const Environment* Environment) const;

        /**
         * @brief Removes an @c Environment given its ID.
         *
         * @param id The ID of the game environment to be removed.
         */
        void RemoveEnvironment(AmEnvironmentID id) const;

        /**
         * @brief Returns the @c Bus with the specified name.
         *
         * @param name The name of the bus.
         *
         * @return A valid bus if found, otherwise an invalid bus.
         */
        [[nodiscard]] Bus FindBus(const AmString& name) const;

        /**
         * @brief Returns the @c Bus with the given ID.
         *
         * @param id The ID of the bus.
         *
         * @return A valid bus if found, otherwise an invalid bus.
         */
        [[nodiscard]] Bus FindBus(AmBusID id) const;

        /**
         * @brief Plays a switch container associated with the given handle in the World scope.
         *
         * @param handle A handle to the switch container to play.
         *
         * @return The channel the switch container is played on. If the switch container could not be
         *         played, an invalid Channel is returned.
         */
        Channel Play(SwitchContainerHandle handle) const;

        /**
         * @brief Plays a switch container associated with the given handle in the World scope at the
         * given location.
         *
         * @param handle A handle to the switch container to play.
         * @param location The location on which play the switch container.
         *
         * @return The channel the switch container is played on. If the switch container could not be
         *         played, an invalid Channel is returned.
         */
        Channel Play(SwitchContainerHandle handle, const AmVec3& location) const;

        /**
         * @brief Plays a switch container associated with the given handle in the World scope at the
         * given location with the given gain.
         *
         * @param handle A handle to the switch container to play.
         * @param location The location on which play the switch container.
         * @param userGain The gain of the sound.
         *
         * @return The channel the switch container is played on. If the switch container could not be
         *         played, an invalid Channel is returned.
         */
        Channel Play(SwitchContainerHandle handle, const AmVec3& location, AmReal32 userGain) const;

        /**
         * @brief Plays a switch container associated with the given handle in an Entity scope.
         *
         * @param handle A handle to the switch container to play.
         * @param entity The entity which is playing the switch container.
         *
         * @return The channel the switch container is played on. If the switch container could not be
         *         played, an invalid Channel is returned.
         */
        Channel Play(SwitchContainerHandle handle, const Entity& entity) const;

        /**
         * @brief Plays a switch container associated with the given handle in an Entity scope with the
         * given gain.
         *
         * @param handle A handle to the switch container to play.
         * @param entity The entity which is playing the switch container.
         * @param userGain The gain of the sound.
         *
         * @return The channel the switch container is played on. If the switch container could not be
         *         played, an invalid Channel is returned.
         */
        Channel Play(SwitchContainerHandle handle, const Entity& entity, AmReal32 userGain) const;

        /**
         * @brief Plays a collection associated with the given handle in the World scope.
         *
         * @param handle A handle to the collection to play.
         *
         * @return The channel the collection is played on. If the collection could not be
         *         played, an invalid Channel is returned.
         */
        Channel Play(CollectionHandle handle) const;

        /**
         * @brief Plays a collection associated with the given handle in the World scope at
         * the given location.
         *
         * @param handle A handle to the collection to play.
         * @param location The location on which play the collection.
         *
         * @return The channel the collection is played on. If the collection could not be
         *         played, an invalid Channel is returned.
         */
        Channel Play(CollectionHandle handle, const AmVec3& location) const;

        /**
         * @brief Plays a collection associated with the given handle in the location with
         * the given gain.
         *
         * @param handle A handle to the collection to play.
         * @param location The location on which play the collection.
         * @param userGain The gain of the sound.
         *
         * @return The channel the collection is played on. If the collection could not be
         *         played, an invalid Channel is returned.
         */
        Channel Play(CollectionHandle handle, const AmVec3& location, AmReal32 userGain) const;

        /**
         * @brief Plays a collection associated with the given handle in the Entity scope.
         *
         * @param handle A handle to the collection to play.
         * @param entity The entity which is playing the collection.
         *
         * @return The channel the collection is played on. If the collection could not be
         * played, an invalid Channel is returned.
         */
        Channel Play(CollectionHandle handle, const Entity& entity) const;

        /**
         * @brief Plays a collection associated with the given handle in an Entity scope
         * with the given gain.
         *
         * @param handle A handle to the collection to play.
         * @param entity The entity which is playing the collection.
         * @param userGain The gain of the sound.
         *
         * @return The channel the collection is played on. If the collection could not be
         * played, an invalid Channel is returned.
         */
        Channel Play(CollectionHandle handle, const Entity& entity, AmReal32 userGain) const;

        /**
         * @brief Plays a sound associated with the given handle in the World scope.
         *
         * @param handle A handle to the sound to play.
         *
         * @return The channel the sound is played on. If the sound could not be
         * played, an invalid Channel is returned.
         */
        Channel Play(SoundHandle handle) const;

        /**
         * @brief Plays a sound associated with the given handle in the World scope
         * at the given location.
         *
         * @param handle A handle to the sound to play.
         * @param location The location on which play the sound.
         *
         * @return The channel the sound is played on. If the sound could not be
         * played, an invalid Channel is returned.
         */
        Channel Play(SoundHandle handle, const AmVec3& location) const;

        /**
         * @brief Plays a sound associated with the given handle in the World scope
         * at the given location with the given gain.
         *
         * @param handle A handle to the sound to play.
         * @param location The location on which play the sound.
         * @param userGain The gain of the sound.
         *
         * @return The channel the sound is played on. If the sound could not be
         * played, an invalid Channel is returned.
         */
        Channel Play(SoundHandle handle, const AmVec3& location, AmReal32 userGain) const;

        /**
         * @brief Plays a sound associated with the given sound handle in an Entity scope.
         *
         * @param handle A handle to the sound to play.
         * @param entity The entity which is playing the sound.
         *
         * @return The channel the sound is played on. If the sound could not be
         * played, an invalid Channel is returned.
         */
        Channel Play(SoundHandle handle, const Entity& entity) const;

        /**
         * @brief Plays a sound associated with the given sound handle in an Entity
         * scope with the given gain.
         *
         * @param handle A handle to the sound to play.
         * @param entity The entity which is playing the sound.
         * @param userGain The gain of the sound.
         *
         * @return The channel the sound is played on. If the sound could not be
         * played, an invalid Channel is returned.
         */
        Channel Play(SoundHandle handle, const Entity& entity, AmReal32 userGain) const;

        /**
         * @brief Plays a sound object associated with the given name in the World scope.
         *
         * @note Playing a sound object with its handle is faster than using the
         * name as using the name requires a map lookup internally.
         *
         * @param name The name of the sound object to play.
         *
         * @return The channel the sound is played on. If the object could not be
         * played, an invalid Channel is returned.
         */
        [[nodiscard]] Channel Play(const AmString& name) const;

        /**
         * @brief Plays a sound object associated with the given name in the World scope.
         *
         * @note Playing a sound object with its handle is faster than using the
         * name as using the name requires a map lookup internally.
         *
         * @param name The name of the sound object to play.
         * @param location The location of the sound.
         *
         * @return The channel the sound is played on. If the object could not be
         * played, an invalid Channel is returned.
         */
        [[nodiscard]] Channel Play(const AmString& name, const AmVec3& location) const;

        /**
         * @brief Plays a sound object associated with the given name in the World scope.
         *
         * @note Playing a sound object with its handle is faster than using the
         * name as using the name requires a map lookup internally.
         *
         * @param name The name of the sound object to play.
         * @param location The location of the sound.
         * @param userGain The gain of the sound.
         *
         * @return The channel the sound is played on. If the object could not be
         * played, an invalid Channel is returned.
         */
        [[nodiscard]] Channel Play(const AmString& name, const AmVec3& location, AmReal32 userGain) const;

        /**
         * @brief Plays a sound object associated with the given name in an Entity scope.
         *
         * @note Playing a sound object with its handle is faster than using the
         * name as using the name requires a map lookup internally.
         *
         * @param name The name of the sound object to play.
         * @param entity The entity which is playing the sound.
         *
         * @return The channel the sound is played on. If the object could not be
         * played, an invalid Channel is returned.
         */
        [[nodiscard]] Channel Play(const AmString& name, const Entity& entity) const;

        /**
         * @brief Plays a sound object associated with the given name in an Entity scope.
         *
         * @note Playing a sound object with its handle is faster than using the
         * name as using the name requires a map lookup internally.
         *
         * @param name The name of the sound object to play.
         * @param entity The entity which is playing the sound.
         * @param userGain The gain of the sound.
         *
         * @return The channel the sound is played on. If the object could not be
         * played, an invalid Channel is returned.
         */
        [[nodiscard]] Channel Play(const AmString& name, const Entity& entity, AmReal32 userGain) const;

        /**
         * @brief Plays a sound object associated with the given ID in the
         * World scope, at the origin of the world.
         *
         * @note Playing a sound object with its handle is faster than using the
         * ID as using the ID requires a map lookup internally.
         *
         * @param id The ID of the sound object to play.
         *
         * @return The channel the sound is played on. If the object could not be
         * played, an invalid Channel is returned.
         */
        [[nodiscard]] Channel Play(AmObjectID id) const;

        /**
         * @brief Plays a sound object associated with the given ID in the
         * World scope, at the given location.
         *
         * @note Playing a sound object with its handle is faster than using the
         * ID as using the ID requires a map lookup internally.
         *
         * @param id The ID of the sound object to play.
         * @param location The location of the sound.
         *
         * @return The channel the sound is played on. If the sound could not be
         * played, an invalid Channel is returned.
         */
        [[nodiscard]] Channel Play(AmObjectID id, const AmVec3& location) const;

        /**
         * @brief Plays a sound object associated with the given ID in the
         * World scope, at the given location, and with the given gain.
         *
         * @note Playing an object with its handle is faster than using the
         * ID as using the ID requires a map lookup internally.
         *
         * @param id The ID of the sound object to play.
         * @param location The location of the sound.
         * @param userGain The gain of the sound.
         *
         * @return The channel the sound is played on. If the sound could not be
         * played, an invalid Channel is returned.
         */
        [[nodiscard]] Channel Play(AmObjectID id, const AmVec3& location, AmReal32 userGain) const;

        /**
         * @brief Plays a sound or a collection associated with the given ID in an Entity scope.
         *
         * @note Playing an object with its handle is faster than using the
         * ID as using the ID requires a map lookup internally.
         *
         * @param id The ID of the sound object to play.
         * @param entity The entity which is playing the sound.
         *
         * @return The channel the sound is played on. If the sound could not be
         * played, an invalid Channel is returned.
         */
        [[nodiscard]] Channel Play(AmObjectID id, const Entity& entity) const;

        /**
         * @brief Plays a sound object associated with the given ID in an Entity
         * scope with the given gain.
         *
         * @note Playing an object with its handle is faster than using the
         * ID as using the ID requires a map lookup internally.
         *
         * @param id The ID of the sound object to play.
         * @param entity The entity which is playing the sound.
         * @param userGain The gain of the sound.
         *
         * @return The channel the sound is played on. If the sound could not be
         * played, an invalid Channel is returned.
         */
        [[nodiscard]] Channel Play(AmObjectID id, const Entity& entity, AmReal32 userGain) const;

        /**
         * @brief Stops all playing sound objects.
         */
        void StopAll() const;

        /**
         * @brief Triggers the event associated to the given handle.
         *
         * @param handle The handle of the event to trigger.
         * @param entity The entity in which trigger the event.
         *
         * @return An @c EventCanceler object which may be used to cancel the execution of the event..
         */
        [[nodiscard]] EventCanceler Trigger(EventHandle handle, const Entity& entity) const;

        /**
         * @brief Triggers the event associated to the given handle.
         *
         * @note Triggering an event with its EventHandle is faster than using the
         * event name as using the name requires a map lookup internally.
         *
         * @param name The name of event to trigger.
         * @param entity The entity in which trigger the event.
         *
         * @return An @c EventCanceler object which may be used to cancel the execution of the event..
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

#pragma region Plugins Management

        /**
         * @brief Loads a plugin library from the given path.
         *
         * @param pluginLibraryName The name of the plugin library to load.
         *
         * @return A handle to the loaded plugin library.
         */
        static AmVoidPtr LoadPlugin(const AmOsString& pluginLibraryName);

        /**
         * @brief Adds a path in the plugins search paths list.
         *
         * @param path The path to add in the plugins search paths list.
         */
        static void AddPluginSearchPath(const AmOsString& path);

        /**
         * @brief Removes a path from the plugins search paths list.
         *
         * @param path The path to remove from the plugins search path list.
         */
        static void RemovePluginSearchPath(const AmOsString& path);

        /**
         * @brief Register all default plugins.
         */
        static bool RegisterDefaultPlugins();

        /**
         * @brief Unregister all default plugins.
         */
        static bool UnregisterDefaultPlugins();

#pragma endregion

        /**
         * @brief Returns an unique instance of the Amplitude Engine.
         */
        [[nodiscard]] static Engine* GetInstance();

        /**
         * @brief Destroys the unique instance of the Amplitude Engine.
         */
        static void DestroyInstance();

    private:
        Channel PlayScopedSwitchContainer(
            SwitchContainerHandle handle, const Entity& entity, const AmVec3& location, AmReal32 userGain) const;
        Channel PlayScopedCollection(CollectionHandle handle, const Entity& entity, const AmVec3& location, AmReal32 userGain) const;
        Channel PlayScopedSound(SoundHandle handle, const Entity& entity, const AmVec3& location, AmReal32 userGain) const;

        // The lis of paths in which search for plugins.
        static std::set<AmOsString> _pluginSearchPaths;

        // Hold the engine config file contents.
        AmString _configSrc;

        // The current state of the engine.
        EngineInternalState* _state;

        // The default audio listener.
        ListenerInternalState* _defaultListener;

        // The file loader implementation.
        FileSystem* _fs;

        // The audio driver used by the engine.
        Driver* _audioDriver;

        // The thread pool used to load audio files.
        AmUniquePtr<MemoryPoolKind::Engine, Thread::Pool> _soundLoaderThreadPool;
    };

} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_ENGINE_H
