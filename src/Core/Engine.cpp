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

#include <algorithm>
#include <filesystem>
#include <memory>
#include <queue>
#include <ranges>
#include <unordered_map>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Core/Engine.h>
#include <Core/EngineInternalState.h>
#include <Core/Event.h>
#include <Core/Playback/BusInternalState.h>
#include <Sound/Attenuation.h>
#include <Sound/Collection.h>
#include <Sound/Effect.h>
#include <Sound/Rtpc.h>
#include <Sound/Sound.h>
#include <Sound/Switch.h>
#include <Sound/SwitchContainer.h>

#include "buses_definition_generated.h"
#include "engine_config_definition_generated.h"
#include "sound_bank_definition_generated.h"

#include <Core/DefaultPlugins.h>

#ifndef AM_PLUGINS_UNSUPPORTED
#include <dylib.hpp>

#if AM_PLATFORM_APPLE
#include <limits.h>
extern "C" {
int _NSGetExecutablePath(char* buf, uint32_t* bufSize);
}
#elif AM_PLATFORM_UNIX
#include <limits.h>
#include <unistd.h>
#endif
#endif

#if AM_PLATFORM_WIN
#undef CreateMutex
#endif

namespace SparkyStudios::Audio::Amplitude
{
    typedef flatbuffers::Vector<uint64_t> BusIdList;
    typedef flatbuffers::Vector<flatbuffers::Offset<DuckBusDefinition>> DuckBusDefinitionList;

#ifndef AM_PLUGINS_UNSUPPORTED
    // The list of loaded plugins.
    static std::unordered_map<std::filesystem::path, dylib::library*> gLoadedPlugins = {};

    std::filesystem::path GetExecutableRoot()
    {
        static std::filesystem::path rootPath;
        if (!rootPath.empty())
            return rootPath;

#if AM_PLATFORM_WIN
        char buffer[MAX_PATH];
        GetModuleFileNameA(NULL, buffer, MAX_PATH);
        rootPath = std::filesystem::path(buffer).parent_path();
#elif AM_PLATFORM_APPLE
        char buffer[PATH_MAX];
        uint32_t size = sizeof(buffer);
        if (_NSGetExecutablePath(buffer, &size) == 0)
            rootPath = std::filesystem::canonical(std::filesystem::path(buffer)).parent_path();
#else
        char buffer[PATH_MAX];
        ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
        if (len != -1)
        {
            buffer[len] = '\0';
            rootPath = std::filesystem::path(buffer).parent_path();
        }
#endif

        return rootPath;
    }
#endif

    // Default Plugins instances
    static std::shared_ptr<DefaultResampler> sDefaultResamplerPlugin = nullptr;
    // ---
    static std::shared_ptr<ConstantFader> sConstantFaderPlugin = nullptr;
    static std::shared_ptr<EaseFader> sEaseFaderPlugin = nullptr;
    static std::shared_ptr<EaseInFader> sEaseInFaderPlugin = nullptr;
    static std::shared_ptr<EaseInOutFader> sEaseInOutFaderPlugin = nullptr;
    static std::shared_ptr<EaseOutFader> sEaseOutFaderPlugin = nullptr;
    static std::shared_ptr<ExponentialFader> sExponentialFaderPlugin = nullptr;
    static std::shared_ptr<LinearFader> sLinearFaderPlugin = nullptr;
    static std::shared_ptr<SCurveSmoothFader> sCurveSmoothFaderPlugin = nullptr;
    static std::shared_ptr<SCurveSharpFader> sCurveSharpFaderPlugin = nullptr;
    // ---
    static std::shared_ptr<AMSCodec> sAMSCodecPlugin = nullptr;
    static std::shared_ptr<MP3Codec> sMP3CodecPlugin = nullptr;
    static std::shared_ptr<WAVCodec> sWAVCodecPlugin = nullptr;
    // ---
    static std::shared_ptr<MiniAudioDriver> sMiniAudioDriverPlugin = nullptr;
    static std::shared_ptr<NullDriver> sNullDriverPlugin = nullptr;
    // ---
    static std::shared_ptr<BassBoostFilter> sBassBoostFilterPlugin = nullptr;
    static std::shared_ptr<BiquadResonantFilter> sBiquadResonantFilterPlugin = nullptr;
    static std::shared_ptr<CompressorFilter> sCompressorFilterPlugin = nullptr;
    static std::shared_ptr<DCRemovalFilter> sDCRemovalFilterPlugin = nullptr;
    static std::shared_ptr<DelayFilter> sDelayFilterPlugin = nullptr;
    static std::shared_ptr<EqualizerFilter> sEqualizerFilterPlugin = nullptr;
    static std::shared_ptr<FlangerFilter> sFlangerFilterPlugin = nullptr;
    static std::shared_ptr<LofiFilter> sLofiFilterPlugin = nullptr;
    static std::shared_ptr<MonoPoleFilter> sMonoPoleFilterPlugin = nullptr;
    static std::shared_ptr<PitchShiftFilter> sPitchShiftFilterPlugin = nullptr;
    static std::shared_ptr<RobotizeFilter> sRobotizeFilterPlugin = nullptr;
    static std::shared_ptr<WaveShaperFilter> sWaveShaperFilterPlugin = nullptr;
    // ---
    static std::shared_ptr<AmbisonicBinauralDecoderNode> sAmbisonicBinauralDecoderNodePlugin = nullptr;
    static std::shared_ptr<AmbisonicMixerNode> sAmbisonicMixerNodePlugin = nullptr;
    static std::shared_ptr<AmbisonicPanningNode> sAmbisonicPanningNodePlugin = nullptr;
    static std::shared_ptr<AmbisonicRotatorNode> sAmbisonicRotatorNodePlugin = nullptr;
    static std::shared_ptr<AttenuationNode> sAttenuationNodePlugin = nullptr;
    static std::shared_ptr<ClampNode> sClampNodePlugin = nullptr;
    static std::shared_ptr<HardClipNode> sHardClipNodePlugin = nullptr;
    static std::shared_ptr<EnvironmentEffectNode> sEnvironmentEffectNodePlugin = nullptr;
    static std::shared_ptr<InputNode> sInputNodePlugin = nullptr;
    static std::shared_ptr<LimiterNode> sLimiterNodePlugin = nullptr;
    static std::shared_ptr<NearFieldEffectNode> sNearFieldEffectNodePlugin = nullptr;
    static std::shared_ptr<ObstructionNode> sObstructionNodePlugin = nullptr;
    static std::shared_ptr<OcclusionNode> sOcclusionNodePlugin = nullptr;
    static std::shared_ptr<OutputNode> sOutputNodePlugin = nullptr;
    static std::shared_ptr<ReflectionsNode> sReflectionsNodePlugin = nullptr;
    static std::shared_ptr<ReverbNode> sReverbNodePlugin = nullptr;
    static std::shared_ptr<RoundoffClipNode> sRoundoffClipNodePlugin = nullptr;
    static std::shared_ptr<StereoMixerNode> sStereoMixerNodePlugin = nullptr;
    static std::shared_ptr<StereoPanningNode> sStereoPanningNodePlugin = nullptr;

    static AmUniquePtr<EngineImpl, eMemoryPoolKind_Engine> gAmplitude = nullptr;
    static std::mutex gInstanceMutex;

    std::set<AmOsString> EngineImpl::_pluginSearchPaths = {};

    class LoadSoundBankTask final : public Thread::PoolTask
    {
    public:
        explicit LoadSoundBankTask(SoundBank* soundBank)
            : PoolTask()
            , _soundBank(soundBank)
        {}

        void Work() override
        {
            _soundBank->LoadSoundFiles(amEngine);
        }

        bool Ready() override
        {
            return _soundBank != nullptr;
        }

    private:
        SoundBank* _soundBank = nullptr;
    };

    bool LoadFile(const std::shared_ptr<File>& file, AmString* dest)
    {
        if (!file || !file->IsValid())
        {
            amLogError("The file is not valid.");
            return false;
        }

        const AmSize bytes = file->Length();

        // Get the file's size:
        dest->assign(bytes + 1, 0);

        // Read the file into the buffer
        file->Seek(0, eFileSeekOrigin_Start);
        const AmUInt32 len = file->Read(reinterpret_cast<AmUInt8Buffer>(&(*dest)[0]), bytes);

        return len == bytes && len > 0;
    }

    AmUInt32 GetMaxNumberOfChannels(const EngineConfigDefinition* config)
    {
        return config->mixer()->virtual_channels() + config->mixer()->active_channels();
    }

    // Returns this channel to the appropriate free list based on whether it's
    // backed by a real channel or not.
    void InsertIntoFreeList(const std::shared_ptr<EngineInternalState>& state, ChannelInternalState* channel)
    {
        channel->Remove();
        channel->Reset();
        FreeList* list = channel->IsReal() ? &state->real_channel_free_list : &state->virtual_channel_free_list;
        list->push_front(*channel);
    }

    void DereferenceSound(const std::shared_ptr<EngineInternalState>& state, AmSoundID id)
    {
        if (const auto it = state->sound_map.find(id); it != state->sound_map.end())
        {
            if (it->second->GetRefCounter()->Decrement() == 0)
            {
                for (auto channelIt = state->playing_channel_list.begin(); channelIt != state->playing_channel_list.end();)
                {
                    auto current = channelIt++;
                    if (current->GetSound() == it->second.get())
                    {
                        current->Halt();
                        InsertIntoFreeList(state, &*current);
                    }
                }

                it->second->ReleaseReferences(state);
                state->sound_map.erase(it);
            }
        }
    }

    void DereferenceCollection(const std::shared_ptr<EngineInternalState>& state, AmCollectionID id)
    {
        if (const auto it = state->collection_map.find(id); it != state->collection_map.end())
        {
            if (it->second->GetRefCounter()->Decrement() == 0)
            {
                for (auto channelIt = state->playing_channel_list.begin(); channelIt != state->playing_channel_list.end();)
                {
                    auto current = channelIt++;
                    if (current->GetCollection() == it->second.get())
                    {
                        current->Halt();
                        InsertIntoFreeList(state, &*current);
                    }
                }

                it->second->ReleaseReferences(state);
                state->collection_map.erase(it);
            }
        }
    }

    void DereferenceSwitchContainer(const std::shared_ptr<EngineInternalState>& state, AmSwitchContainerID id)
    {
        if (const auto it = state->switch_container_map.find(id); it != state->switch_container_map.end())
        {
            if (it->second->GetRefCounter()->Decrement() == 0)
            {
                for (auto channelIt = state->playing_channel_list.begin(); channelIt != state->playing_channel_list.end();)
                {
                    auto current = channelIt++;
                    if (current->GetSwitchContainer() == it->second.get())
                    {
                        current->Halt();
                        InsertIntoFreeList(state, &*current);
                    }
                }

                it->second->ReleaseReferences(state);
                state->switch_container_map.erase(it);
            }
        }
    }

    void DereferenceEffect(const std::shared_ptr<EngineInternalState>& state, AmEffectID id)
    {
        if (const auto it = state->effect_map.find(id); it != state->effect_map.end())
        {
            if (it->second->GetRefCounter()->Decrement() == 0)
            {
                it->second->ReleaseReferences(state);
                state->effect_map.erase(it);
            }
        }
    }

    void DereferenceAttenuation(const std::shared_ptr<EngineInternalState>& state, AmAttenuationID id)
    {
        if (const auto it = state->attenuation_map.find(id); it != state->attenuation_map.end())
        {
            if (it->second->GetRefCounter()->Decrement() == 0)
            {
                it->second->ReleaseReferences(state);
                state->attenuation_map.erase(it);
            }
        }
    }

    void DereferenceSwitch(const std::shared_ptr<EngineInternalState>& state, AmSwitchID id)
    {
        if (const auto it = state->switch_map.find(id); it != state->switch_map.end())
        {
            if (it->second->GetRefCounter()->Decrement() == 0)
            {
                it->second->ReleaseReferences(state);
                state->switch_map.erase(it);
            }
        }
    }

    void DereferenceRtpc(const std::shared_ptr<EngineInternalState>& state, AmRtpcID id)
    {
        if (const auto it = state->rtpc_map.find(id); it != state->rtpc_map.end())
        {
            if (it->second->GetRefCounter()->Decrement() == 0)
            {
                it->second->ReleaseReferences(state);
                state->rtpc_map.erase(it);
            }
        }
    }

    void DereferenceEvent(const std::shared_ptr<EngineInternalState>& state, AmEventID id)
    {
        if (const auto it = state->event_map.find(id); it != state->event_map.end())
        {
            if (it->second->GetRefCounter()->Decrement() == 0)
            {
                it->second->ReleaseReferences(state);
                state->event_map.erase(it);
            }
        }
    }

