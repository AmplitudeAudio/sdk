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

#ifndef _AM_CORE_ENGINE_H
#define _AM_CORE_ENGINE_H

#include <queue>

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

#include <SparkyStudios/Audio/Amplitude/Core/Device.h>
#include <SparkyStudios/Audio/Amplitude/Core/Driver.h>
#include <SparkyStudios/Audio/Amplitude/Core/Entity.h>
#include <SparkyStudios/Audio/Amplitude/Core/Environment.h>
#include <SparkyStudios/Audio/Amplitude/Core/Event.h>
#include <SparkyStudios/Audio/Amplitude/Core/Listener.h>
#include <SparkyStudios/Audio/Amplitude/Core/Playback/Bus.h>
#include <SparkyStudios/Audio/Amplitude/Core/Playback/Channel.h>
#include <SparkyStudios/Audio/Amplitude/Core/Version.h>

#include <SparkyStudios/Audio/Amplitude/Mixer/Amplimix.h>
#include <SparkyStudios/Audio/Amplitude/Mixer/Pipeline.h>

#include <SparkyStudios/Audio/Amplitude/Sound/Attenuation.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Collection.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Effect.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Rtpc.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Sound.h>
#include <SparkyStudios/Audio/Amplitude/Sound/SoundObject.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Switch.h>
#include <SparkyStudios/Audio/Amplitude/Sound/SwitchContainer.h>

#if !defined(AM_BUILDSYSTEM_BUILDING_PLUGIN) && !defined(AM_BUILDSYSTEM_BUILDING_AMPLITUDE)

/**
 * @brief Macro to get the current Amplitude engine instance.
 */
#define amEngine SparkyStudios::Audio::Amplitude::Engine::GetInstance()

#endif // AM_BUILDSYSTEM_BUILDING_PLUGIN

namespace SparkyStudios::Audio::Amplitude
{
    typedef SwitchContainer* SwitchContainerHandle;
    typedef Collection* CollectionHandle;
    typedef Sound* SoundHandle;
    typedef SoundObject* SoundObjectHandle;
    typedef Event* EventHandle;
    typedef Pipeline* PipelineHandle;
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
    public:
        virtual ~Engine() = default;

#pragma region Miscalaneous

        /**
         * @brief Gets the version structure.
         *
         * @return The version string structure
         */
        [[nodiscard]] virtual const struct Version* Version() const = 0;

#pragma endregion

#pragma region Initialization

        /**
         * @brief Initializes the Amplitude engine.
         *
         * @param configFile The path to the configuration file.
         *
         * @return Whether the engine has been successfully initialized.
         */
        virtual bool Initialize(const AmOsString& configFile) = 0;

        /**
         * @brief Deinitializes the Amplitude engine.
         *
         * @return @c true when the engine has been successfully deinitialized, @c false otherwise.
         */
        virtual bool Deinitialize() = 0;

        /**
         * @brief Checks if the Amplitude engine has been successfully initialized.
         *
         * @return @c true if the engine has been successfully initialized, @c false otherwise.
         */
        [[nodiscard]] virtual bool IsInitialized() const = 0;

#pragma endregion

#pragma region FileSystem Management

        /**
         * @brief Set a file system implementation to be used by the engine.
         *
         * @param fs The file system implementation.
         */
        virtual void SetFileSystem(FileSystem* fs) = 0;

        /**
         * @brief Gets the file system implementation used by the engine.
         *
         * @return The file system implementation used by the engine.
         */
        [[nodiscard]] virtual const FileSystem* GetFileSystem() const = 0;

        /**
         * @brief Opens the file system, usually in a separate thread.
         */
        virtual void StartOpenFileSystem() = 0;

        /**
         * @brief Returns @c true if the file system has been fully loaded. Must call
         * @c Engine::StartOpenFileSystem() first.
         */
        virtual bool TryFinalizeOpenFileSystem() = 0;

        /**
         * @brief Closes the file system, usually in a separate thread.
         */
        virtual void StartCloseFileSystem() = 0;

        /**
         * @brief Return @c true if the file system has been fully closed. Must call
         * @c Engine::StartCloseFileSystem() first.
         */
        virtual bool TryFinalizeCloseFileSystem() = 0;

#pragma endregion

#pragma region Runtime Loop

        /**
         * @brief Updates the engine state for the given number of milliseconds.
         *
         * @param delta The number of milliseconds since the last frame.
         */
        virtual void AdvanceFrame(AmTime delta) const = 0;

        /**
         * @brief Executes the given callback on the next frame.
         *
         * @param callback The callback to be called when the next frame is ready.
         */
        virtual void OnNextFrame(std::function<void(AmTime delta)> callback) const = 0;

