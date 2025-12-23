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

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Core/Engine.h>
#include <Core/EntityInternalState.h>
#include <Core/Event.h>

namespace SparkyStudios::Audio::Amplitude
{
    EventAction::EventAction()
        : EventAction(nullptr)
    {}

    EventAction::EventAction(EventImpl* parent)
        : _active(false)
        , _type(EventActionType_None)
        , _scope(Scope_Entity)
        , _targets()
        , _accumulatedTime(0.0)
        , _parent(parent)
    {}

    void EventAction::Initialize(const EventActionDefinition* definition)
    {
        _active = definition->active();
        _type = definition->type();
        _scope = definition->scope();
        const flatbuffers::uoffset_t targets_count = definition->targets() ? definition->targets()->size() : 0;
        for (flatbuffers::uoffset_t i = 0; i < targets_count; ++i)
            _targets.push_back(definition->targets()->Get(i));
    }

    void EventAction::Run(const Entity& entity)
    {
        if (!_active)
            return;

        switch (_type)
        {
        default:
            [[fallthrough]];
        case EventActionType_None:
            return;

        case EventActionType_Play:
            return ExecutePlay(entity);

        case EventActionType_Pause:
            return ExecutePause(entity);

        case EventActionType_Resume:
            return ExecuteResume(entity);

        case EventActionType_Stop:
            return ExecuteStop(entity);

        case EventActionType_Seek:
            return ExecuteSeek(entity);

        case EventActionType_MuteBus:
            return ExecuteMute(entity, true);

        case EventActionType_UnmuteBus:
            return ExecuteMute(entity, false);

        case EventActionType_Wait:
            return ExecuteWait(entity);
        }
    }

    void EventAction::Run()
    {
        if (_scope == Scope_Entity)
        {
            AMPLITUDE_ASSERT(false);
            amLogWarning("Running an entity scoped event action without an entity.");
        }

        Run(Entity(nullptr));
    }

    void EventAction::Abort()
    {
        if (!_active)
            return;

        switch (_type)
        {
        default:
            [[fallthrough]];
        case EventActionType_None:
            return;

        case EventActionType_Play:
            return AbortPlay();

        case EventActionType_Pause:
            return AbortPause();

        case EventActionType_Resume:
            return AbortResume();

        case EventActionType_Stop:
            return AbortStop();

        case EventActionType_Seek:
            return AbortSeek();

        case EventActionType_MuteBus:
            return AbortMute(true);

        case EventActionType_UnmuteBus:
            return AbortMute(false);

        case EventActionType_Wait:
            return AbortWait();
        }
    }

    void EventAction::AdvanceFrame(AmTime delta_time)
    {
        if (!_active)
            return;

        switch (_type)
        {
        default:
            [[fallthrough]];
        case EventActionType_None:
            return;

        case EventActionType_Wait:
            _accumulatedTime += delta_time;
        }
    }

    bool EventAction::IsExecuting(const Entity& entity) const
    {
        if (!_active)
            return false;

        switch (_type)
        {
        case EventActionType_None:
            return false;

        case EventActionType_Play:
            return IsExecutingPlay(entity);

        case EventActionType_Pause:
            return IsExecutingPause(entity);

        case EventActionType_Resume:
            return IsExecutingResume(entity);

        case EventActionType_Stop:
            return IsExecutingStop(entity);

        case EventActionType_Seek:
            return false;

        case EventActionType_MuteBus:
            return IsExecutingMute(entity, true);

        case EventActionType_UnmuteBus:
            return IsExecutingMute(entity, false);

        case EventActionType_Wait:
            return IsExecutingWait(entity);
        }

        return false;
    }

    void EventAction::ExecutePlay(const Entity& entity)
    {
        if (_scope == Scope_Entity)
        {
            if (!entity.Valid())
            {
                amLogWarning("Running an entity scoped event action without an entity.");
                return;
            }

            for (auto&& target : _targets)
                _playingChannels.push_back(amEngine->Play(target, entity));
        }
        else
        {
            for (auto&& target : _targets)
                _playingChannels.push_back(amEngine->Play(target));
        }
    }

    void EventAction::ExecutePause(const Entity& entity)
    {
        if (_scope == Scope_Entity)
        {
            if (!entity.Valid())
            {
                amLogWarning("Running an entity scoped event action without an entity.");
                return;
            }

            for (auto&& target : _targets)
                for (auto&& item : entity.GetState()->GetPlayingSoundList())
                    if (target == item.GetPlayingObjectId())
                        item.Pause();
        }
        else
        {
            for (auto&& target : _targets)
                for (auto&& item : amEngine->GetState()->playing_channel_list)
                    if (target == item.GetPlayingObjectId())
                        item.Pause();
        }
    }