    void AssignBestRoom(ChannelInternalState* newChannel, const AmVector3& location, std::shared_ptr<EngineInternalState> state)
    {
        RoomInternalState* bestRoom = nullptr;
        AmReal32 minDistanceSquared = std::numeric_limits<AmReal32>::max();

        for (auto&& room : state->room_list)
        {
            auto& shape = room.GetShape();

            if (shape.Contains(location))
            {
                bestRoom = &room;
                break;
            }

            if (const AmReal32 distance = shape.GetShortestDistanceToEdge(location); distance < minDistanceSquared)
            {
                minDistanceSquared = distance;
                bestRoom = &room;
            }
        }

        newChannel->SetRoom(Room(bestRoom));
    }

    EngineImpl::EngineImpl()
        : _frameThreadMutex()
        , _configSrc()
        , _state(nullptr)
        , _defaultListener(nullptr)
        , _fs()
        , _audioDriver(nullptr)
    {}

    EngineImpl::~EngineImpl()
    {
        Deinitialize();

        _configSrc.clear();

        _audioDriver = nullptr;

#ifndef AM_PLUGINS_UNSUPPORTED
        using namespace dylib;

        for (const auto& plugin : gLoadedPlugins)
        {
            if (const auto unregisterFunc = plugin.second->get_function<bool()>("UnregisterPlugin"); !unregisterFunc())
                amLogError("An error occurred while unloading the plugin '%s'", plugin.second->get_function<const char*()>("PluginName")());

            ampooldelete(eMemoryPoolKind_Engine, library, plugin.second);
        }

        gLoadedPlugins.clear();
#endif
    }

    AmVoidPtr Engine::LoadPlugin(const AmOsString& pluginLibraryName)
    {
#ifndef AM_PLUGINS_UNSUPPORTED
        if (pluginLibraryName.empty())
        {
            amLogError("The plugin library path is empty");
            return nullptr;
        }

        bool foundPath = false;
        DiskFileSystem fs;
        AmOsString pluginsDirectoryPath;
        AmOsString finalName;

        for (const auto& path : { GetExecutableRoot(), std::filesystem::current_path() })
        {
            pluginsDirectoryPath = path.native();
            finalName = AM_STRING_TO_OS_STRING(dylib::decorations::os_default().prefix) + pluginLibraryName +
                AM_STRING_TO_OS_STRING(dylib::decorations::os_default().suffix);

            fs.SetBasePath(pluginsDirectoryPath);

            // Search for the library in the current directory
            if (const auto realPath = fs.ResolvePath(pluginsDirectoryPath); fs.Exists(fs.Join({ realPath, finalName })))
            {
                pluginsDirectoryPath = realPath;
                foundPath = true;
                break;
            }
        }

        if (!foundPath)
        {
            // Search for the library in the search paths
            for (const auto& path : EngineImpl::_pluginSearchPaths)
            {
                if (const auto realPath = fs.ResolvePath(path); fs.Exists(fs.Join({ realPath, finalName })))
                {
                    pluginsDirectoryPath = realPath;
                    foundPath = true;
                    break;
                }
            }
        }

        if (!foundPath)
        {
            amLogError("The plugin '" AM_OS_CHAR_FMT "' cannot be found in any of the search paths.", pluginLibraryName.c_str());
            return nullptr;
        }

        std::filesystem::path pluginPath = std::filesystem::path(pluginsDirectoryPath) / finalName;

        if (gLoadedPlugins.find(pluginPath) != gLoadedPlugins.end())
        {
            amLogWarning("The plugin '" AM_OS_CHAR_FMT "' is already loaded.", pluginLibraryName.c_str());
            return gLoadedPlugins[pluginPath]->native_handle();
        }

        auto* plugin = ampoolnew(eMemoryPoolKind_Engine, dylib::library, AM_OS_STRING_TO_STRING(pluginPath.native()), dylib::decorations());

        if (!plugin->get_symbol("RegisterPlugin"))
        {
            amLogError(
                "Failed to load plugin '" AM_OS_CHAR_FMT "'. The library doesn't export a RegisterPlugin symbol.",
                pluginLibraryName.c_str());
            return nullptr;
        }

        if (!plugin->get_symbol("UnregisterPlugin"))
        {
            amLogError(
                "Failed to load plugin '" AM_OS_CHAR_FMT "'. The library doesn't export a UnregisterPlugin symbol.",
                pluginLibraryName.c_str());
            return nullptr;
        }

        if (!plugin->get_symbol("PluginName"))
        {
            amLogError(
                "Failed to load plugin '" AM_OS_CHAR_FMT "'. The library doesn't export a PluginName symbol.", pluginLibraryName.c_str());
            return nullptr;
        }

        if (!plugin->get_symbol("PluginVersion"))
        {
            amLogError(
                "Failed to load plugin '" AM_OS_CHAR_FMT "'. The library doesn't export a PluginVersion symbol.",
                pluginLibraryName.c_str());
            return nullptr;
        }

        if (const auto registerFunc = plugin->get_function<bool(Engine*, MemoryManager*)>("RegisterPlugin");
            !registerFunc(amEngine, amMemory))
        {
            amLogError("Failed to load plugin '" AM_OS_CHAR_FMT "'. The plugin registration has failed.", pluginLibraryName.c_str());
            return nullptr;
        }

        {
            const auto GetPluginName = plugin->get_function<const char*()>("PluginName");
            const auto GetPluginVersion = plugin->get_function<const char*()>("PluginVersion");
            amLogInfo("Loaded Plugin '%s' Version: %s", GetPluginName(), GetPluginVersion());
        }

        gLoadedPlugins[pluginPath] = plugin;

        return plugin->native_handle();
#else
        amLogError("The plugin system is not supported on this platform.");
        return nullptr;
#endif
    }

    void Engine::AddPluginSearchPath(const AmOsString& path)
    {
        if (path.empty())
        {
            amLogError("The plugin search path is empty.");
            return;
        }

        if (EngineImpl::_pluginSearchPaths.contains(path))
            return;

        EngineImpl::_pluginSearchPaths.emplace(path);
    }

    void Engine::RemovePluginSearchPath(const AmOsString& path)
    {
        if (path.empty())
        {
            amLogError("The plugin search path is empty.");
            return;
        }

        const auto it = EngineImpl::_pluginSearchPaths.find(path);
        if (it == EngineImpl::_pluginSearchPaths.cend())
            return;

        EngineImpl::_pluginSearchPaths.erase(it);
    }

    bool Engine::RegisterDefaultExtensions()
    {
        if (gAmplitude != nullptr && gAmplitude->_state != nullptr)
            return false; // Cannot register the default plugins when the engine is already initialized.

        // Ensure to clean up registries
        UnregisterDefaultExtensions();

        sDefaultResamplerPlugin = RegisterExtension<DefaultResampler>();
        // ---
        sConstantFaderPlugin = RegisterExtension<ConstantFader>();
        sEaseFaderPlugin = RegisterExtension<EaseFader>();
        sEaseInFaderPlugin = RegisterExtension<EaseInFader>();
        sEaseInOutFaderPlugin = RegisterExtension<EaseInOutFader>();
        sEaseOutFaderPlugin = RegisterExtension<EaseOutFader>();
        sExponentialFaderPlugin = RegisterExtension<ExponentialFader>();
        sLinearFaderPlugin = RegisterExtension<LinearFader>();
        sCurveSmoothFaderPlugin = RegisterExtension<SCurveSmoothFader>();
        sCurveSharpFaderPlugin = RegisterExtension<SCurveSharpFader>();
        // ---
        sAMSCodecPlugin = RegisterExtension<AMSCodec>();
        sMP3CodecPlugin = RegisterExtension<MP3Codec>();
        sWAVCodecPlugin = RegisterExtension<WAVCodec>();
        // ---
        sMiniAudioDriverPlugin = RegisterExtension<MiniAudioDriver>();
        sNullDriverPlugin = RegisterExtension<NullDriver>();
        // ---
        sBassBoostFilterPlugin = RegisterExtension<BassBoostFilter>();
        sBiquadResonantFilterPlugin = RegisterExtension<BiquadResonantFilter>();
        sCompressorFilterPlugin = RegisterExtension<CompressorFilter>();
        sDCRemovalFilterPlugin = RegisterExtension<DCRemovalFilter>();
        sDelayFilterPlugin = RegisterExtension<DelayFilter>();
        sEqualizerFilterPlugin = RegisterExtension<EqualizerFilter>();
        sFlangerFilterPlugin = RegisterExtension<FlangerFilter>();
        sLofiFilterPlugin = RegisterExtension<LofiFilter>();
        sMonoPoleFilterPlugin = RegisterExtension<MonoPoleFilter>();
        sPitchShiftFilterPlugin = RegisterExtension<PitchShiftFilter>();
        sRobotizeFilterPlugin = RegisterExtension<RobotizeFilter>();
        sWaveShaperFilterPlugin = RegisterExtension<WaveShaperFilter>();
        // ---
        sAmbisonicBinauralDecoderNodePlugin = RegisterExtension<AmbisonicBinauralDecoderNode>();
        sAmbisonicMixerNodePlugin = RegisterExtension<AmbisonicMixerNode>();
        sAmbisonicPanningNodePlugin = RegisterExtension<AmbisonicPanningNode>();
        sAmbisonicRotatorNodePlugin = RegisterExtension<AmbisonicRotatorNode>();
        sAttenuationNodePlugin = RegisterExtension<AttenuationNode>();
        sClampNodePlugin = RegisterExtension<ClampNode>();
        sHardClipNodePlugin = RegisterExtension<HardClipNode>();
        sEnvironmentEffectNodePlugin = RegisterExtension<EnvironmentEffectNode>();
        sInputNodePlugin = RegisterExtension<InputNode>();
        sLimiterNodePlugin = RegisterExtension<LimiterNode>();
        sNearFieldEffectNodePlugin = RegisterExtension<NearFieldEffectNode>();
        sObstructionNodePlugin = RegisterExtension<ObstructionNode>();
        sOcclusionNodePlugin = RegisterExtension<OcclusionNode>();
        sOutputNodePlugin = RegisterExtension<OutputNode>();
        sReflectionsNodePlugin = RegisterExtension<ReflectionsNode>();
        sReverbNodePlugin = RegisterExtension<ReverbNode>();
        sRoundoffClipNodePlugin = RegisterExtension<RoundoffClipNode>();
        sStereoMixerNodePlugin = RegisterExtension<StereoMixerNode>();
        sStereoPanningNodePlugin = RegisterExtension<StereoPanningNode>();

        return true;
    }

    bool Engine::UnregisterDefaultExtensions()
    {
        if (gAmplitude != nullptr && gAmplitude->_state != nullptr)
            return false; // Cannot unregister the default plugins when the engine is already initialized.

        UnregisterExtension(sDefaultResamplerPlugin);
        // ---
        UnregisterExtension(sConstantFaderPlugin);
        UnregisterExtension(sEaseFaderPlugin);
        UnregisterExtension(sEaseInFaderPlugin);
        UnregisterExtension(sEaseInOutFaderPlugin);
        UnregisterExtension(sEaseOutFaderPlugin);
        UnregisterExtension(sExponentialFaderPlugin);
        UnregisterExtension(sLinearFaderPlugin);
        UnregisterExtension(sCurveSmoothFaderPlugin);
        UnregisterExtension(sCurveSharpFaderPlugin);
        // ---
        UnregisterExtension(sAMSCodecPlugin);
        UnregisterExtension(sMP3CodecPlugin);
        UnregisterExtension(sWAVCodecPlugin);
        // ---
        UnregisterExtension(sMiniAudioDriverPlugin);
        UnregisterExtension(sNullDriverPlugin);
        // ---
        UnregisterExtension(sBassBoostFilterPlugin);
        UnregisterExtension(sBiquadResonantFilterPlugin);
        UnregisterExtension(sCompressorFilterPlugin);
        UnregisterExtension(sDCRemovalFilterPlugin);
        UnregisterExtension(sDelayFilterPlugin);
        UnregisterExtension(sEqualizerFilterPlugin);
        UnregisterExtension(sFlangerFilterPlugin);
        UnregisterExtension(sLofiFilterPlugin);
        UnregisterExtension(sMonoPoleFilterPlugin);
        UnregisterExtension(sPitchShiftFilterPlugin);
        UnregisterExtension(sRobotizeFilterPlugin);
        UnregisterExtension(sWaveShaperFilterPlugin);
        // ---
        UnregisterExtension(sAmbisonicBinauralDecoderNodePlugin);
        UnregisterExtension(sAmbisonicMixerNodePlugin);
        UnregisterExtension(sAmbisonicPanningNodePlugin);
        UnregisterExtension(sAmbisonicRotatorNodePlugin);
        UnregisterExtension(sAttenuationNodePlugin);
        UnregisterExtension(sClampNodePlugin);
        UnregisterExtension(sHardClipNodePlugin);
        UnregisterExtension(sEnvironmentEffectNodePlugin);
        UnregisterExtension(sInputNodePlugin);
        UnregisterExtension(sLimiterNodePlugin);
        UnregisterExtension(sNearFieldEffectNodePlugin);
        UnregisterExtension(sObstructionNodePlugin);
        UnregisterExtension(sOcclusionNodePlugin);
        UnregisterExtension(sOutputNodePlugin);
        UnregisterExtension(sReflectionsNodePlugin);
        UnregisterExtension(sReverbNodePlugin);
        UnregisterExtension(sRoundoffClipNodePlugin);
        UnregisterExtension(sStereoMixerNodePlugin);
        UnregisterExtension(sStereoPanningNodePlugin);

        return true;
    }