        /**
         * @brief Gets the total elapsed time in milliseconds since the start of the engine.
         *
         * @return The total elapsed time in milliseconds since the start of the engine.
         */
        [[nodiscard]] virtual AmTime GetTotalTime() const = 0;

#pragma endregion

#pragma region SoundBank Management

        /**
         * @brief Loads a sound bank from a file. Queue the sound files in that sound
         * bank for loading. Call @c Engine::StartLoadSoundFiles() to trigger loading
         * of the sound files on a separate thread.
         *
         * @param filename The path to the sound bank file.
         *
         * @return @c true when the sound bank is successfully loaded, @c false otherwise.
         */
        virtual bool LoadSoundBank(const AmOsString& filename) = 0;

        /**
         * @brief Loads a sound bank from a file. Queue the sound files in that sound
         * bank for loading. Call @c Engine::StartLoadSoundFiles() to trigger loading
         * of the sound files on a separate thread.
         *
         * @param filename The path to the sound bank file.
         * @param outID The ID of the loaded sound bank.
         *
         * @return @c true when the sound bank is successfully loaded, @c false otherwise.
         */
        virtual bool LoadSoundBank(const AmOsString& filename, AmBankID& outID) = 0;

        /**
         * @brief Loads a sound bank from memory. Queue the sound files in that sound
         * bank for loading. Call @c Engine::StartLoadSoundFiles() to trigger loading
         * of the sound files on a separate thread.
         *
         * @param fileData The sound bank data to be loaded.
         *
         * @return @c true when the sound bank is successfully loaded, @c false otherwise.
         */
        virtual bool LoadSoundBankFromMemory(const char* fileData) = 0;

        /**
         * @brief Loads a sound bank from memory. Queue the sound files in that sound
         * bank for loading. Call @c Engine::StartLoadSoundFiles() to trigger loading
         * of the sound files on a separate thread.
         *
         * @param fileData The sound bank data to be loaded.
         * @param outID The ID of the loaded sound bank.
         *
         * @return @c true when the sound bank is successfully loaded, @c false otherwise.
         */
        virtual bool LoadSoundBankFromMemory(const char* fileData, AmBankID& outID) = 0;

        /**
         * @brief Loads a sound bank from memory. Queue the sound files in that sound
         * bank for loading. Call @c Engine::StartLoadSoundFiles() to trigger loading
         * of the sound files on a separate thread.
         *
         * @param ptr The pointer to the sound bank data to be loaded.
         * @param size The size of the memory to read.
         *
         * @return @c true when the sound bank is successfully loaded, @c false otherwise.
         */
        virtual bool LoadSoundBankFromMemoryView(void* ptr, AmSize size) = 0;

        /**
         * @brief Loads a sound bank from memory. Queue the sound files in that sound
         * bank for loading. Call @c Engine::StartLoadSoundFiles() to trigger loading
         * of the sound files on a separate thread.
         *
         * @param ptr The pointer to the sound bank data to be loaded.
         * @param size The size of the memory to read.
         * @param outID The ID of the loaded sound bank.
         *
         * @return @c true when the sound bank is successfully loaded, @c false otherwise.
         */
        virtual bool LoadSoundBankFromMemoryView(void* ptr, AmSize size, AmBankID& outID) = 0;

        /**
         * @brief Unloads a sound bank.
         *
         * @param filename The file to unload.
         */
        virtual void UnloadSoundBank(const AmOsString& filename) = 0;

        /**
         * @brief Unloads a sound bank.
         *
         * @param id The sound bank id to unload.
         */
        virtual void UnloadSoundBank(AmBankID id) = 0;

        /**
         * @brief Unloads all the loaded sound banks.
         */
        virtual void UnloadSoundBanks() = 0;

        /**
         * @brief Starts the loading of sound files referenced in loaded sound banks.
         *
         * This process will run in another thread. You must call @c Engine::TryFinalizeLoadSoundFiles() to
         * know when the loading has completed, and to release used resources.
         */
        virtual void StartLoadSoundFiles() = 0;

        /**
         * @brief Checks if the loading of sound files has been completed, and releases used resources.
         *
         * @return @c true when the sound files have been successfully loaded, @c false otherwise.
         */
        virtual bool TryFinalizeLoadSoundFiles() = 0;

#pragma endregion

#pragma region Handles

        /**
         * @brief Gets a @c SwitchContainerHandle given its name as defined in its JSON data.
         *
         * @param name The unique name as defined in the JSON data.
         */
        [[nodiscard]] virtual SwitchContainerHandle GetSwitchContainerHandle(const AmString& name) const = 0;

        /**
         * @brief Gets a @C SwitchContainerHandle given its ID as defined in its JSON data.
         *
         * @param id The unique ID as defined in the JSON data.
         */
        [[nodiscard]] virtual SwitchContainerHandle GetSwitchContainerHandle(AmSwitchContainerID id) const = 0;