    void EventAction::ExecuteResume(const Entity& entity)
    {
        if (_scope == Scope_Entity)
        {
            if (!entity.Valid())
            {
                amLogWarning("Running an entity scoped event action without an entity.");
                return;
            }

            for (auto&& target : _targets)
                for (auto&& item : entity.GetState()->GetPlayingSoundList())
                    if (target == item.GetPlayingObjectId())
                        item.Resume();
        }
        else
        {
            for (auto&& target : _targets)
                for (auto&& item : amEngine->GetState()->playing_channel_list)
                    if (target == item.GetPlayingObjectId())
                        item.Resume();
        }
    }

    void EventAction::ExecuteStop(const Entity& entity)
    {
        if (_scope == Scope_Entity)
        {
            if (!entity.Valid())
            {
                amLogWarning("Running an entity scoped event action without an entity.");
                return;
            }

            for (auto&& target : _targets)
                for (auto&& item : entity.GetState()->GetPlayingSoundList())
                    if (target == item.GetPlayingObjectId())
                        item.Halt();
        }
        else
        {
            for (auto&& target : _targets)
                for (auto&& item : amEngine->GetState()->playing_channel_list)
                    if (target == item.GetPlayingObjectId())
                        item.Halt();
        }
    }

    void EventAction::ExecuteSeek(const Entity& entity)
    {
        // noop
    }

    void EventAction::ExecuteMute(const Entity& entity, bool mute)
    {
        for (auto&& target : _targets)
            if (Bus bus = amEngine->FindBus(target); bus.Valid())
                bus.SetMute(mute);
    }

    void EventAction::ExecuteWait(const Entity& entity)
    {
        _accumulatedTime = 0.0;
    }

    bool EventAction::IsExecutingPlay(const Entity& entity) const
    {
        for (auto&& channel : _playingChannels)
            if (channel.GetPlaybackState() == eChannelPlaybackState_Playing)
                return true;

        return false;
    }

    bool EventAction::IsExecutingPause(const Entity& entity) const
    {
        if (_scope == Scope_Entity)
        {
            if (!entity.Valid())
            {
                amLogWarning("Running an entity scoped event action without an entity.");
                return false;
            }

            for (auto&& target : _targets)
                for (auto&& item : entity.GetState()->GetPlayingSoundList())
                    if (target == item.GetPlayingObjectId() && item.Valid() && item.GetChannelState() == eChannelPlaybackState_Playing)
                        return true;
        }
        else
        {
            for (auto&& target : _targets)
                for (auto&& item : amEngine->GetState()->playing_channel_list)
                    if (target == item.GetPlayingObjectId() && item.Valid() && item.GetChannelState() == eChannelPlaybackState_Playing)
                        return true;
        }

        return false;
    }

    bool EventAction::IsExecutingResume(const Entity& entity) const
    {
        if (_scope == Scope_Entity)
        {
            if (!entity.Valid())
            {
                amLogWarning("Running an entity scoped event action without an entity.");
                return false;
            }

            for (auto&& target : _targets)
                for (auto&& item : entity.GetState()->GetPlayingSoundList())
                    if (target == item.GetPlayingObjectId() && item.Valid() && item.GetChannelState() == eChannelPlaybackState_Paused)
                        return true;
        }
        else
        {
            for (auto&& target : _targets)
                for (auto&& item : amEngine->GetState()->playing_channel_list)
                    if (target == item.GetPlayingObjectId() && item.Valid() && item.GetChannelState() == eChannelPlaybackState_Paused)
                        return true;
        }

        return false;
    }

    bool EventAction::IsExecutingStop(const Entity& entity) const
    {
        if (_scope == Scope_Entity)
        {
            if (!entity.Valid())
            {
                amLogWarning("Running an entity scoped event action without an entity.");
                return false;
            }

            for (auto&& target : _targets)
                for (auto&& item : entity.GetState()->GetPlayingSoundList())
                    if (target == item.GetPlayingObjectId() && item.Valid() && item.GetChannelState() != eChannelPlaybackState_Stopped)
                        return true;
        }
        else
        {
            for (auto&& target : _targets)
                for (auto&& item : amEngine->GetState()->playing_channel_list)
                    if (target == item.GetPlayingObjectId() && item.Valid() && item.GetChannelState() != eChannelPlaybackState_Stopped)
                        return true;
        }

        return false;
    }

    bool EventAction::IsExecutingSeek(const Entity& entity) const
    {
        return false;
    }

    bool EventAction::IsExecutingMute(const Entity& entity, bool mute) const
    {
        for (auto&& target : _targets)
            if (Bus bus = amEngine->FindBus(target); bus.Valid() && bus.IsMuted() != mute)
                return true;

        return false;
    }

    bool EventAction::IsExecutingWait(const Entity& entity) const
    {
        return _accumulatedTime < static_cast<AmTime>(_targets[0]);
    }

    void EventAction::AbortPlay()
    {
        for (auto&& channel : _playingChannels)
            channel.Stop(0);
    }

    void EventAction::AbortPause()
    {
        // noop
    }