    Engine* Engine::GetInstance()
    {
        // Cannot implement the singleton pattern with an uninitialized memory manager.
        if (!MemoryManager::IsInitialized())
            return nullptr;

        std::lock_guard lock(gInstanceMutex);

        // Amplitude Engine unique instance.
        if (gAmplitude == nullptr)
            gAmplitude.reset(ampoolnew(eMemoryPoolKind_Engine, EngineImpl));

        return gAmplitude.get();
    }

    void Engine::DestroyInstance()
    {
        std::lock_guard lock(gInstanceMutex);
        gAmplitude.reset();
    }

    std::shared_ptr<BusInternalState> FindBusInternalState(const std::shared_ptr<EngineInternalState>& state, AmBusID id)
    {
        if (const auto it = std::ranges::find_if(
                state->buses,
                [&id](const std::shared_ptr<BusInternalState>& bus)
                {
                    return bus->GetId() == id;
                });
            it != state->buses.end())
        {
            return *it;
        }

        return nullptr;
    }

    std::shared_ptr<BusInternalState> FindBusInternalState(const std::shared_ptr<EngineInternalState>& state, const AmString& name)
    {
        if (const auto it = std::ranges::find_if(
                state->buses,
                [&name](const std::shared_ptr<BusInternalState>& bus)
                {
                    return bus->GetName() == name;
                });
            it != state->buses.end())
        {
            return *it;
        }

        return nullptr;
    }

    static bool PopulateChildBuses(
        const std::shared_ptr<EngineInternalState>& state, const std::shared_ptr<BusInternalState>& parent, const BusIdList* childIdList)
    {
        std::vector<std::shared_ptr<BusInternalState>>* output = &parent->GetChildBuses();

        for (flatbuffers::uoffset_t i = 0; childIdList && i < childIdList->size(); ++i)
        {
            const AmBusID busId = childIdList->Get(i);

            if (auto bus = FindBusInternalState(state, busId))
            {
                output->push_back(bus);
            }
            else
            {
                amLogError("Unknown bus with ID '" AM_ID_CHAR_FMT "' listed in child buses.", busId);
                return false;
            }
        }

        return true;
    }

    static bool PopulateDuckBuses(
        const std::shared_ptr<EngineInternalState>& state,
        const std::shared_ptr<BusInternalState>& parent,
        const DuckBusDefinitionList* duckBusDefinitionList)
    {
        DuckBusList* output = &parent->GetDuckBuses();

        for (flatbuffers::uoffset_t i = 0; duckBusDefinitionList && i < duckBusDefinitionList->size(); ++i)
        {
            const DuckBusDefinition* duck = duckBusDefinitionList->Get(i);

            if (auto bus = ampoolunique(eMemoryPoolKind_Engine, DuckBusInternalState, parent.get()); bus->Initialize(duck))
            {
                output->push_back(std::move(bus));
            }
            else
            {
                bus.reset(nullptr);
                amLogError("Unknown bus with ID '" AM_ID_CHAR_FMT "' listed in duck buses.", duck->id());
                return false;
            }
        }

        return true;
    }

    // The InternalChannelStates have three lists they are a part of: The engine's
    // priority list, the bus's playing sound list, and which free list they are in.
    // Initially, all nodes are in a free list because nothing is playing. Separate
    // free lists are kept for real channels and virtual channels (where 'real'
    // channels are channels that have a channel_id
    static void InitializeChannelFreeLists(
        FreeList* realChannelFreeList,
        FreeList* virtualChannelFreeList,
        std::vector<ChannelInternalState>* channels,
        const AmUInt32 virtualChannels,
        const AmUInt32 realChannels)
    {
        // We do our own tracking of audio channels so that when a new sound is
        // played we can determine if one of the currently playing channels is lower
        // priority so that we can drop it.
        const AmUInt32 totalChannels = realChannels + virtualChannels;
        channels->resize(totalChannels);

        for (AmSize i = 0; i < totalChannels; ++i)
        {
            ChannelInternalState& channel = (*channels)[i];

            // Track real channels separately from virtual channels.
            if (i < realChannels)
            {
                channel.GetRealChannel().Initialize(static_cast<int>(i + 1));
                realChannelFreeList->push_front(channel);
            }
            else
            {
                channel.GetRealChannel().Initialize(kAmInvalidObjectId);
                virtualChannelFreeList->push_front(channel);
            }
        }
    }

    static void InitializeListenerFreeList(
        std::vector<ListenerInternalState*>* listenerStateFreeList, ListenerStateVector* listenerList, const AmUInt32 listSize)
    {
        listenerList->resize(listSize);
        listenerStateFreeList->reserve(listSize);
        for (AmSize i = 0; i < listSize; ++i)
        {
            ListenerInternalState& listener = (*listenerList)[i];
            listenerStateFreeList->push_back(&listener);
        }
    }

    static void InitializeEntityFreeList(
        std::vector<EntityInternalState*>* entityStateFreeList, EntityStateVector* entityList, const AmUInt32 listSize)
    {
        entityList->resize(listSize);
        entityStateFreeList->reserve(listSize);
        for (AmSize i = 0; i < listSize; ++i)
        {
            EntityInternalState& entity = (*entityList)[i];
            entityStateFreeList->push_back(&entity);
        }
    }

    static void InitializeEnvironmentFreeList(
        std::vector<EnvironmentInternalState*>* environmentStateFreeList, EnvironmentStateVector* environmentList, const AmUInt32 listSize)
    {
        environmentList->resize(listSize);
        environmentStateFreeList->reserve(listSize);
        for (AmSize i = 0; i < listSize; ++i)
        {
            EnvironmentInternalState& environment = (*environmentList)[i];
            environmentStateFreeList->push_back(&environment);
        }
    }

    static void InitializeRoomFreeList(
        std::vector<RoomInternalState*>* roomStateFreeList, RoomStateVector* roomList, const AmUInt32 listSize)
    {
        roomList->resize(listSize);
        roomStateFreeList->reserve(listSize);
        for (AmSize i = 0; i < listSize; ++i)
        {
            RoomInternalState& room = (*roomList)[i];
            roomStateFreeList->push_back(&room);
        }
    }

    bool EngineImpl::Initialize(const AmOsString& configFile)
    {
        std::lock_guard lock(_updateMutex);

        _configFilePath = configFile;

        if (!LoadFile(_fs->OpenFile(_configFilePath), &_configSrc))
        {
            amLogError("Could not load audio config file at path '" AM_OS_CHAR_FMT "'.", _configFilePath.c_str());
            return false;
        }

        return Initialize(GetEngineConfigDefinition());
    }

    bool EngineImpl::Initialize(const EngineConfigDefinition* config)
    {
        if (IsInitialized())
            return true;

        // Lock plugins registries
        Driver::LockRegistry();
        Codec::LockRegistry();
        Resampler::LockRegistry();
        Filter::LockRegistry();
        Fader::LockRegistry();
        Node::LockRegistry();

        // Create the internal engine state
        _state = ampoolshared(eMemoryPoolKind_Engine, EngineInternalState);
        _state->version = &Amplitude::GetVersion();

        // Load the audio driver
        if (config->driver())
        {
            if (_audioDriver = Driver::Find(config->driver()->str()); _audioDriver == nullptr)
            {
                amLogWarning("Could not load the audio driver '%s'. Loading the default driver.", config->driver()->c_str());
                _audioDriver = Driver::Default();
            }
        }
        else
        {
            _audioDriver = Driver::Default();
        }

        if (_audioDriver == nullptr)
        {
            amLogError("Could not load the audio driver. Loading the null driver as fallback.");
            _audioDriver = sNullDriverPlugin;
        }

        if (_audioDriver == nullptr)
        {
            amLogCritical(
                "Failed to load the specified driver, the default driver, and the null driver. Please check your engine "
                "configuration, and ensure that all the needed plugins are loaded.");
            Deinitialize();
            return false;
        }

        // Load the pipeline from the specified file
        if (const AmOsString& pipelineFilePath =
                _fs->Join({ AM_OS_STRING("pipelines"), AM_STRING_TO_OS_STRING(config->mixer()->pipeline()->c_str()) });
            !_state->pipeline.LoadDefinitionFromPath(pipelineFilePath, _state))
        {
            amLogCritical("Could not load the pipeline asset.");
            Deinitialize();
            return false;
        }

        // Store the panning mode
        _state->panning_mode = static_cast<ePanningMode>(config->mixer()->panning_mode());

        // Store the resampler name
        if (config->mixer()->resampler() != nullptr && !config->mixer()->resampler()->empty())
            _state->resampler = config->mixer()->resampler()->str();
        else
            _state->resampler = "default";

        if (const auto* hrtfConfig = config->hrtf(); hrtfConfig != nullptr)
        {
            // Store the HRIR sampling mode
            _state->hrir_sampling_mode = static_cast<eHRIRSphereSamplingMode>(config->hrtf()->hrir_sampling());

            // Load the HRIR sphere
            _state->hrir_sphere = ampoolshared(eMemoryPoolKind_Engine, HRIRSphereImpl);
            _state->hrir_sphere->SetResource(AM_STRING_TO_OS_STRING(config->hrtf()->amir_file()->c_str()));
            _state->hrir_sphere->SetSamplingMode(_state->hrir_sampling_mode);
            _state->hrir_sphere->Load(GetFileSystem());
        }
        else if (_state->panning_mode != ePanningMode_Stereo)
        {
            amLogCritical(
                "The HRTF configuration is missing, but the panning mode is not stereo. Please provide an HRTF configuration, or "
                "set the panning mode to Stereo.");
            Deinitialize();
            return false;
        }

        // Initialize audio mixer
        if (!_state->mixer.Init(config))
        {
            amLogCritical("Could not initialize the audio mixer.");
            Deinitialize();
            return false;
        }

        // Initialize the channel internal data.
        InitializeChannelFreeLists(
            &_state->real_channel_free_list, &_state->virtual_channel_free_list, &_state->channel_state_memory,
            config->mixer()->virtual_channels(), config->mixer()->active_channels());

        // Initialize the listener internal data.
        InitializeListenerFreeList(&_state->listener_state_free_list, &_state->listener_state_memory, config->game()->listeners());

        // Initialize the entity internal data.
        InitializeEntityFreeList(&_state->entity_state_free_list, &_state->entity_state_memory, config->game()->entities());

        // Initialize the environment internal data.
        InitializeEnvironmentFreeList(
            &_state->environment_state_free_list, &_state->environment_state_memory, config->game()->environments());

        // Initialize the room internal data.
        InitializeRoomFreeList(&_state->room_state_free_list, &_state->room_state_memory, config->game()->rooms());

        // Load the audio buses.
        if (const AmOsString& busesFilePath = AM_STRING_TO_OS_STRING(config->buses_file()->c_str());
            !LoadFile(_fs->OpenFile(busesFilePath), &_state->buses_source))
        {
            amLogCritical("Could not load audio bus file.");
            Deinitialize();
            return false;
        }
        const BusDefinitionList* busDefList = Amplitude::GetBusDefinitionList(_state->buses_source.c_str());
        const auto busCount = busDefList->buses()->size();
        _state->buses.resize(busCount);
        for (flatbuffers::uoffset_t i = 0; i < busCount; ++i)
        {
            _state->buses[i] = ampoolshared(eMemoryPoolKind_Engine, BusInternalState);
            _state->buses[i]->Initialize(busDefList->buses()->Get(i));
        }

        // Set up the children and ducking pointers.
        for (auto& bus : _state->buses)
        {
            const BusDefinition* def = bus->GetBusDefinition();
            if (!PopulateChildBuses(_state, bus, def->child_buses()))
            {
                Deinitialize();
                return false;
            }
            if (!PopulateDuckBuses(_state, bus, def->duck_buses()))
            {
                Deinitialize();
                return false;
            }
        }

        // Fetch the master bus by ID
        _state->master_bus = FindBusInternalState(_state, kAmMasterBusId);
        if (!_state->master_bus)
        {
            // Fetch the master bus by name
            _state->master_bus = FindBusInternalState(_state, "master");
            if (!_state->master_bus)
            {
                amLogCritical("Unable to find a master bus.");
                Deinitialize();
                return false;
            }
        }

        // Set the listener fetch mode
        _state->listener_fetch_mode = config->game()->listener_fetch_mode();

        // Doppler effect settings
        _state->sound_speed = config->game()->sound_speed();
        _state->doppler_factor = config->game()->doppler_factor();

        // Samples per streams
        _state->samples_per_stream = config->output()->buffer_size() / 2;

        // Save obstruction/occlusion configurations
        _state->obstruction_config.Init(config->game()->obstruction());
        _state->occlusion_config.Init(config->game()->occlusion());

        // Environment Amounts
        _state->track_environments = config->game()->track_environments();

        // Engine state
        _state->mute = false;
        _state->master_gain = 1.0f;

        // Open the audio device through the driver
        if (!_audioDriver->Open(_state->mixer.GetDeviceDescription()))
        {
            amLogError(
                "Could not open the audio device using the '%s' driver. Loading the null driver as fallback.",
                _audioDriver->GetName().c_str());
            _audioDriver = sNullDriverPlugin;

            if (_audioDriver == nullptr || !_audioDriver->Open(_state->mixer.GetDeviceDescription()))
            {
                amLogCritical("Could not open the audio device.");
                Deinitialize();
                return false;
            }
        }

        // Mark the engine as ready
        _state->paused = false;
        _state->initialized = true;

        amLogDebug("Amplitude Engine initialized successfully.");
        return true;
    }