        /**
         * @brief Gets a @c SwitchContainerHandle given its definition file's name.
         *
         * @param filename The filename containing the flatbuffer binary data.
         */
        [[nodiscard]] virtual SwitchContainerHandle GetSwitchContainerHandleFromFile(const AmOsString& filename) const = 0;

        /**
         * @brief Gets a @c CollectionHandle given its name as defined in its JSON data.
         *
         * @param name The unique name as defined in the JSON data.
         */
        [[nodiscard]] virtual CollectionHandle GetCollectionHandle(const AmString& name) const = 0;

        /**
         * @brief Gets a @c CollectionHandle given its ID as defined in its JSON data.
         *
         * @param id The unique ID as defined in the JSON data.
         */
        [[nodiscard]] virtual CollectionHandle GetCollectionHandle(AmCollectionID id) const = 0;

        /**
         * @brief Gets a @c CollectionHandle given its definition file's name.
         *
         * @param filename The filename containing the flatbuffer binary data.
         */
        [[nodiscard]] virtual CollectionHandle GetCollectionHandleFromFile(const AmOsString& filename) const = 0;

        /**
         * @brief Gets a @c SoundHandle given its name as defined in its JSON data.
         *
         * @param name The unique name as defined in the JSON data.
         */
        [[nodiscard]] virtual SoundHandle GetSoundHandle(const AmString& name) const = 0;

        /**
         * @brief Gets a @c SoundHandle given its ID as defined in its JSON data.
         *
         * @param id The unique ID as defined in the JSON data.
         */
        [[nodiscard]] virtual SoundHandle GetSoundHandle(AmSoundID id) const = 0;

        /**
         * @brief Gets a @c SoundHandle given its defintion file's name.
         *
         * @param filename The filename containing the flatbuffer binary data.
         */
        [[nodiscard]] virtual SoundHandle GetSoundHandleFromFile(const AmOsString& filename) const = 0;

        /**
         * @brief Gets a @c SoundObjectHandle given its name as defined in its JSON data.
         *
         * Can be a @c SwitchContainerHandle, a @c CollectionHandle, or a @c SoundHandle.
         *
         * @param name The unique name as defined in the JSON data.
         */
        [[nodiscard]] virtual SoundObjectHandle GetSoundObjectHandle(const AmString& name) const = 0;

        /**
         * @brief Gets a @c SoundObjectHandle given its ID as defined in its JSON data.
         *
         * Can be a @c SwitchContainerHandle, a @c CollectionHandle, or a @c SoundHandle.
         *
         * @param id The unique ID as defined in the JSON data.
         */
        [[nodiscard]] virtual SoundObjectHandle GetSoundObjectHandle(AmSoundID id) const = 0;

        /**
         * @brief Gets a @c SoundObjectHandle given its definition file's name.
         *
         * Can be a @c SwitchContainerHandle, a @c CollectionHandle, or a @c SoundHandle.
         *
         * @param filename The filename containing the flatbuffer binary data.
         */
        [[nodiscard]] virtual SoundObjectHandle GetSoundObjectHandleFromFile(const AmOsString& filename) const = 0;

        /**
         * @brief Gets an @c EventHandle given its name as defined in its JSON data.
         *
         * @param name The unique name as defined in the JSON data.
         */
        [[nodiscard]] virtual EventHandle GetEventHandle(const AmString& name) const = 0;

        /**
         * @brief Gets an @c EventHandle given its ID as defined in its JSON data.
         *
         * @param id The unique ID as defined in the JSON data.
         */
        [[nodiscard]] virtual EventHandle GetEventHandle(AmEventID id) const = 0;

        /**
         * @brief Gets an @c EventHandle given its defintion file's name.
         *
         * @param filename The filename containing the flatbuffer binary data.
         */
        [[nodiscard]] virtual EventHandle GetEventHandleFromFile(const AmOsString& filename) const = 0;

        /**
         * @brief Gets an @c AttenuationHandle given its name as defined in its JSON data.
         *
         * @param name The unique name as defined in the JSON data.
         */
        [[nodiscard]] virtual AttenuationHandle GetAttenuationHandle(const AmString& name) const = 0;

        /**
         * @brief Gets an @c AttenuationHandle given its ID as defined in its JSON data.
         *
         * @param id The unique ID as defined in the JSON data.
         */
        [[nodiscard]] virtual AttenuationHandle GetAttenuationHandle(AmAttenuationID id) const = 0;

        /**
         * @brief Gets an @c AttenuationHandle given its definition file's name.
         *
         * @param filename The filename containing the flatbuffer binary data.
         */
        [[nodiscard]] virtual AttenuationHandle GetAttenuationHandleFromFile(const AmOsString& filename) const = 0;