    void EventAction::AbortResume()
    {
        // noop
    }

    void EventAction::AbortStop()
    {
        // noop
    }

    void EventAction::AbortSeek()
    {
        // noop
    }

    void EventAction::AbortMute(bool mute)
    {
        // noop
    }

    void EventAction::AbortWait()
    {
        _accumulatedTime = static_cast<AmTime>(_targets[0]);
    }

    EventImpl::EventImpl()
        : _runMode(EventActionRunningMode_Parallel)
        , _actions()
    {}

    EventImpl::~EventImpl()
    {
        _actions.clear();
    }

    std::shared_ptr<EventInstanceImpl> EventImpl::Trigger(const Entity& entity) const
    {
        amLogDebug("Event '%s' triggered.", m_name.c_str());

        auto event = ampoolshared(eMemoryPoolKind_Engine, EventInstanceImpl, this);
        event->Start(entity);

        return event;
    }

    bool EventImpl::LoadDefinition(const EventDefinition* definition, std::shared_ptr<EngineInternalState> state)
    {
        m_id = definition->id();
        m_name = definition->name()->str();

        _runMode = definition->run_mode();

        const flatbuffers::uoffset_t actions_count = definition->actions() ? definition->actions()->size() : 0;
        _actions.resize(actions_count);

        for (flatbuffers::uoffset_t i = 0; i < actions_count; ++i)
        {
            const EventActionDefinition* item = definition->actions()->Get(i);

            EventAction& action = _actions[i];
            action._parent = this;
            action.Initialize(item);
        }

        return true;
    }

    const EventDefinition* EventImpl::GetDefinition() const
    {
        return GetEventDefinition(m_source.c_str());
    }

    EventCanceler::EventCanceler()
        : EventCanceler(nullptr)
    {}

    EventCanceler::EventCanceler(std::shared_ptr<EventInstance> event)
        : _event(std::move(event))
    {}

    EventCanceler::~EventCanceler()
    {
        if (_event == nullptr)
            return;

        _event = nullptr;
    }

    bool EventCanceler::Valid() const
    {
        return _event != nullptr && _event->IsRunning();
    }

    void EventCanceler::Cancel() const
    {
        if (!Valid())
            return;

        _event->Abort();
    }

    std::shared_ptr<EventInstance> EventCanceler::GetEvent() const
    {
        return _event;
    }

    EventInstanceImpl::EventInstanceImpl()
        : _runMode(EventActionRunningMode_Parallel)
        , _actions()
        , _running(false)
        , _runningActionIndex(0)
        , _entity(nullptr)
        , _onFinishCallback(nullptr)
    {}

    EventInstanceImpl::EventInstanceImpl(const EventImpl* parent)
        : _runMode(parent->_runMode)
        , _actions(parent->_actions)
        , _running(false)
        , _runningActionIndex(0)
        , _entity(nullptr)
        , _onFinishCallback(nullptr)
    {}

    void EventInstanceImpl::AdvanceFrame(AmTime deltaTime)
    {
        if (!_running)
            return;

        if (_runMode == EventActionRunningMode_Parallel)
        {
            _running = false;

            for (auto&& action : _actions)
            {
                if (action.IsExecuting(_entity))
                {
                    _running = true;
                    action.AdvanceFrame(deltaTime);
                }
            }
        }
        else if (_runMode == EventActionRunningMode_Sequential)
        {
            if (_runningActionIndex < _actions.size())
            {
                if (!_actions[_runningActionIndex].IsExecuting(_entity))
                {
                    ++_runningActionIndex;
                    _actions[_runningActionIndex].Run(_entity);
                }
                else
                {
                    _actions[_runningActionIndex].AdvanceFrame(deltaTime);
                }
            }
            else
            {
                _running = false;
            }
        }

        if (!_running && _onFinishCallback)
            _onFinishCallback(false);
    }

    bool EventInstanceImpl::IsRunning() const
    {
        return _running;
    }

    void EventInstanceImpl::Abort()
    {
        _running = false;

        for (auto&& action : _actions)
            if (action.IsExecuting(_entity))
                action.Abort();

        if (_onFinishCallback)
            _onFinishCallback(true);
    }

    void EventInstanceImpl::OnFinish(std::function<void(bool)> callback)
    {
        _onFinishCallback = callback;
    }

    Entity EventInstanceImpl::GetEntity() const
    {
        return _entity;
    }

    void EventInstanceImpl::Start(const Entity& entity)
    {
        if (_running)
            return;

        _running = true;
        _entity = entity;

        if (_runMode == EventActionRunningMode_Parallel)
        {
            for (auto&& action : _actions)
                action.Run(_entity);
        }
        else if (_runMode == EventActionRunningMode_Sequential)
        {
            _runningActionIndex = 0;
            _actions[_runningActionIndex].Run(_entity);
        }
    }
} // namespace SparkyStudios::Audio::Amplitude