    bool EngineImpl::Deinitialize()
    {
        std::lock_guard<std::recursive_mutex> lock(_updateMutex);

        if (_state == nullptr)
            return true;

        _state->stopping = true;

        _state->mixer.Wait();

        // Stop all sounds
        StopAll();

        // Process one last frame to update stopped channel states
        AdvanceFrame(0);

        // Pause the engine so no further updates are made
        _state->paused = true;

        // Release channels
        EraseFinishedSounds(_state);

        // Close the audio device through the driver
        if (_audioDriver != nullptr)
            _audioDriver->Close();

        if (_state->mixer.IsInitialized())
            _state->mixer.Deinit();

        // Unload sound banks
        while (HasLoadedSoundBanks())
            UnloadSoundBanks();

        // Release HRIR sphere
        if (_state->hrir_sphere != nullptr)
            _state->hrir_sphere.reset();

        // Release buses
        _state->master_bus.reset();
        _state->buses.clear();

        _state.reset();
        _audioDriver.reset();

        // Unlock registries
        Driver::UnlockRegistry();
        Codec::UnlockRegistry();
        Resampler::UnlockRegistry();
        Filter::UnlockRegistry();
        Fader::UnlockRegistry();
        Node::UnlockRegistry();

        return true;
    }

    bool EngineImpl::IsInitialized() const
    {
        // An initialized engine have a running state
        return _state != nullptr && _state->initialized;
    }

    void EngineImpl::SetFileSystem(std::shared_ptr<FileSystem> fs)
    {
        _fs = fs;
    }

    std::shared_ptr<const FileSystem> EngineImpl::GetFileSystem() const
    {
        return _fs;
    }

    bool EngineImpl::LoadSoundBank(const AmOsString& filename)
    {
        AmBankID outID = kAmInvalidObjectId;
        return LoadSoundBank(filename, outID);
    }

    bool EngineImpl::LoadSoundBank(const AmOsString& filename, AmBankID& outID)
    {
        outID = kAmInvalidObjectId;
        bool success = true;

        if (const auto findIt = _state->sound_bank_id_map.find(filename); findIt == _state->sound_bank_id_map.end() ||
            (findIt != _state->sound_bank_id_map.end() && !_state->sound_bank_map.contains(findIt->second)))
        {
            auto soundBank = ampoolunique(eMemoryPoolKind_Engine, SoundBank);
            success = soundBank->Initialize(filename, this);

            if (success)
            {
                soundBank->GetRefCounter()->Increment();

                const AmBankID id = soundBank->GetId();
                _state->sound_bank_id_map[filename] = id;
                _state->sound_bank_map[id] = std::move(soundBank);
                outID = id;
            }
            else if (findIt != _state->sound_bank_id_map.end())
            {
                _state->sound_bank_id_map.erase(findIt);
            }
        }
        else
        {
            _state->sound_bank_map[findIt->second]->GetRefCounter()->Increment();
        }

        return success;
    }

    bool EngineImpl::LoadSoundBankFromMemoryView(AmConstVoidPtr ptr, AmSize size)
    {
        AmBankID outID = kAmInvalidObjectId;
        return LoadSoundBankFromMemoryView(ptr, size, outID);
    }

    bool EngineImpl::LoadSoundBankFromMemoryView(AmConstVoidPtr ptr, AmSize size, AmBankID& outID)
    {
        outID = kAmInvalidObjectId;
        bool success = true;

        const auto* soundBankDefinition = Amplitude::GetSoundBankDefinition(ptr);
        const AmOsString filename = AM_STRING_TO_OS_STRING(soundBankDefinition->name()->str());

        if (const auto findIt = _state->sound_bank_id_map.find(filename); findIt == _state->sound_bank_id_map.end() ||
            (findIt != _state->sound_bank_id_map.end() && !_state->sound_bank_map.contains(findIt->second)))
        {
            auto soundBank = ampoolunique(eMemoryPoolKind_Engine, SoundBank);
            success = soundBank->InitializeFromMemoryView(ptr, size, this);

            if (success)
            {
                soundBank->GetRefCounter()->Increment();

                const AmBankID id = soundBank->GetId();
                _state->sound_bank_id_map[filename] = id;
                _state->sound_bank_map[id] = std::move(soundBank);
                outID = id;
            }
            else
            {
                _state->sound_bank_id_map.erase(findIt);
            }
        }
        else
        {
            _state->sound_bank_map[findIt->second]->GetRefCounter()->Increment();
        }

        return success;
    }

    void EngineImpl::UnloadSoundBank(const AmOsString& filename)
    {
        if (const auto findIt = _state->sound_bank_id_map.find(filename); findIt == _state->sound_bank_id_map.end())
        {
            amLogWarning("Cannot unload Sound Bank '" AM_OS_CHAR_FMT "'. Sound Bank not loaded.", filename.c_str());
            AMPLITUDE_ASSERT(false);
        }
        else
        {
            UnloadSoundBank(findIt->second);
        }
    }

    void EngineImpl::UnloadSoundBank(AmBankID id)
    {
        if (const auto findIt = _state->sound_bank_map.find(id); findIt == _state->sound_bank_map.end())
        {
            amLogWarning("Cannot unload Sound Bank with ID '" AM_ID_CHAR_FMT "'. Sound Bank not loaded.", id);
            AMPLITUDE_ASSERT(false);
        }
        else if (findIt->second->GetRefCounter()->Decrement() == 0)
        {
            std::lock_guard lock(_frameThreadMutex);

            findIt->second->Deinitialize(this);
            _state->sound_bank_map.erase(id);
        }
    }

    void EngineImpl::UnloadSoundBanks()
    {
        std::lock_guard lock(_frameThreadMutex);

        std::vector<AmBankID> idsToDelete;
        idsToDelete.reserve(_state->sound_bank_map.size());

        for (const auto& item : _state->sound_bank_map | std::ranges::views::values)
        {
            if (RefCounter* ref = item->GetRefCounter(); ref->GetCount() > 0 && ref->Decrement() == 0)
            {
                item->Deinitialize(this);
                idsToDelete.push_back(item->GetId());
            }
        }

        for (const auto id : idsToDelete)
            _state->sound_bank_map.erase(id);
    }

    void EngineImpl::EnsureSoundBankLoaded(const AmOsString& filename)
    {
        if (HasLoadedSoundBank(filename))
            return;

        LoadSoundBank(filename);
    }

    bool EngineImpl::HasLoadedSoundBank(const AmOsString& filename) const
    {
        if (_state == nullptr)
            return false;

        if (const auto findIt = _state->sound_bank_id_map.find(filename); findIt != _state->sound_bank_id_map.end())
            return _state->sound_bank_map.contains(findIt->second);

        return false;
    }

    bool EngineImpl::HasLoadedSoundBank(AmBankID id) const
    {
        if (_state == nullptr)
            return false;

        return _state->sound_bank_map.contains(id);
    }

    bool EngineImpl::HasLoadedSoundBanks() const
    {
        if (_state == nullptr)
            return false;

        return !_state->sound_bank_map.empty();
    }

    void EngineImpl::StartOpenFileSystem()
    {
        _fs->StartOpenFileSystem();
    }

    bool EngineImpl::TryFinalizeOpenFileSystem()
    {
        return _fs->TryFinalizeOpenFileSystem();
    }

    void EngineImpl::StartCloseFileSystem()
    {
        _fs->StartCloseFileSystem();
    }

    bool EngineImpl::TryFinalizeCloseFileSystem()
    {
        return _fs->TryFinalizeCloseFileSystem();
    }

    void EngineImpl::StartLoadSoundFiles()
    {
        if (_soundLoaderThreadPool == nullptr)
            _soundLoaderThreadPool.reset(ampoolnew(eMemoryPoolKind_Engine, Thread::Pool));

        _soundLoaderThreadPool->Init(8);

        for (const auto& bank : _state->sound_bank_map | std::views::values)
        {
            auto task = ampoolshared(eMemoryPoolKind_Engine, LoadSoundBankTask, bank.get());
            _soundLoaderThreadPool->AddTask(task);
        }
    }

    bool EngineImpl::TryFinalizeLoadSoundFiles()
    {
        if (_soundLoaderThreadPool == nullptr)
            return true;

        if (_soundLoaderThreadPool->HasTasks())
            return false;

        _soundLoaderThreadPool.reset(nullptr);
        return true;
    }

    /**
     * @brief Cache key for FindBestListener lookups.
     *
     * Uses spatial quantization (10cm grid) to increase cache hit rates
     * for nearby locations.
     */
    struct ListenerCacheKey
    {
        AmInt32 gridX, gridY, gridZ;
        eListenerFetchMode fetchMode;

        ListenerCacheKey(const AmVector3& location, eListenerFetchMode mode)
            : fetchMode(mode)
        {
            constexpr AmReal32 kGridSize = 0.1f; // 10cm precision
            gridX = static_cast<AmInt32>(std::floor(location.x / kGridSize));
            gridY = static_cast<AmInt32>(std::floor(location.y / kGridSize));
            gridZ = static_cast<AmInt32>(std::floor(location.z / kGridSize));
        }

        bool operator==(const ListenerCacheKey& other) const
        {
            return gridX == other.gridX && gridY == other.gridY && gridZ == other.gridZ && fetchMode == other.fetchMode;
        }

        struct Hash
        {
            std::size_t operator()(const ListenerCacheKey& key) const
            {
                std::size_t h1 = std::hash<AmInt32>{}(key.gridX);
                std::size_t h2 = std::hash<AmInt32>{}(key.gridY);
                std::size_t h3 = std::hash<AmInt32>{}(key.gridZ);
                std::size_t h4 = std::hash<int>{}(static_cast<int>(key.fetchMode));

                std::size_t seed = 0;

                // Combine using golden ratio constant (0x9e3779b9)
                seed ^= h1 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
                seed ^= h2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
                seed ^= h3 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
                seed ^= h4 + 0x9e3779b9 + (seed << 6) + (seed >> 2);

                return seed;
            }
        };
    };

    ListenerInternalState* FindBestListener(
        ListenerList& listeners, const AmVector3& location, eListenerFetchMode fetchMode, ListenerCache* cache = nullptr)
    {
        if (listeners.empty())
            return nullptr;

        // Fast paths for non-spatial modes
        if (fetchMode == eListenerFetchMode_None)
            return nullptr;

        if (fetchMode == eListenerFetchMode_First)
            return listeners.empty() ? nullptr : &*listeners.begin();

        if (fetchMode == eListenerFetchMode_Last)
            return listeners.empty() ? nullptr : &*--listeners.end();

        ListenerCacheKey key(location, fetchMode);
        AmSize hashValue = ListenerCacheKey::Hash{}(key);

        // For spatial modes (Nearest/Farthest/Default), check cache if available
        if (cache != nullptr)
        {
            auto it = cache->cache.find(hashValue);
            if (it != cache->cache.end())
                return static_cast<ListenerInternalState*>(it->second);
        }

        // Cache miss or cache disabled
        ListenerInternalState* result = nullptr;

        ListenerList::iterator bestListener;
        const AmVector4 location4 = { location.x, location.y, location.z, 1.0f };

        switch (fetchMode)
        {
        case eListenerFetchMode_Nearest:
            [[fallthrough]];
        case eListenerFetchMode_Farthest:
            {
                auto listener = listeners.begin();
                auto listenerSpaceLocation = Transform(listener->GetInverseMatrix(), location4).xyz;
                AmReal32 distanceSquared = SquaredLength(listenerSpaceLocation);
                bestListener = listener;

                for (++listener; listener != listeners.end(); ++listener)
                {
                    const AmVector3 transformedLocation = Transform(listener->GetInverseMatrix(), location4).xyz;
                    if (const AmReal32 magnitudeSquared = SquaredLength(transformedLocation);
                        fetchMode == eListenerFetchMode_Nearest ? magnitudeSquared < distanceSquared : magnitudeSquared > distanceSquared)
                    {
                        bestListener = listener;
                        distanceSquared = magnitudeSquared;
                    }
                }
                result = &*bestListener;
            }
            break;

        case eListenerFetchMode_Default:
            {
                ListenerInternalState* state = amEngine->GetDefaultListener().GetState();
                if (state == nullptr)
                    return nullptr;

                for (auto& listener : listeners)
                    if (listener.GetId() == state->GetId())
                    {
                        result = &listener;
                        break;
                    }
            }
            break;

        default:
            break;
        }

        if (cache != nullptr)
            cache->cache[hashValue] = result;

        return result;
    }