        /**
         * @brief Gets a @c SwitchHandle given its name as defined in its JSON data.
         *
         * @param name The unique name as defined in the JSON data.
         */
        [[nodiscard]] virtual SwitchHandle GetSwitchHandle(const AmString& name) const = 0;

        /**
         * @brief Gets a @c SwitchHandle given its ID as defined in its JSON data.
         *
         * @param id The unique ID as defined in the JSON data.
         */
        [[nodiscard]] virtual SwitchHandle GetSwitchHandle(AmSwitchID id) const = 0;

        /**
         * @brief Gets a @c SwitchHandle given its defintion file's name.
         *
         * @param filename The filename containing the flatbuffer binary data.
         */
        [[nodiscard]] virtual SwitchHandle GetSwitchHandleFromFile(const AmOsString& filename) const = 0;

        /**
         * @brief Gets a @c RtpcHandle given its name as defined in its JSON data.
         *
         * @param name The unique name as defined in the JSON data.
         */
        [[nodiscard]] virtual RtpcHandle GetRtpcHandle(const AmString& name) const = 0;

        /**
         * @brief Gets an @c RtpcHandle given its ID as defined in its JSON data.
         *
         * @param id The unique ID as defined in the JSON data.
         */
        [[nodiscard]] virtual RtpcHandle GetRtpcHandle(AmRtpcID id) const = 0;

        /**
         * @brief Gets an @c RtpcHandle given its definition file's name.
         *
         * @param filename The filename containing the flatbuffer binary data.
         */
        [[nodiscard]] virtual RtpcHandle GetRtpcHandleFromFile(const AmOsString& filename) const = 0;

        /**
         * @brief Gets an @c EffectHandle given its name as defined in its JSON data.
         *
         * @param name The unique name as defined in the JSON data.
         */
        [[nodiscard]] virtual EffectHandle GetEffectHandle(const AmString& name) const = 0;

        /**
         * @brief Gets an @c EffectHandle given its ID as defined in its JSON data.
         *
         * @param id The unique ID as defined in the JSON data.
         */
        [[nodiscard]] virtual EffectHandle GetEffectHandle(AmEffectID id) const = 0;

        /**
         * @brief Gets an @c EffectHandle given its definition file's name.
         *
         * @param filename The filename containing the flatbuffer binary data.
         */
        [[nodiscard]] virtual EffectHandle GetEffectHandleFromFile(const AmOsString& filename) const = 0;

        /**
         * @brief Gets this @c PipelineHandle loaded in the engine configuration.
         */
        [[nodiscard]] virtual PipelineHandle GetPipelineHandle() const = 0;

#pragma endregion

#pragma region Master Gain& Mute State

        /**
         * @brief Adjusts the master gain of the mixer.
         *
         * @param gain The master gain.
         */
        virtual void SetMasterGain(AmReal32 gain) const = 0;

        /**
         * @brief Gets the mixer master gain.
         *
         * @return The mixer master gain.
         */
        [[nodiscard]] virtual AmReal32 GetMasterGain() const = 0;

        /**
         * @brief Mutes the engine, but keep processing audio.
         *
         * @param mute Whether to mute or unmute the engine.
         */
        virtual void SetMute(bool mute) const = 0;

        /**
         * @brief Whether the engine is currently muted.
         *
         * @return @c true if the engine is muted, @c false otherwise.
         */
        [[nodiscard]] virtual bool IsMuted() const = 0;

        /**
         * @brief Pauses or resumes all playing sounds and streams.
         *
         * @param pause Whether to pause or resume the engine.
         */
        virtual void Pause(bool pause) const = 0;

        /**
         * @brief Whether the engine is currently paused.
         *
         * @return @c true if the engine is currently paused, @c false otherwise.
         */
        [[nodiscard]] virtual bool IsPaused() const = 0;

#pragma endregion

#pragma region Listeners Management

        /**
         * @brief Sets the default sound listener.
         *
         * @param listener A valid and initialized @c Listener instance.
         */
        virtual void SetDefaultListener(const Listener* listener) = 0;

        /**
         * @brief Sets the default sound listener.
         *
         * @param id A valid @c Listener ID.
         */
        virtual void SetDefaultListener(AmListenerID id) = 0;

        /**
         * @brief Returns a @c Listener object storing the state of the default
         * audio listener.
         *
         * @return An initialized @c Listener object if a default listener was set,
         * otherwise an unitialized @c Listener object.
         */
        [[nodiscard]] virtual Listener GetDefaultListener() const = 0;

