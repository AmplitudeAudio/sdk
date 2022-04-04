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

#include <Core/EngineInternalState.h>
#include <Core/EntityInternalState.h>

#include "collection_definition_generated.h"
#include "event_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    EventAction::EventAction()
        : EventAction(nullptr)
    {}

    EventAction::EventAction(Event* parent)
        : _parent(parent)
        , _active(false)
        , _type(EventActionType_None)
        , _targets()
        , _scope(Scope_Entity)
    {}

    void EventAction::Initialize(const EventActionDefinition* definition)
    {
        _active = definition->active();
        _type = definition->type();
        _scope = definition->scope();
        flatbuffers::uoffset_t targets_count = definition->targets() ? definition->targets()->size() : 0;
        for (flatbuffers::uoffset_t i = 0; i < targets_count; ++i)
        {
            _targets.push_back(definition->targets()->Get(i));
        }
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
        }
    }

    void EventAction::Run()
    {
        if (_scope == Scope_Entity)
        {
            AMPLITUDE_ASSERT(false);
            CallLogFunc("[WARNING] Running an entity scoped event action without an entity.");
        }

        Run(Entity(nullptr));
    }

    bool EventAction::AdvanceFrame(AmTime delta_time)
    {
        // TODO
        return false;
    }

    void EventAction::ExecutePlay(const Entity& entity)
    {
        if (entity.Valid())
        {
            for (auto&& target : _targets)
            {
                (void)amEngine->Play(target, entity);
            }
        }
        else
        {
            for (auto&& target : _targets)
            {
                (void)amEngine->Play(target);
            }
        }
    }

    void EventAction::ExecutePause(const Entity& entity)
    {
        if (_scope == Scope_Entity)
        {
            for (auto&& target : _targets)
            {
                for (auto&& item : entity.GetState()->GetPlayingSoundList())
                {
                    if (target == item.GetPlayingObjectId())
                    {
                        item.Pause();
                    }
                }
            }
        }
        else
        {
            for (auto&& target : _targets)
            {
                for (auto&& item : amEngine->GetState()->playing_channel_list)
                {
                    if (target == item.GetPlayingObjectId())
                    {
                        item.Pause();
                    }
                }
            }
        }
    }

    void EventAction::ExecuteResume(const Entity& entity)
    {
        if (_scope == Scope_Entity)
        {
            for (auto&& target : _targets)
            {
                for (auto&& item : entity.GetState()->GetPlayingSoundList())
                {
                    if (target == item.GetPlayingObjectId())
                    {
                        item.Resume();
                    }
                }
            }
        }
        else
        {
            for (auto&& target : _targets)
            {
                for (auto&& item : amEngine->GetState()->playing_channel_list)
                {
                    if (target == item.GetPlayingObjectId())
                    {
                        item.Resume();
                    }
                }
            }
        }
    }

    void EventAction::ExecuteStop(const Entity& entity)
    {
        if (_scope == Scope_Entity)
        {
            for (auto&& target : _targets)
            {
                for (auto&& item : entity.GetState()->GetPlayingSoundList())
                {
                    if (target == item.GetPlayingObjectId())
                    {
                        item.Halt();
                    }
                }
            }
        }
        else
        {
            for (auto&& target : _targets)
            {
                for (auto&& item : amEngine->GetState()->playing_channel_list)
                {
                    if (target == item.GetPlayingObjectId())
                    {
                        item.Halt();
                    }
                }
            }
        }
    }

    void EventAction::ExecuteSeek(const Entity& entity)
    {}

    void EventAction::ExecuteMute(const Entity& entity, bool mute)
    {
        for (auto&& target : _targets)
        {
            Bus bus = amEngine->FindBus(target);

            if (bus.Valid())
            {
                bus.SetMute(mute);
            }
        }
    }

    Event::Event()
        : _source()
        , _id(kAmInvalidObjectId)
        , _name()
        , _actions()
        , _refCounter()
    {}

    bool Event::LoadEventDefinition(const std::string& event)
    {
        _source = event;
        const EventDefinition* definition = GetEventDefinition();

        _id = definition->id();
        _name = definition->name()->str();

        flatbuffers::uoffset_t actions_count = definition->actions() ? definition->actions()->size() : 0;
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

    bool Event::LoadEventDefinitionFromFile(AmOsString filename)
    {
        std::string source;
        return LoadFile(filename, &source) && LoadEventDefinition(source);
    }

    EventInstance Event::Trigger(const Entity& entity) const
    {
        CallLogFunc("[Debug] Event " AM_OS_CHAR_FMT " triggered.\n", AM_STRING_TO_OS_STRING(_name.c_str()));

        auto event = EventInstance(this);
        event.Start(entity);

        return event;
    }

    AmEventID Event::GetId() const
    {
        return _id;
    }

    const std::string& Event::GetName() const
    {
        return _name;
    }

    const EventDefinition* Event::GetEventDefinition() const
    {
        return Amplitude::GetEventDefinition(_source.c_str());
    }

    RefCounter* Event::GetRefCounter()
    {
        return &_refCounter;
    }

    EventCanceler::EventCanceler()
        : _event(nullptr)
    {}

    EventCanceler::EventCanceler(EventInstance* event)
        : _event(event)
    {}

    bool EventCanceler::Valid() const
    {
        return _event != nullptr;
    }

    void EventCanceler::Cancel() const
    {
        AMPLITUDE_ASSERT(Valid());
        _event->Abort();
    }

    EventInstance* EventCanceler::GetEvent() const
    {
        return _event;
    }

    EventInstance::EventInstance()
        : _actions()
        , _running(false)
    {}

    EventInstance::EventInstance(const Event* parent)
        : _actions(parent->_actions)
        , _running(false)
    {}

    void EventInstance::Start(const Entity& entity)
    {
        _running = true;
        for (auto&& action : _actions)
        {
            action.Run(entity);
        }
    }

    void EventInstance::AdvanceFrame(AmTime delta_time)
    {
        if (!_running)
            return;

        _running = false;
        for (auto&& action : _actions)
        {
            _running |= action.AdvanceFrame(delta_time);
        }
    }

    bool EventInstance::IsRunning() const
    {
        return _running;
    }

    void EventInstance::Abort()
    {
        _running = false;
    }
} // namespace SparkyStudios::Audio::Amplitude