    static void CalculateGainAndPitch(
        AmReal32* gain,
        AmReal32* pitch,
        const ListenerInternalState* listener,
        const ChannelInternalState* channel,
        const AmReal32 soundGain,
        const AmReal32 soundPitch,
        const BusInternalState* bus,
        const eSpatialization spatialization,
        const AmReal32 userGain)
    {
        *gain = soundGain * bus->GetGain() * userGain;
        *pitch = soundPitch;

        if (spatialization != eSpatialization_None && listener != nullptr && channel != nullptr)
            *pitch *= channel->GetDopplerFactor(listener->GetId());
    }

    // Given the priority of a node, and the list of ChannelInternalStates sorted descending
    // by priority, find the location in the list where the node should be inserted before.
    // Incumbents with equal priority are preserved ahead of the new node. If the node
    // is highest priority, this returns begin(). If it is lower than or equal to the lowest
    // active sound, this returns end().
    PriorityList::iterator FindInsertionPoint(PriorityList* list, const AmReal32 priority)
    {
        PriorityList::reverse_iterator it;

        for (it = list->rbegin(); it != list->rend(); ++it)
        {
            if (const AmReal32 p = it->Priority(); p >= priority)
                break;
        }

        return it.base();
    }

    // Given a location to insert a node, take a ChannelInternalState from the
    // appropriate list and insert it there. Return the new ChannelInternalState.
    //
    // There are three places a ChannelInternalState may be taken from. First, if
    // there are any real channels available in the real channel free list, use one
    // of those so that your channel can play.
    //
    // If there are no real channels, then use a free virtual channel instead so
    // that your channel can at least be tracked.
    //
    // If there are no real or virtual channels, evict the lowest priority playing sound
    // if the new sound has strictly higher priority.
    //
    // If the node you are trying to insert has lower or equal priority compared to the lowest
    // playing sound, do nothing and return a nullptr.
    ChannelInternalState* FindFreeChannelInternalState(
        PriorityList::iterator insertionPoint,
        PriorityList* list,
        FreeList* realChannelFreeList,
        FreeList* virtualChannelFreeList,
        const bool paused)
    {
        ChannelInternalState* newChannel = nullptr;
        // Grab a free ChannelInternalState if there is one and the engine is not
        // paused. The engine is paused, grab a virtual channel for now, and it will
        // fix itself when the engine is not paused.
        if (!paused && !realChannelFreeList->empty())
        {
            newChannel = &realChannelFreeList->front();
            realChannelFreeList->pop_front();
            PriorityList::insert_before(*insertionPoint, *newChannel, &ChannelInternalState::priority_node);
        }
        else if (!virtualChannelFreeList->empty())
        {
            newChannel = &virtualChannelFreeList->front();
            virtualChannelFreeList->pop_front();
            PriorityList::insert_before(*insertionPoint, *newChannel, &ChannelInternalState::priority_node);
        }
        else if (insertionPoint != list->end())
        {
            // If there are no free channels, and the new sound is strictly higher priority
            // than the lowest priority sound, evict the lowest priority sound.
            newChannel = &list->back();
            newChannel->Halt();

            // Move it to a new spot in the list if it needs to be moved.
            if (&*insertionPoint != newChannel)
            {
                list->pop_back();
                list->insert(insertionPoint, *newChannel);
            }
        }

        return newChannel;
    }

    Channel EngineImpl::Play(SwitchContainerHandle handle) const
    {
        return Play(handle, kVector3Zero, 1.0f);
    }

    Channel EngineImpl::Play(SwitchContainerHandle handle, const AmVector3& location) const
    {
        return Play(handle, location, 1.0f);
    }

    Channel EngineImpl::Play(SwitchContainerHandle handle, const AmVector3& location, const AmReal32 userGain) const
    {
        return PlayScopedSwitchContainer(handle, Entity(nullptr), location, userGain);
    }

    Channel EngineImpl::Play(SwitchContainerHandle handle, const Entity& entity) const
    {
        return Play(handle, entity, 1.0f);
    }

    Channel EngineImpl::Play(SwitchContainerHandle handle, const Entity& entity, const AmReal32 userGain) const
    {
        return PlayScopedSwitchContainer(handle, entity, entity.GetLocation(), userGain);
    }

    Channel EngineImpl::Play(CollectionHandle handle) const
    {
        return Play(handle, kVector3Zero, 1.0f);
    }

    Channel EngineImpl::Play(CollectionHandle handle, const AmVector3& location) const
    {
        return Play(handle, location, 1.0f);
    }

    Channel EngineImpl::Play(CollectionHandle handle, const AmVector3& location, const AmReal32 userGain) const
    {
        return PlayScopedCollection(handle, Entity(nullptr), location, userGain);
    }

    Channel EngineImpl::Play(CollectionHandle handle, const Entity& entity) const
    {
        return Play(handle, entity, 1.0f);
    }

    Channel EngineImpl::Play(CollectionHandle handle, const Entity& entity, const AmReal32 userGain) const
    {
        return PlayScopedCollection(handle, entity, entity.GetLocation(), userGain);
    }

    Channel EngineImpl::Play(SoundHandle handle) const
    {
        return Play(handle, kVector3Zero, 1.0f);
    }

    Channel EngineImpl::Play(SoundHandle handle, const AmVector3& location) const
    {
        return Play(handle, location, 1.0f);
    }

    Channel EngineImpl::Play(SoundHandle handle, const AmVector3& location, AmReal32 userGain) const
    {
        return PlayScopedSound(handle, Entity(nullptr), location, userGain);
    }

    Channel EngineImpl::Play(SoundHandle handle, const Entity& entity) const
    {
        return Play(handle, entity, 1.0f);
    }

    Channel EngineImpl::Play(SoundHandle handle, const Entity& entity, AmReal32 userGain) const
    {
        return PlayScopedSound(handle, entity, entity.GetLocation(), userGain);
    }

    Channel EngineImpl::Play(const AmString& name) const
    {
        return Play(name, kVector3Zero, 1.0f);
    }

    Channel EngineImpl::Play(const AmString& name, const AmVector3& location) const
    {
        return Play(name, location, 1.0f);
    }

    Channel EngineImpl::Play(const AmString& name, const AmVector3& location, const AmReal32 userGain) const
    {
        if (SoundHandle handle = GetSoundHandle(name))
            return Play(handle, location, userGain);

        if (CollectionHandle handle = GetCollectionHandle(name))
            return Play(handle, location, userGain);

        if (SwitchContainerHandle handle = GetSwitchContainerHandle(name))
            return Play(handle, location, userGain);

        amLogError("Cannot play object: invalid name (%s).", name.c_str());
        return Channel(nullptr);
    }

    Channel EngineImpl::Play(const AmString& name, const Entity& entity) const
    {
        return Play(name, entity, 1.0f);
    }

    Channel EngineImpl::Play(const AmString& name, const Entity& entity, const AmReal32 userGain) const
    {
        if (Sound* handle = GetSoundHandle(name))
            return Play(handle, entity, userGain);

        if (Collection* handle = GetCollectionHandle(name))
            return Play(handle, entity, userGain);

        if (SwitchContainer* handle = GetSwitchContainerHandle(name))
            return Play(handle, entity, userGain);

        amLogError("Cannot play sound: invalid name (%s).", name.c_str());
        return Channel(nullptr);
    }

    Channel EngineImpl::Play(AmObjectID id) const
    {
        return Play(id, kVector3Zero, 1.0f);
    }

    Channel EngineImpl::Play(AmObjectID id, const AmVector3& location) const
    {
        return Play(id, location, 1.0f);
    }

    Channel EngineImpl::Play(AmObjectID id, const AmVector3& location, const AmReal32 userGain) const
    {
        if (Sound* handle = GetSoundHandle(id))
            return Play(handle, location, userGain);

        if (Collection* handle = GetCollectionHandle(id))
            return Play(handle, location, userGain);

        if (SwitchContainer* handle = GetSwitchContainerHandle(id))
            return Play(handle, location, userGain);

        amLogError("Cannot play sound: invalid ID (" AM_ID_CHAR_FMT ").", id);
        return Channel(nullptr);
    }

    Channel EngineImpl::Play(AmObjectID id, const Entity& entity) const
    {
        return Play(id, entity, 1.0f);
    }

    Channel EngineImpl::Play(AmObjectID id, const Entity& entity, const AmReal32 userGain) const
    {
        if (Sound* handle = GetSoundHandle(id))
            return Play(handle, entity, userGain);

        if (Collection* handle = GetCollectionHandle(id))
            return Play(handle, entity, userGain);

        if (SwitchContainer* handle = GetSwitchContainerHandle(id))
            return Play(handle, entity, userGain);

        amLogError("Cannot play sound: invalid ID (" AM_ID_CHAR_FMT ").", id);
        return Channel(nullptr);
    }

    void EngineImpl::StopAll() const
    {
        for (auto&& channel : _state->channel_state_memory)
            if (channel.Valid())
                channel.Halt();
    }

    EventCanceler EngineImpl::Trigger(EventHandle handle, const Entity& entity) const
    {
        if (handle == nullptr)
        {
            amLogError("Cannot trigger event: Invalid event handle.");
            return EventCanceler(nullptr);
        }

        auto instance = dynamic_cast<EventImpl*>(handle)->Trigger(entity);
        _state->running_events.push_back(std::move(instance));

        return EventCanceler(_state->running_events.back());
    }

    EventCanceler EngineImpl::Trigger(const AmString& name, const Entity& entity) const
    {
        if (Event* handle = GetEventHandle(name))
            return Trigger(handle, entity);

        amLogError("Cannot trigger event: invalid name (%s).", name.c_str());
        return EventCanceler(nullptr);
    }

    EventCanceler EngineImpl::Trigger(AmEventID id, const Entity& entity) const
    {
        if (Event* handle = GetEventHandle(id))
            return Trigger(handle, entity);

        amLogError("Cannot trigger event: invalid ID (" AM_ID_CHAR_FMT ").", id);
        return EventCanceler(nullptr);
    }

    void EngineImpl::CancelEvents(const Entity& entity) const
    {
        std::lock_guard lock(_frameThreadMutex);

        for (auto& event : _state->running_events)
            if (event->GetEntity().GetId() == entity.GetId())
                event->Abort();
    }

    void EngineImpl::CancelAllEvents() const
    {
        std::lock_guard lock(_frameThreadMutex);

        for (auto& event : _state->running_events)
            event->Abort();

        _state->running_events.clear();
    }

    void EngineImpl::SetSwitchState(SwitchHandle handle, AmObjectID stateId) const
    {
        if (handle == nullptr)
        {
            amLogError("Cannot update switch state: Invalid switch handle.");
            return;
        }

        handle->SetState(stateId);
    }

    void EngineImpl::SetSwitchState(SwitchHandle handle, const AmString& stateName) const
    {
        if (handle == nullptr)
        {
            amLogError("Cannot update switch state: Invalid switch handle.");
            return;
        }

        handle->SetState(stateName);
    }

    void EngineImpl::SetSwitchState(SwitchHandle handle, const SwitchState& state) const
    {
        if (handle == nullptr)
        {
            amLogError("Cannot update switch state: Invalid switch handle.");
            return;
        }

        handle->SetState(state);
    }

    void EngineImpl::SetSwitchState(AmSwitchID id, AmObjectID stateId) const
    {
        if (Switch* handle = GetSwitchHandle(id))
            return SetSwitchState(handle, stateId);

        amLogError("Cannot update switch: Invalid ID (" AM_ID_CHAR_FMT ").", id);
    }

    void EngineImpl::SetSwitchState(AmSwitchID id, const AmString& stateName) const
    {
        if (Switch* handle = GetSwitchHandle(id))
            return SetSwitchState(handle, stateName);

        amLogError("Cannot update switch: Invalid ID (" AM_ID_CHAR_FMT ").", id);
    }

    void EngineImpl::SetSwitchState(AmSwitchID id, const SwitchState& state) const
    {
        if (Switch* handle = GetSwitchHandle(id))
            return SetSwitchState(handle, state);

        amLogError("Cannot update switch: Invalid ID (" AM_ID_CHAR_FMT ").", id);
    }

    void EngineImpl::SetSwitchState(const AmString& name, AmObjectID stateId) const
    {
        if (Switch* handle = GetSwitchHandle(name))
            return SetSwitchState(handle, stateId);

        amLogError("Cannot update switch: Invalid name (%s).", name.c_str());
    }