        /**
         * @brief Initializes and returns a @c Listener.
         *
         * @param id The sound listener ID.
         *
         * @return An initialized @c Listener.
         */
        [[nodiscard]] virtual Listener AddListener(AmListenerID id) const = 0;

        /**
         * @brief Returns the @c Listener with the given ID.
         *
         * @param id The sound listener ID.
         *
         * @return An initialized @c Listener if that one has been registered before,
         * otherwise an unitialized @c Listener.
         */
        [[nodiscard]] virtual Listener GetListener(AmListenerID id) const = 0;

        /**
         * @brief Removes a @c Listener given its ID.
         *
         * @param id The ID of the @c Listener to be removed.
         */
        virtual void RemoveListener(AmListenerID id) const = 0;

        /**
         * @brief Removes a @c Listener given its handle.
         *
         * @param listener The @c Listener to be removed.
         */
        virtual void RemoveListener(const Listener* listener) const = 0;

#pragma endregion

#pragma region Entities Management

        /**
         * @brief Initializes and returns an @c Entity.
         *
         * @param id The game entity ID.
         *
         * @return An initialized @c Entity.
         */
        [[nodiscard]] virtual Entity AddEntity(AmEntityID id) const = 0;

        /**
         * @brief Returns the @c Entity with the given ID.
         *
         * @param id The game entity ID.
         *
         * @return An initialized @c Entity if that one has been registered before,
         * otherwise an uninitialized @c Entity.
         */
        [[nodiscard]] virtual Entity GetEntity(AmEntityID id) const = 0;

        /**
         * @brief Removes an @c Entity.
         *
         * @param entity The game entity to be removed.
         */
        virtual void RemoveEntity(const Entity* entity) const = 0;

        /**
         * @brief Removes an @c Entity given its ID.
         *
         * @param id The ID of the game entity to be removed.
         */
        virtual void RemoveEntity(AmEntityID id) const = 0;

#pragma endregion

#pragma region Environments Management

        /**
         * @brief Initializes and return an @c Environment.
         *
         * @param id The game environment ID.
         *
         * @return An initialized @c Environment.
         */
        [[nodiscard]] virtual Environment AddEnvironment(AmEnvironmentID id) const = 0;

        /**
         * @brief Returns the @c Environment with the given ID.
         *
         * @param id The game environment ID.
         *
         * @return An initialized @c Environment if that one has been registered before,
         * otherwise an uninitialized @c Environment.
         */
        [[nodiscard]] virtual Environment GetEnvironment(AmEnvironmentID id) const = 0;

        /**
         * @brief Removes an @c Environment.
         *
         * @param Environment The game environment to be removed.
         */
        virtual void RemoveEnvironment(const Environment* Environment) const = 0;

        /**
         * @brief Removes an @c Environment given its ID.
         *
         * @param id The ID of the game environment to be removed.
         */
        virtual void RemoveEnvironment(AmEnvironmentID id) const = 0;

#pragma endregion

#pragma region Buses Management

        /**
         * @brief Returns the @c Bus with the specified name.
         *
         * @param name The name of the bus.
         *
         * @return A valid bus if found, otherwise an invalid bus.
         */
        [[nodiscard]] virtual Bus FindBus(const AmString& name) const = 0;

        /**
         * @brief Returns the @c Bus with the given ID.
         *
         * @param id The ID of the bus.
         *
         * @return A valid bus if found, otherwise an invalid bus.
         */
        [[nodiscard]] virtual Bus FindBus(AmBusID id) const = 0;

#pragma endregion

#pragma region Playback

        /**
         * @brief Plays a switch container associated with the given handle in the World scope.
         *
         * @param handle A handle to the switch container to play.
         *
         * @return The channel the switch container is played on. If the switch container could not be
         * played, an invalid @c Channel is returned.
         */
        [[nodiscard]] virtual Channel Play(SwitchContainerHandle handle) const = 0;

        /**
         * @brief Plays a switch container associated with the given handle in the World scope at the
         * given location.
         *
         * @param handle A handle to the switch container to play.
         * @param location The location on which play the switch container.
         *
         * @return The channel the switch container is played on. If the switch container could not be
         * played, an invalid @c Channel is returned.
         */
        [[nodiscard]] virtual Channel Play(SwitchContainerHandle handle, const AmVec3& location) const = 0;

        /**
         * @brief Plays a switch container associated with the given handle in the World scope at the
         * given location with the given gain.
         *
         * @param handle A handle to the switch container to play.
         * @param location The location on which play the switch container.
         * @param userGain The gain of the sound.
         *
         * @return The channel the switch container is played on. If the switch container could not be
         * played, an invalid @c Channel is returned.
         */
        [[nodiscard]] virtual Channel Play(SwitchContainerHandle handle, const AmVec3& location, AmReal32 userGain) const = 0;