    void EngineImpl::SetSwitchState(const AmString& name, const AmString& stateName) const
    {
        if (Switch* handle = GetSwitchHandle(name))
            return SetSwitchState(handle, stateName);

        amLogError("Cannot update switch: Invalid name (%s).", name.c_str());
    }

    void EngineImpl::SetSwitchState(const AmString& name, const SwitchState& state) const
    {
        if (Switch* handle = GetSwitchHandle(name))
            return SetSwitchState(handle, state);

        amLogError("Cannot update switch: Invalid name (%s).", name.c_str());
    }

    void EngineImpl::SetRtpcValue(RtpcHandle handle, double value) const
    {
        if (handle == nullptr)
        {
            amLogError("Cannot update RTPC value: Invalid RTPC handle.");
            return;
        }

        handle->SetValue(value);
    }

    void EngineImpl::SetRtpcValue(AmRtpcID id, double value) const
    {
        if (Rtpc* handle = GetRtpcHandle(id))
            return SetRtpcValue(handle, value);

        amLogError("Cannot update RTPC value: Invalid RTPC ID (" AM_ID_CHAR_FMT ").", id);
    }

    void EngineImpl::SetRtpcValue(const AmString& name, double value) const
    {
        if (Rtpc* handle = GetRtpcHandle(name))
            return SetRtpcValue(handle, value);

        amLogError("Cannot update RTPC value: Invalid RTPC name (%s).", name.c_str());
    }

    SwitchContainerHandle EngineImpl::GetSwitchContainerHandle(const AmString& name) const
    {
        const auto pair = std::ranges::find_if(
            _state->switch_container_map,
            [&name](const auto& item)
            {
                return item.second->GetName() == name;
            });

        return pair == _state->switch_container_map.end() ? nullptr : pair->second.get();
    }

    SwitchContainerHandle EngineImpl::GetSwitchContainerHandle(AmSwitchContainerID id) const
    {
        const auto pair = _state->switch_container_map.find(id);
        return pair == _state->switch_container_map.end() ? nullptr : pair->second.get();
    }

    SwitchContainerHandle EngineImpl::GetSwitchContainerHandleFromFile(const AmOsString& filename) const
    {
        const auto pair = _state->switch_container_id_map.find(filename);
        return pair == _state->switch_container_id_map.end() ? nullptr : GetSwitchContainerHandle(pair->second);
    }

    CollectionHandle EngineImpl::GetCollectionHandle(const AmString& name) const
    {
        const auto pair = std::ranges::find_if(
            _state->collection_map,
            [&name](const auto& item)
            {
                return item.second->GetName() == name;
            });

        return pair == _state->collection_map.end() ? nullptr : pair->second.get();
    }

    CollectionHandle EngineImpl::GetCollectionHandle(AmCollectionID id) const
    {
        const auto pair = _state->collection_map.find(id);
        return pair == _state->collection_map.end() ? nullptr : pair->second.get();
    }

    CollectionHandle EngineImpl::GetCollectionHandleFromFile(const AmOsString& filename) const
    {
        const auto pair = _state->collection_id_map.find(filename);
        return pair == _state->collection_id_map.end() ? nullptr : GetCollectionHandle(pair->second);
    }

    SoundHandle EngineImpl::GetSoundHandle(const AmString& name) const
    {
        const auto pair = std::ranges::find_if(
            _state->sound_map,
            [&name](const auto& item)
            {
                return item.second->GetName() == name;
            });

        return pair == _state->sound_map.end() ? nullptr : pair->second.get();
    }

    SoundHandle EngineImpl::GetSoundHandle(AmSoundID id) const
    {
        const auto pair = _state->sound_map.find(id);
        return pair == _state->sound_map.end() ? nullptr : pair->second.get();
    }

    SoundHandle EngineImpl::GetSoundHandleFromFile(const AmOsString& filename) const
    {
        const auto pair = _state->sound_id_map.find(filename);
        return pair == _state->sound_id_map.end() ? nullptr : GetSoundHandle(pair->second);
    }

    SoundObjectHandle EngineImpl::GetSoundObjectHandle(const AmString& name) const
    {
        if (Sound* handle = GetSoundHandle(name))
            return handle;

        if (Collection* handle = GetCollectionHandle(name))
            return handle;

        if (SwitchContainer* handle = GetSwitchContainerHandle(name))
            return handle;

        return nullptr;
    }

    SoundObjectHandle EngineImpl::GetSoundObjectHandle(AmSoundID id) const
    {
        if (Sound* handle = GetSoundHandle(id))
            return handle;

        if (Collection* handle = GetCollectionHandle(id))
            return handle;

        if (SwitchContainer* handle = GetSwitchContainerHandle(id))
            return handle;

        return nullptr;
    }

    SoundObjectHandle EngineImpl::GetSoundObjectHandleFromFile(const AmOsString& filename) const
    {
        if (Sound* handle = GetSoundHandleFromFile(filename))
            return handle;

        if (Collection* handle = GetCollectionHandleFromFile(filename))
            return handle;

        if (SwitchContainer* handle = GetSwitchContainerHandleFromFile(filename))
            return handle;

        return nullptr;
    }

    EventHandle EngineImpl::GetEventHandle(const AmString& name) const
    {
        const auto pair = std::ranges::find_if(
            _state->event_map,
            [&name](const auto& item)
            {
                return item.second->GetName() == name;
            });

        return pair == _state->event_map.end() ? nullptr : pair->second.get();
    }

    EventHandle EngineImpl::GetEventHandle(AmEventID id) const
    {
        const auto pair = _state->event_map.find(id);
        return pair == _state->event_map.end() ? nullptr : pair->second.get();
    }

    EventHandle EngineImpl::GetEventHandleFromFile(const AmOsString& filename) const
    {
        const auto pair = _state->event_id_map.find(filename);
        return pair == _state->event_id_map.end() ? nullptr : GetEventHandle(pair->second);
    }

    AttenuationHandle EngineImpl::GetAttenuationHandle(const AmString& name) const
    {
        const auto pair = std::ranges::find_if(
            _state->attenuation_map,
            [&name](const auto& item)
            {
                return item.second->GetName() == name;
            });

        return pair == _state->attenuation_map.end() ? nullptr : pair->second.get();
    }

    AttenuationHandle EngineImpl::GetAttenuationHandle(AmAttenuationID id) const
    {
        const auto pair = _state->attenuation_map.find(id);
        return pair == _state->attenuation_map.end() ? nullptr : pair->second.get();
    }

    AttenuationHandle EngineImpl::GetAttenuationHandleFromFile(const AmOsString& filename) const
    {
        const auto pair = _state->attenuation_id_map.find(filename);
        return pair == _state->attenuation_id_map.end() ? nullptr : GetAttenuationHandle(pair->second);
    }

    SwitchHandle EngineImpl::GetSwitchHandle(const AmString& name) const
    {
        const auto pair = std::ranges::find_if(
            _state->switch_map,
            [&name](const auto& item)
            {
                return item.second->GetName() == name;
            });

        return pair == _state->switch_map.end() ? nullptr : pair->second.get();
    }

    SwitchHandle EngineImpl::GetSwitchHandle(AmSwitchID id) const
    {
        const auto pair = _state->switch_map.find(id);
        return pair == _state->switch_map.end() ? nullptr : pair->second.get();
    }

    SwitchHandle EngineImpl::GetSwitchHandleFromFile(const AmOsString& filename) const
    {
        const auto pair = _state->switch_id_map.find(filename);
        return pair == _state->switch_id_map.end() ? nullptr : GetSwitchHandle(pair->second);
    }

    RtpcHandle EngineImpl::GetRtpcHandle(const AmString& name) const
    {
        const auto pair = std::ranges::find_if(
            _state->rtpc_map,
            [&name](const auto& item)
            {
                return item.second->GetName() == name;
            });

        return pair == _state->rtpc_map.end() ? nullptr : pair->second.get();
    }

    RtpcHandle EngineImpl::GetRtpcHandle(AmRtpcID id) const
    {
        const auto pair = _state->rtpc_map.find(id);
        return pair == _state->rtpc_map.end() ? nullptr : pair->second.get();
    }

    RtpcHandle EngineImpl::GetRtpcHandleFromFile(const AmOsString& filename) const
    {
        const auto pair = _state->rtpc_id_map.find(filename);
        return pair == _state->rtpc_id_map.end() ? nullptr : GetRtpcHandle(pair->second);
    }

    EffectHandle EngineImpl::GetEffectHandle(const AmString& name) const
    {
        const auto pair = std::ranges::find_if(
            _state->effect_map,
            [&name](const auto& item)
            {
                return item.second->GetName() == name;
            });

        return pair == _state->effect_map.end() ? nullptr : pair->second.get();
    }

    EffectHandle EngineImpl::GetEffectHandle(AmEffectID id) const
    {
        const auto pair = _state->effect_map.find(id);
        return pair == _state->effect_map.end() ? nullptr : pair->second.get();
    }

    EffectHandle EngineImpl::GetEffectHandleFromFile(const AmOsString& filename) const
    {
        const auto pair = _state->effect_id_map.find(filename);
        return pair == _state->effect_id_map.end() ? nullptr : GetEffectHandle(pair->second);
    }

    PipelineHandle EngineImpl::GetPipelineHandle() const
    {
        return &_state->pipeline;
    }

    void EngineImpl::SetMasterGain(const AmReal32 gain) const
    {
        if (!IsInitialized() || IsStopping())
            return;

        _state->master_gain = gain;
        _state->mixer.SetMasterGain(gain);
    }

    AmReal32 EngineImpl::GetMasterGain() const
    {
        return _state->master_gain;
    }

    void EngineImpl::SetMute(const bool mute) const
    {
        if (!IsInitialized() || IsStopping())
            return;

        _state->mute = mute;
    }

    bool EngineImpl::IsMuted() const
    {
        return _state->mute;
    }

    void EngineImpl::SetDefaultListener(const Listener* listener)
    {
        if (!IsInitialized() || IsStopping())
            return;

        if (listener == nullptr)
            _defaultListener = nullptr;

        else if (listener->Valid())
            _defaultListener = listener->GetState();
    }

    void EngineImpl::SetDefaultListener(AmListenerID id)
    {
        if (!IsInitialized() || IsStopping())
            return;

        if (id == kAmInvalidObjectId)
            return;

        if (const auto findIt = std::ranges::find_if(
                _state->listener_state_memory,
                [&id](const ListenerInternalState& state)
                {
                    return state.GetId() == id;
                });
            findIt != _state->listener_state_memory.end())
        {
            _defaultListener = (&*findIt);
        }
    }

    Listener EngineImpl::GetDefaultListener() const
    {
        return Listener(_defaultListener);
    }

    Listener EngineImpl::AddListener(AmListenerID id) const
    {
        if (id == kAmInvalidObjectId || _state->listener_state_free_list.empty())
            return Listener(nullptr);

        if (const Listener item = GetListener(id); item.Valid())
            return item;

        ListenerInternalState* listener = _state->listener_state_free_list.back();
        listener->SetId(id);
        _state->listener_state_free_list.pop_back();
        _state->listener_list.push_back(*listener);

        return Listener(listener);
    }

    Listener EngineImpl::GetListener(AmListenerID id) const
    {
        if (id == kAmInvalidObjectId || _state->listener_state_free_list.empty())
            return Listener(nullptr);

        const auto findIt = std::ranges::find_if(
            _state->listener_state_memory,
            [&id](const ListenerInternalState& state)
            {
                return state.GetId() == id;
            });

        return Listener(findIt != _state->listener_state_memory.end() ? &*findIt : nullptr);
    }

    void EngineImpl::RemoveListener(AmListenerID id) const
    {
        if (id == kAmInvalidObjectId || _state->listener_state_free_list.empty())
            return;

        if (const auto findIt = std::ranges::find_if(
                _state->listener_state_memory,
                [&id](const ListenerInternalState& state)
                {
                    return state.GetId() == id;
                });
            findIt != _state->listener_state_memory.end())
        {
            findIt->SetId(kAmInvalidObjectId);
            findIt->node.remove();
            _state->listener_state_free_list.push_back(&*findIt);
        }
    }

    void EngineImpl::RemoveListener(const Listener* listener) const
    {
        if (!listener->Valid())
            return;

        listener->GetState()->SetId(kAmInvalidObjectId);
        listener->GetState()->node.remove();
        _state->listener_state_free_list.push_back(listener->GetState());
    }

    Entity EngineImpl::AddEntity(AmEntityID id) const
    {
        if (id == kAmInvalidObjectId || _state->entity_state_free_list.empty())
            return Entity(nullptr);

        if (const Entity item = GetEntity(id); item.Valid())
            return item;

        EntityInternalState* entity = _state->entity_state_free_list.back();
        entity->SetId(id);
        _state->entity_state_free_list.pop_back();
        _state->entity_list.push_back(*entity);

        return Entity(entity);
    }