        /**
         * @brief Plays a switch container associated with the given handle in an Entity scope.
         *
         * @param handle A handle to the switch container to play.
         * @param entity The entity which is playing the switch container.
         *
         * @return The channel the switch container is played on. If the switch container could not be
         * played, an invalid @c Channel is returned.
         */
        [[nodiscard]] virtual Channel Play(SwitchContainerHandle handle, const Entity& entity) const = 0;

        /**
         * @brief Plays a switch container associated with the given handle in an Entity scope with the
         * given gain.
         *
         * @param handle A handle to the switch container to play.
         * @param entity The entity which is playing the switch container.
         * @param userGain The gain of the sound.
         *
         * @return The channel the switch container is played on. If the switch container could not be
         * played, an invalid @c Channel is returned.
         */
        [[nodiscard]] virtual Channel Play(SwitchContainerHandle handle, const Entity& entity, AmReal32 userGain) const = 0;

        /**
         * @brief Plays a collection associated with the given handle in the World scope.
         *
         * @param handle A handle to the collection to play.
         *
         * @return The channel the collection is played on. If the collection could not be
         * played, an invalid @c Channel is returned.
         */
        [[nodiscard]] virtual Channel Play(CollectionHandle handle) const = 0;

        /**
         * @brief Plays a collection associated with the given handle in the World scope at
         * the given location.
         *
         * @param handle A handle to the collection to play.
         * @param location The location on which play the collection.
         *
         * @return The channel the collection is played on. If the collection could not be
         * played, an invalid @c Channel is returned.
         */
        [[nodiscard]] virtual Channel Play(CollectionHandle handle, const AmVec3& location) const = 0;

        /**
         * @brief Plays a collection associated with the given handle in the location with
         * the given gain.
         *
         * @param handle A handle to the collection to play.
         * @param location The location on which play the collection.
         * @param userGain The gain of the sound.
         *
         * @return The channel the collection is played on. If the collection could not be
         * played, an invalid @c Channel is returned.
         */
        [[nodiscard]] virtual Channel Play(CollectionHandle handle, const AmVec3& location, AmReal32 userGain) const = 0;

        /**
         * @brief Plays a collection associated with the given handle in the Entity scope.
         *
         * @param handle A handle to the collection to play.
         * @param entity The entity which is playing the collection.
         *
         * @return The channel the collection is played on. If the collection could not be
         * played, an invalid @c Channel is returned.
         */
        [[nodiscard]] virtual Channel Play(CollectionHandle handle, const Entity& entity) const = 0;

        /**
         * @brief Plays a collection associated with the given handle in an Entity scope
         * with the given gain.
         *
         * @param handle A handle to the collection to play.
         * @param entity The entity which is playing the collection.
         * @param userGain The gain of the sound.
         *
         * @return The channel the collection is played on. If the collection could not be
         * played, an invalid @c Channel is returned.
         */
        [[nodiscard]] virtual Channel Play(CollectionHandle handle, const Entity& entity, AmReal32 userGain) const = 0;

        /**
         * @brief Plays a sound associated with the given handle in the World scope.
         *
         * @param handle A handle to the sound to play.
         *
         * @return The channel the sound is played on. If the sound could not be
         * played, an invalid @c Channel is returned.
         */
        [[nodiscard]] virtual Channel Play(SoundHandle handle) const = 0;

        /**
         * @brief Plays a sound associated with the given handle in the World scope
         * at the given location.
         *
         * @param handle A handle to the sound to play.
         * @param location The location on which play the sound.
         *
         * @return The channel the sound is played on. If the sound could not be
         * played, an invalid @c Channel is returned.
         */
        [[nodiscard]] virtual Channel Play(SoundHandle handle, const AmVec3& location) const = 0;

        /**
         * @brief Plays a sound associated with the given handle in the World scope
         * at the given location with the given gain.
         *
         * @param handle A handle to the sound to play.
         * @param location The location on which play the sound.
         * @param userGain The gain of the sound.
         *
         * @return The channel the sound is played on. If the sound could not be
         * played, an invalid @c Channel is returned.
         */
        [[nodiscard]] virtual Channel Play(SoundHandle handle, const AmVec3& location, AmReal32 userGain) const = 0;

        /**
         * @brief Plays a sound associated with the given sound handle in an Entity scope.
         *
         * @param handle A handle to the sound to play.
         * @param entity The entity which is playing the sound.
         *
         * @return The channel the sound is played on. If the sound could not be
         * played, an invalid @c Channel is returned.
         */
        [[nodiscard]] virtual Channel Play(SoundHandle handle, const Entity& entity) const = 0;