    Entity EngineImpl::GetEntity(AmEntityID id) const
    {
        if (id == kAmInvalidObjectId || _state->entity_state_memory.empty())
            return Entity(nullptr);

        const auto findIt = std::ranges::find_if(
            _state->entity_state_memory,
            [&id](const EntityInternalState& state)
            {
                return state.GetId() == id;
            });

        return Entity(findIt != _state->entity_state_memory.end() ? &*findIt : nullptr);
    }

    void EngineImpl::RemoveEntity(const Entity* entity) const
    {
        if (!entity->Valid())
            return;

        entity->GetState()->SetId(kAmInvalidObjectId);
        entity->GetState()->node.remove();
        _state->entity_state_free_list.push_back(entity->GetState());
    }

    void EngineImpl::RemoveEntity(AmEntityID id) const
    {
        if (id == kAmInvalidObjectId || _state->entity_state_memory.empty())
            return;

        if (const auto findIt = std::ranges::find_if(
                _state->entity_state_memory,
                [&id](const EntityInternalState& state)
                {
                    return state.GetId() == id;
                });
            findIt != _state->entity_state_memory.end())
        {
            findIt->SetId(kAmInvalidObjectId);
            findIt->node.remove();
            _state->entity_state_free_list.push_back(&*findIt);
        }
    }

    Environment EngineImpl::AddEnvironment(AmEnvironmentID id) const
    {
        if (id == kAmInvalidObjectId || _state->environment_state_free_list.empty())
            return Environment(nullptr);

        if (const Environment item = GetEnvironment(id); item.Valid())
            return item;

        EnvironmentInternalState* environment = _state->environment_state_free_list.back();
        environment->SetId(id);
        _state->environment_state_free_list.pop_back();
        _state->environment_list.push_back(*environment);

        return Environment(environment);
    }

    Environment EngineImpl::GetEnvironment(AmEnvironmentID id) const
    {
        if (id == kAmInvalidObjectId || _state->environment_state_free_list.empty())
            return Environment(nullptr);

        const auto findIt = std::ranges::find_if(
            _state->environment_state_memory,
            [&id](const EnvironmentInternalState& state)
            {
                return state.GetId() == id;
            });

        return Environment(findIt != _state->environment_state_memory.end() ? &*findIt : nullptr);
    }

    void EngineImpl::RemoveEnvironment(const Environment* environment) const
    {
        if (!environment->Valid())
            return;

        environment->GetState()->SetId(kAmInvalidObjectId);
        environment->GetState()->node.remove();
        _state->environment_state_free_list.push_back(environment->GetState());
    }

    void EngineImpl::RemoveEnvironment(AmEnvironmentID id) const
    {
        if (id == kAmInvalidObjectId || _state->environment_state_free_list.empty())
            return;

        if (const auto findIt = std::ranges::find_if(
                _state->environment_state_memory,
                [&id](const EnvironmentInternalState& state)
                {
                    return state.GetId() == id;
                });
            findIt != _state->environment_state_memory.end())
        {
            findIt->SetId(kAmInvalidObjectId);
            findIt->node.remove();
            _state->environment_state_free_list.push_back(&*findIt);
        }
    }

    Room EngineImpl::AddRoom(AmRoomID id) const
    {
        if (id == kAmInvalidObjectId || _state->room_state_free_list.empty())
            return Room(nullptr);

        if (const Room item = GetRoom(id); item.Valid())
            return item;

        RoomInternalState* room = _state->room_state_free_list.back();
        room->SetId(id);
        _state->room_state_free_list.pop_back();
        _state->room_list.push_back(*room);

        return Room(room);
    }

    Room EngineImpl::GetRoom(AmRoomID id) const
    {
        if (id == kAmInvalidObjectId || _state->room_state_free_list.empty())
            return Room(nullptr);

        const auto findIt = std::ranges::find_if(
            _state->room_state_memory,
            [&id](const RoomInternalState& state)
            {
                return state.GetId() == id;
            });

        return Room(findIt != _state->room_state_memory.end() ? &*findIt : nullptr);
    }

    void EngineImpl::RemoveRoom(const Room* room) const
    {
        if (!room->Valid())
            return;

        room->GetState()->SetId(kAmInvalidObjectId);
        room->GetState()->node.remove();
        _state->room_state_free_list.push_back(room->GetState());
    }

    void EngineImpl::RemoveRoom(AmRoomID id) const
    {
        if (id == kAmInvalidObjectId || _state->room_state_free_list.empty())
            return;

        if (const auto findIt = std::ranges::find_if(
                _state->room_state_memory,
                [&id](const RoomInternalState& state)
                {
                    return state.GetId() == id;
                });
            findIt != _state->room_state_memory.end())
        {
            findIt->SetId(kAmInvalidObjectId);
            findIt->node.remove();
            _state->room_state_free_list.push_back(&*findIt);
        }
    }

    Bus EngineImpl::FindBus(const AmString& name) const
    {
        return Bus(FindBusInternalState(_state, name).get());
    }

    Bus EngineImpl::FindBus(AmBusID id) const
    {
        return Bus(FindBusInternalState(_state, id).get());
    }

    void EngineImpl::Pause(bool pause) const
    {
        if (!IsInitialized() || IsStopping())
            return;

        if (_state->paused == pause)
            return;

        _state->paused = pause;

        for (PriorityList& list = _state->playing_channel_list; auto&& state : list)
        {
            if (!state.Paused() && state.IsReal())
            {
                if (pause)
                {
                    // Pause the real channel underlying this virtual channel. This freezes
                    // playback of the channel without marking it as paused from the audio
                    // engine's point of view, so that we know to restart it when the audio
                    // engine is not paused.
                    state.GetRealChannel().Pause();
                }
                else
                {
                    // Resumed all channels that were not explicitly paused.
                    state.GetRealChannel().Resume();
                }
            }
        }
    }

    bool EngineImpl::IsPaused() const
    {
        return _state->paused;
    }

    void EraseFinishedSounds(const std::shared_ptr<EngineInternalState>& state)
    {
        PriorityList& list = state->playing_channel_list;
        for (auto channelInternalState = list.begin(); channelInternalState != list.end();)
        {
            auto current = channelInternalState++;
            current->UpdateState();
            if (current->Stopped())
            {
                InsertIntoFreeList(state, &*current);
            }
        }
    }

    static void UpdateChannel(ChannelInternalState* channel, std::shared_ptr<EngineInternalState> state)
    {
        if (channel->Stopped())
            return;

        AmReal32 gain;
        AmReal32 pitch;

        bool isEntityScope = false;

        // Find the best listener for this channel.
        ListenerInternalState* listener =
            FindBestListener(state->listener_list, channel->GetLocation(), state->listener_fetch_mode, &state->listenerCache);

        if (const SwitchContainer* switchContainer = channel->GetSwitchContainer(); switchContainer != nullptr)
        {
            CalculateGainAndPitch(
                &gain, &pitch, listener, nullptr, switchContainer->GetGain().GetValue(), switchContainer->GetPitch().GetValue(),
                switchContainer->GetBus().GetState(), switchContainer->GetSpatialization(), channel->GetUserGain());

            isEntityScope = switchContainer->GetScope() == eScope_Entity;
        }
        else if (const Collection* collection = channel->GetCollection(); collection != nullptr)
        {
            CalculateGainAndPitch(
                &gain, &pitch, listener, nullptr, collection->GetGain().GetValue(), collection->GetPitch().GetValue(),
                collection->GetBus().GetState(), collection->GetSpatialization(), channel->GetUserGain());

            isEntityScope = collection->GetScope() == eScope_Entity;
        }
        else if (const Sound* sound = channel->GetSound(); sound != nullptr)
        {
            CalculateGainAndPitch(
                &gain, &pitch, listener, nullptr, sound->GetGain().GetValue(), sound->GetPitch().GetValue(), sound->GetBus().GetState(),
                sound->GetSpatialization(), channel->GetUserGain());

            isEntityScope = sound->GetScope() == eScope_Entity;
        }
        else
        {
            AMPLITUDE_ASSERT(false);
        }

        if (isEntityScope)
            AssignBestRoom(channel, channel->GetLocation(), state);

        channel->SetGain(gain);
        channel->SetPitch(pitch);
        channel->SetListener(Listener(listener));
    }

    // If there are any free real channels, assign those to virtual channels that
    // need them. If the priority list has gaps (i.e. if there are real channels
    // that are lower priority than virtual channels) then move the lower priority
    // real channels to the higher priority virtual channels.
    static void UpdateRealChannels(PriorityList* priorityList, FreeList* realFreeList, FreeList* virtualFreeList)
    {
        auto reverseIterator = priorityList->rbegin();
        for (auto state = priorityList->begin(); state != priorityList->end(); ++state)
        {
            if (!state->IsReal())
            {
                // First check if there are any free real channels.
                if (!realFreeList->empty())
                {
                    // We have a free real channel. Assign this channel id to the channel
                    // that is trying to resume, clear the free channel, and push it into
                    // the virtual free list.
                    ChannelInternalState* freeChannel = &realFreeList->front();
                    state->Devirtualize(freeChannel);

                    freeChannel->Remove();
                    virtualFreeList->push_front(*freeChannel);

                    state->Resume();
                }
                else if (&*reverseIterator != &*state)
                {
                    // If there aren't any free channels, then scan from the back of the
                    // list for low priority real channels.
                    reverseIterator = std::find_if(
                        reverseIterator, PriorityList::reverse_iterator(state),
                        [](const ChannelInternalState& channel)
                        {
                            return channel.GetRealChannel().Valid();
                        });
                    if (reverseIterator == priorityList->rend())
                    {
                        // There is no more swapping that can be done. Return.
                        return;
                    }
                    // Found a real channel that we can give to the higher priority
                    // channel.
                    state->Devirtualize(&*reverseIterator);
                }
            }
        }
    }

    void EngineImpl::AdvanceFrame(AmTime delta) const
    {
        std::lock_guard lock(_updateMutex);

        if (!IsInitialized())
            return;

        if (_state->paused)
            return;

        // It is safe to clear the listener cache here because it is only used to
        // find the best listener for each channel. Listener positions are updated
        // before the best listener is selected.
        _state->listenerCache.Clear();

        {
            std::lock_guard lock(_frameThreadMutex);

            // Execute pending frame callbacks.
            std::function<void(AmTime)> callback;
            while (_nextFrameCallbacks.TryDequeue(callback))
                callback(delta);
        }

        EraseFinishedSounds(_state);

        _state->room_list.sort(
            [](const RoomInternalState& a, const RoomInternalState& b) -> bool
            {
                return a.GetVolume() > b.GetVolume();
            });

        for (const auto& rtpc : _state->rtpc_map | std::views::values)
            rtpc->Update(delta);

        for (const auto& effect : _state->effect_map | std::views::values)
            effect->Update();

        for (auto&& state : _state->listener_list)
            state.Update();

        for (auto&& state : _state->environment_list)
            state.Update();

        for (auto&& state : _state->room_list)
            state.Update();

        for (auto&& state : _state->entity_list)
        {
            state.Update();

            if (!_state->track_environments)
            {
                bool needsRecalc = state.AreEnvironmentFactorsDirty();

                for (auto&& env : _state->environment_list)
                {
                    // Skip if cached and environment unchanged
                    if (!needsRecalc && state.IsEnvironmentVersionCurrent(env.GetId(), env.GetVersion()))
                        continue;

                    // Recalculate and cache
                    AmReal32 factor = env.GetFactor(Entity(&state));
                    state.SetEnvironmentFactor(env.GetId(), factor);
                    state.UpdateEnvironmentVersion(env.GetId(), env.GetVersion());
                }

                state.MarkEnvironmentFactorsClean();
            }
        }

        for (auto&& bus : _state->buses)
            bus->ResetDuckGain();

        for (auto&& bus : _state->buses)
            bus->UpdateDuckGain(delta);

        if (_state->master_bus)
        {
            const AmReal32 masterGain = _state->mute.load() ? 0.0f : _state->master_gain.load();
            _state->master_bus->AdvanceFrame(delta, masterGain);
        }

        for (auto&& state : _state->playing_channel_list)
        {
            UpdateChannel(&state, _state);
            if (state.IsPriorityDirty())
                _state->channelPriorityDirty = true;
        }

        // Only sort if any channel's priority changed
        if (_state->channelPriorityDirty)
        {
            _state->playing_channel_list.sort(ChannelPriorityComparator{});
            _state->channelPriorityDirty = false;
        }

        UpdateRealChannels(&_state->playing_channel_list, &_state->real_channel_free_list, &_state->virtual_channel_free_list);

        for (AmSize i = 0; i < _state->running_events.size(); ++i)
        {
            auto event = _state->running_events[i];

            if (!event->IsRunning())
            {
                _state->running_events.erase(_state->running_events.begin() + i);
                --i;
                continue;
            }

            event->AdvanceFrame(delta);
        }

        ++_state->current_frame;
        _state->total_time += delta;
    }