        /**
         * @brief Plays a sound associated with the given sound handle in an Entity
         * scope with the given gain.
         *
         * @param handle A handle to the sound to play.
         * @param entity The entity which is playing the sound.
         * @param userGain The gain of the sound.
         *
         * @return The channel the sound is played on. If the sound could not be
         * played, an invalid @c Channel is returned.
         */
        [[nodiscard]] virtual Channel Play(SoundHandle handle, const Entity& entity, AmReal32 userGain) const = 0;

        /**
         * @brief Plays a sound object associated with the given name in the World scope.
         *
         * @note Playing a sound object with its handle is faster than using the
         * name as using the name requires a map lookup internally.
         *
         * @param name The name of the sound object to play.
         *
         * @return The channel the sound is played on. If the object could not be
         * played, an invalid @c Channel is returned.
         */
        [[nodiscard]] virtual Channel Play(const AmString& name) const = 0;

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
         * played, an invalid @c Channel is returned.
         */
        [[nodiscard]] virtual Channel Play(const AmString& name, const AmVec3& location) const = 0;

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
         * played, an invalid @c Channel is returned.
         */
        [[nodiscard]] virtual Channel Play(const AmString& name, const AmVec3& location, AmReal32 userGain) const = 0;

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
         * played, an invalid @c Channel is returned.
         */
        [[nodiscard]] virtual Channel Play(const AmString& name, const Entity& entity) const = 0;

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
         * played, an invalid @c Channel is returned.
         */
        [[nodiscard]] virtual Channel Play(const AmString& name, const Entity& entity, AmReal32 userGain) const = 0;

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
         * played, an invalid @c Channel is returned.
         */
        [[nodiscard]] virtual Channel Play(AmObjectID id) const = 0;

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
         * played, an invalid @c Channel is returned.
         */
        [[nodiscard]] virtual Channel Play(AmObjectID id, const AmVec3& location) const = 0;

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
         * played, an invalid @c Channel is returned.
         */
        [[nodiscard]] virtual Channel Play(AmObjectID id, const AmVec3& location, AmReal32 userGain) const = 0;

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
         * played, an invalid @c Channel is returned.
         */
        [[nodiscard]] virtual Channel Play(AmObjectID id, const Entity& entity) const = 0;

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
         * played, an invalid @c Channel is returned.
         */
        [[nodiscard]] virtual Channel Play(AmObjectID id, const Entity& entity, AmReal32 userGain) const = 0;

        /**
         * @brief Stops all playing sound objects.
         */
        virtual void StopAll() const = 0;

#pragma endregion

#pragma region Events

        /**
         * @brief Triggers the event associated to the given handle.
         *
         * @param handle The handle of the event to trigger.
         * @param entity The entity in which trigger the event.
         *
         * @return An @c EventCanceler object which may be used to cancel the execution of the event..
         */
        [[nodiscard]] virtual EventCanceler Trigger(EventHandle handle, const Entity& entity) const = 0;

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
        [[nodiscard]] virtual EventCanceler Trigger(const AmString& name, const Entity& entity) const = 0;

#pragma endregion

#pragma region Switches

        /**
         * @brief Set the active state of the defined @c Switch.
         *
         * @param handle The handle of the @c Switch.
         * @param stateId The ID of the active state to set.
         */
        virtual void SetSwitchState(SwitchHandle handle, AmObjectID stateId) const = 0;

        /**
         * @brief Set the active state of the defined @c Switch.
         *
         * @param handle The handle of the @c Switch.
         * @param stateName The name of the active state to set.
         */
        virtual void SetSwitchState(SwitchHandle handle, const AmString& stateName) const = 0;

        /**
         * @brief Set the active state of the defined @c Switch.
         *
         * @param handle The handle of the @c Switch.
         * @param state The active state to set.
         */
        virtual void SetSwitchState(SwitchHandle handle, const SwitchState& state) const = 0;

        /**
         * @brief Set the active state of the defined @c Switch.
         *
         * @param id The ID of the @c Switch to update.
         * @param stateId The ID of the active state to set.
         */
        virtual void SetSwitchState(AmSwitchID id, AmObjectID stateId) const = 0;

        /**
         * @brief Set the active state of the defined @c Switch.
         *
         * @param id The ID of the @c Switch to update.
         * @param stateName The name of the active state to set.
         */
        virtual void SetSwitchState(AmSwitchID id, const AmString& stateName) const = 0;

        /**
         * @brief Set the active state of the defined @c Switch.
         *
         * @param id The ID of the @c Switch to update.
         * @param state The active state to set.
         */
        virtual void SetSwitchState(AmSwitchID id, const SwitchState& state) const = 0;

        /**
         * @brief Set the active state of the defined @c Switch.
         *
         * @param name The name of the @c Switch to update.
         * @param stateId The ID of the active state to set.
         */
        virtual void SetSwitchState(const AmString& name, AmObjectID stateId) const = 0;

        /**
         * @brief Set the active state of the defined @c Switch.
         *
         * @param name The name of the @c Switch to update.
         * @param stateName The name of the active state to set.
         */
        virtual void SetSwitchState(const AmString& name, const AmString& stateName) const = 0;

        /**
         * @brief Set the active state of the defined @c Switch.
         *
         * @param name The name of the @c Switch to update.
         * @param state The active state to set.
         */
        virtual void SetSwitchState(const AmString& name, const SwitchState& state) const = 0;

#pragma endregion

#pragma region RTPC

        /**
         * @brief Set the value of a @c RTPC.
         *
         * @param handle The @c RTPC handle to update.
         * @param value The value to set to the @c RTPC.
         */
        virtual void SetRtpcValue(RtpcHandle handle, double value) const = 0;

        /**
         * @brief Set the value of a @c RTPC.
         *
         * @param id The ID of the @c RTPC to update.
         * @param value The value to set to the @c RTPC.
         */
        virtual void SetRtpcValue(AmRtpcID id, double value) const = 0;

        /**
         * @brief Set the value of a @c RTPC.
         *
         * @param name THe name of the @c RTPC to update.
         * @param value The value to set to the @c RTPC.
         */
        virtual void SetRtpcValue(const AmString& name, double value) const = 0;

#pragma endregion

#pragma region Driver

        /**
         * @brief Gets the audio driver used by this Engine.
         *
         * @return The audio driver.
         */
        [[nodiscard]] virtual Driver* GetDriver() const = 0;

#pragma endregion

#pragma region Amplimix

        /**
         * @brief Gets the mixer instance.
         *
         * @return The Amplimix mixer instance.
         */
        [[nodiscard]] virtual Amplimix* GetMixer() const = 0;

#pragma endregion

#pragma region Engine State& Configuration

        /**
         * @brief Get the current speed of sound.
         *
         * @return The speed of sound.
         */
        [[nodiscard]] virtual AmReal32 GetSoundSpeed() const = 0;

        /**
         * @brief Get the engine Doppler factor.
         *
         * @return The Doppler factor.
         */
        [[nodiscard]] virtual AmReal32 GetDopplerFactor() const = 0;

        /**
         * @brief Get the number of samples to process in one stream.
         *
         * @return The number of samples per stream.
         */
        [[nodiscard]] virtual AmUInt32 GetSamplesPerStream() const = 0;

        /**
         * @brief Checks whether the game is tracking environment amounts
         * himself. This is useful for engines like O3DE.
         *
         * @return Whether the game is tracking environment amounts.
         */
        [[nodiscard]] virtual bool IsGameTrackingEnvironmentAmounts() const = 0;

        /**
         * @brief Gets the maximum number of listeners handled by the engine.
         *
         * @return The maximum number of listeners.
         */
        [[nodiscard]] virtual AmUInt32 GetMaxListenersCount() const = 0;

        /**
         * @brief Gets the maximum number of game entities handled by the engine. This value
         * does not reflect the maximum number of simultaneous sound handled by the engine.
         *
         * @return The maximum number of game entities.
         */
        [[nodiscard]] virtual AmUInt32 GetMaxEntitiesCount() const = 0;

        /**
         * @brief Gets the occlusion coefficient curve defined in the loaded engine configuration.
         *
         * @return The occlusion coefficient curve.
         */
        [[nodiscard]] virtual const Curve& GetOcclusionCoefficientCurve() const = 0;

        /**
         * @brief Gets the occlusion gain curve defined in the loaded engine configuration.
         *
         * @return The occlusion gain curve.
         */
        [[nodiscard]] virtual const Curve& GetOcclusionGainCurve() const = 0;

        /**
         * @brief Gets the obstruction low pass curve defined in the loaded engine configuration.
         *
         * @return The obstruction low pass curve.
         */
        [[nodiscard]] virtual const Curve& GetObstructionLowPassCurve() const = 0;

        /**
         * @brief Gets the obstruction gain curve defined in the loaded engine configuration.
         *
         * @return The obstruction gain curve.
         */
        [[nodiscard]] virtual const Curve& GetObstructionGainCurve() const = 0;

        /**
         * @brief Gets the panning mode defined in the loaded engine configuration.
         *
         * @return The panning mode.
         */
        [[nodiscard]] virtual ePanningMode GetPanningMode() const = 0;

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

#pragma region Singleton Implementation

        /**
         * @brief Returns an unique instance of the Amplitude Engine.
         */
        [[nodiscard]] static Engine* GetInstance();

        /**
         * @brief Destroys the unique instance of the Amplitude Engine.
         */
        static void DestroyInstance();

#pragma endregion
    };

} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_CORE_ENGINE_H