    void EngineImpl::OnNextFrame(std::function<void(AmTime delta)> callback) const
    {
        std::lock_guard lock(_frameThreadMutex);
        _nextFrameCallbacks.TryEnqueue(std::move(callback));
    }

    void EngineImpl::WaitUntilNextFrame() const
    {
        WaitUntilFrames(1);
    }

    void EngineImpl::WaitUntilFrames(AmUInt64 frameCount) const
    {
        const AmUInt64 nextFrame = _state->current_frame + frameCount;
        while (_state->current_frame < nextFrame)
            Thread::Sleep(1);
    }

    AmTime EngineImpl::GetTotalTime() const
    {
        return _state->total_time;
    }

    const AmVersion* EngineImpl::Version() const
    {
        return _state->version;
    }

    const EngineConfigDefinition* EngineImpl::GetEngineConfigDefinition() const
    {
        return Amplitude::GetEngineConfigDefinition(_configSrc.c_str());
    }

    std::shared_ptr<Driver> EngineImpl::GetDriver() const
    {
        return _audioDriver;
    }

#pragma region Amplimix

    Amplimix* EngineImpl::GetMixer() const
    {
        return &_state->mixer;
    }

#pragma endregion

#pragma region Engine State

    std::shared_ptr<EngineInternalState> EngineImpl::GetState() const
    {
        return _state;
    }

    const AmOsString& EngineImpl::GetConfigurationPath() const
    {
        return _configFilePath;
    }

    AmReal32 EngineImpl::GetSoundSpeed() const
    {
        return _state->sound_speed;
    }

    AmReal32 EngineImpl::GetDopplerFactor() const
    {
        return _state->doppler_factor;
    }

    AmUInt32 EngineImpl::GetActiveEntitiesCount() const
    {
        return _state->entity_list.size();
    }

    AmUInt32 EngineImpl::GetActiveListenersCount() const
    {
        return _state->listener_list.size();
    }

    AmUInt32 EngineImpl::GetActiveEnvironmentsCount() const
    {
        return _state->environment_list.size();
    }

    AmUInt32 EngineImpl::GetActiveRoomsCount() const
    {
        return _state->room_list.size();
    }

    AmUInt32 EngineImpl::GetSamplesPerStream() const
    {
        return _state->samples_per_stream;
    }

    bool EngineImpl::IsGameTrackingEnvironmentAmounts() const
    {
        return _state->track_environments;
    }

    AmUInt32 EngineImpl::GetMaxListenersCount() const
    {
        return GetEngineConfigDefinition()->game()->listeners();
    }

    AmUInt32 EngineImpl::GetMaxEntitiesCount() const
    {
        return GetEngineConfigDefinition()->game()->entities();
    }

    AmUInt32 EngineImpl::GetMaxEnvironmentsCount() const
    {
        return GetEngineConfigDefinition()->game()->environments();
    }

    AmUInt32 EngineImpl::GetMaxRoomsCount() const
    {
        return GetEngineConfigDefinition()->game()->rooms();
    }

    const Curve& EngineImpl::GetOcclusionCoefficientCurve() const
    {
        return _state->occlusion_config.lpf;
    }

    const Curve& EngineImpl::GetOcclusionGainCurve() const
    {
        return _state->occlusion_config.gain;
    }

    const Curve& EngineImpl::GetObstructionCoefficientCurve() const
    {
        return _state->obstruction_config.lpf;
    }

    const Curve& EngineImpl::GetObstructionGainCurve() const
    {
        return _state->obstruction_config.gain;
    }

    ePanningMode EngineImpl::GetPanningMode() const
    {
        return _state->panning_mode;
    }

    eHRIRSphereSamplingMode EngineImpl::GetHRIRSphereSamplingMode() const
    {
        return _state->hrir_sampling_mode;
    }

    std::shared_ptr<const HRIRSphere> EngineImpl::GetHRIRSphere() const
    {
        return _state->hrir_sphere;
    }

    const AmString& EngineImpl::GetResampler() const
    {
        return _state->resampler;
    }

    Channel EngineImpl::GetChannel(AmChannelID channelID) const
    {
        const auto findIt = std::ranges::find_if(
            _state->channel_state_memory,
            [channelID](const ChannelInternalState& state)
            {
                return (state.GetChannelStateId() == channelID);
            });

        return Channel(findIt != _state->channel_state_memory.end() ? &*findIt : nullptr);
    }

    bool EngineImpl::IsStopping() const
    {
        return _state->stopping;
    }

#pragma endregion

    Channel EngineImpl::PlayScopedSwitchContainer(
        SwitchContainerHandle handle, const Entity& entity, const AmVector3& location, const AmReal32 userGain) const
    {
        if (handle == nullptr)
        {
            amLogError("Cannot play switch container: Invalid switch container handle.");
            return Channel(nullptr);
        }

        bool isEntityScope = handle->GetScope() == eScope_Entity;

        if (isEntityScope && !entity.Valid())
        {
            amLogError("Cannot play a switch container in Entity scope. No entity defined.");
            return Channel(nullptr);
        }

        if (entity.Valid())
        {
            // Process the first entity update
            entity.GetState()->Update();
        }

        // Find the best listener for this channel.
        ListenerInternalState* listener = FindBestListener(
            _state->listener_list, isEntityScope ? entity.GetLocation() : location, _state->listener_fetch_mode, &_state->listenerCache);

        // Find where it belongs in the list.
        AmReal32 gain;
        AmReal32 pitch;
        CalculateGainAndPitch(
            &gain, &pitch, listener, nullptr, handle->GetGain().GetValue(), handle->GetPitch().GetValue(), handle->GetBus().GetState(),
            handle->GetSpatialization(), userGain);
        const AmReal32 priority = gain * handle->GetPriority().GetValue();
        const auto insertionPoint = FindInsertionPoint(&_state->playing_channel_list, priority);

        // Decide which ChannelInternalState object to use.
        ChannelInternalState* newChannel = FindFreeChannelInternalState(
            insertionPoint, &_state->playing_channel_list, &_state->real_channel_free_list, &_state->virtual_channel_free_list,
            _state->paused);

        // The channel could not be added to the list; not a high enough priority.
        if (newChannel == nullptr)
        {
            amLogDebug("Cannot play switch container: Not high enough priority.");
            return Channel(nullptr);
        }

        newChannel->SetChannelState(eChannelPlaybackState_Pending);

        // Now that we have our new channel, set the data on it and update the next pointers.
        if (isEntityScope)
            newChannel->SetEntity(entity);

        newChannel->SetSwitchContainer(dynamic_cast<SwitchContainerImpl*>(handle));
        newChannel->SetUserGain(userGain);

        if (isEntityScope)
            AssignBestRoom(newChannel, location, _state);

        newChannel->SetGain(gain);
        newChannel->SetPitch(pitch);
        newChannel->SetLocation(location);
        newChannel->SetListener(Listener(listener));

        // Attempt to play the channel, if the engine is paused, the channel will be played later.
        OnNextFrame(
            [this, newChannel, handle](AmTime delta)
            {
                if (!newChannel->Play())
                {
                    amLogError("Failed to play switch container: %s.", handle->GetName().c_str());

                    // Error playing the sound, put it back in the free list.
                    InsertIntoFreeList(_state, newChannel);
                }
            });

        return Channel(newChannel);
    }

    Channel EngineImpl::PlayScopedCollection(
        CollectionHandle handle, const Entity& entity, const AmVector3& location, const AmReal32 userGain) const
    {
        if (handle == nullptr)
        {
            amLogError("Cannot play collection: Invalid collection handle.");
            return Channel(nullptr);
        }

        bool isEntityScope = handle->GetScope() == eScope_Entity;

        if (isEntityScope && !entity.Valid())
        {
            amLogError("Cannot play a collection in Entity scope. No entity defined.");
            return Channel(nullptr);
        }

        if (entity.Valid())
        {
            // Process the first entity update
            entity.GetState()->Update();
        }

        // Find the best listener for this channel.
        ListenerInternalState* listener = FindBestListener(
            _state->listener_list, isEntityScope ? entity.GetLocation() : location, _state->listener_fetch_mode, &_state->listenerCache);

        // Find where it belongs in the list.
        AmReal32 gain;
        AmReal32 pitch;
        CalculateGainAndPitch(
            &gain, &pitch, listener, nullptr, handle->GetGain().GetValue(), handle->GetPitch().GetValue(), handle->GetBus().GetState(),
            handle->GetSpatialization(), userGain);
        const AmReal32 priority = gain * handle->GetPriority().GetValue();
        const auto insertionPoint = FindInsertionPoint(&_state->playing_channel_list, priority);

        // Decide which ChannelInternalState object to use.
        ChannelInternalState* newChannel = FindFreeChannelInternalState(
            insertionPoint, &_state->playing_channel_list, &_state->real_channel_free_list, &_state->virtual_channel_free_list,
            _state->paused);

        // The channel could not be added to the list; not high enough priority.
        if (newChannel == nullptr)
        {
            amLogDebug("Cannot play collection: Not high enough priority.");
            return Channel(nullptr);
        }

        newChannel->SetChannelState(eChannelPlaybackState_Pending);

        // Now that we have our new channel, set the data on it and update the next pointers.
        if (isEntityScope)
            newChannel->SetEntity(entity);

        newChannel->SetCollection(dynamic_cast<CollectionImpl*>(handle));
        newChannel->SetUserGain(userGain);

        if (isEntityScope)
            AssignBestRoom(newChannel, location, _state);

        newChannel->SetGain(gain);
        newChannel->SetPitch(pitch);
        newChannel->SetLocation(location);
        newChannel->SetListener(Listener(listener));

        // Attempt to play the channel, if the engine is paused, the channel will be played later.
        OnNextFrame(
            [this, newChannel, handle](AmTime delta)
            {
                if (!newChannel->Play())
                {
                    amLogError("Failed to play collection: %s.", handle->GetName().c_str());

                    // Error playing the sound, put it back in the free list.
                    InsertIntoFreeList(_state, newChannel);
                }
            });

        return Channel(newChannel);
    }

    Channel EngineImpl::PlayScopedSound(SoundHandle handle, const Entity& entity, const AmVector3& location, AmReal32 userGain) const
    {
        if (handle == nullptr)
        {
            amLogError("Cannot play sound: Invalid sound handle.");
            return Channel(nullptr);
        }

        bool isEntityScope = handle->GetScope() == eScope_Entity;

        if (isEntityScope && !entity.Valid())
        {
            amLogError("Cannot play a sound in Entity scope. No entity defined.");
            return Channel(nullptr);
        }

        if (entity.Valid())
        {
            // Process the first entity update
            entity.GetState()->Update();
        }

        // Find the best listener for this channel.
        ListenerInternalState* listener = FindBestListener(
            _state->listener_list, isEntityScope ? entity.GetLocation() : location, _state->listener_fetch_mode, &_state->listenerCache);

        // Find where it belongs in the list.
        AmReal32 gain;
        AmReal32 pitch;
        CalculateGainAndPitch(
            &gain, &pitch, listener, nullptr, handle->GetGain().GetValue(), handle->GetPitch().GetValue(), handle->GetBus().GetState(),
            handle->GetSpatialization(), userGain);
        const AmReal32 priority = gain * handle->GetPriority().GetValue();
        const auto insertionPoint = FindInsertionPoint(&_state->playing_channel_list, priority);

        // Decide which ChannelInternalState object to use.
        ChannelInternalState* newChannel = FindFreeChannelInternalState(
            insertionPoint, &_state->playing_channel_list, &_state->real_channel_free_list, &_state->virtual_channel_free_list,
            _state->paused);

        // The sound could not be added to the list; not high enough priority.
        if (newChannel == nullptr)
        {
            amLogDebug("Cannot play sound: Not high enough priority.");
            return Channel(nullptr);
        }

        newChannel->SetChannelState(eChannelPlaybackState_Pending);

        // Now that we have our new channel, set the data on it and update the next pointers.
        if (isEntityScope)
            newChannel->SetEntity(entity);

        newChannel->SetSound(dynamic_cast<SoundImpl*>(handle));
        newChannel->SetUserGain(userGain);

        if (isEntityScope)
            AssignBestRoom(newChannel, location, _state);

        newChannel->SetGain(gain);
        newChannel->SetPitch(pitch);
        newChannel->SetLocation(location);
        newChannel->SetListener(Listener(listener));

        // Attempt to play the channel, if the engine is paused, the channel will be played later.
        OnNextFrame(
            [this, newChannel, handle](AmTime delta)
            {
                if (!newChannel->Play())
                {
                    amLogError("Failed to play sound: %s.", handle->GetName().c_str());

                    // Error playing the sound, put it back in the free list.
                    InsertIntoFreeList(_state, newChannel);
                }
            });

        return Channel(newChannel);
    }
} // namespace SparkyStudios::Audio::Amplitude
