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

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>

#include <Sound/Filters/FreeverbFilter.h>

namespace SparkyStudios::Audio::Amplitude
{
    FreeverbFilter::FreeverbFilter()
        : Filter("Freeverb")
        , _roomSize(0.5f)
        , _damp(0.5f)
        , _width(1.0f)
        , _mode(0.0f)
    {}

    AmResult FreeverbFilter::Init(AmReal32 roomSize, AmReal32 damp, AmReal32 width, AmReal32 mode)
    {
        if (mode < 0 || mode > 1 || roomSize <= 0 || damp < 0 || width <= 0)
            return AM_ERROR_INVALID_PARAMETER;

        _roomSize = roomSize;
        _damp = damp;
        _width = width;
        _mode = mode;

        return AM_ERROR_NO_ERROR;
    }

    AmUInt32 FreeverbFilter::GetParamCount()
    {
        return ATTRIBUTE_LAST;
    }

    AmString FreeverbFilter::GetParamName(AmUInt32 index)
    {
        switch (index)
        {
        case ATTRIBUTE_WET:
            return "Wet";
        case ATTRIBUTE_DRY:
            return "Dry";
        case ATTRIBUTE_ROOM_SIZE:
            return "Room Size";
        case ATTRIBUTE_DAMP:
            return "Damp";
        case ATTRIBUTE_WIDTH:
            return "Width";
        case ATTRIBUTE_MODE:
            return "Freeze";
        default:
            return "";
        }
    }

    AmUInt32 FreeverbFilter::GetParamType(AmUInt32 index)
    {
        if (index >= ATTRIBUTE_MODE)
            return PARAM_BOOL;

        return PARAM_FLOAT;
    }

    AmReal32 FreeverbFilter::GetParamMax(AmUInt32 index)
    {
        return 1.0f;
    }

    AmReal32 FreeverbFilter::GetParamMin(AmUInt32 index)
    {
        return 0.0f;
    }

    FilterInstance* FreeverbFilter::CreateInstance()
    {
        return ampoolnew(MemoryPoolKind::Filtering, FreeverbFilterInstance, this);
    }

    void FreeverbFilter::DestroyInstance(FilterInstance* instance)
    {
        ampooldelete(MemoryPoolKind::Filtering, FreeverbFilterInstance, (FreeverbFilterInstance*)instance);
    }

    FreeverbFilterInstance::FreeverbFilterInstance(FreeverbFilter* parent)
        : FilterInstance(parent)
    {
        Init(parent->GetParamCount());

        _model = new Freeverb::ReverbModel();

        m_parameters[FreeverbFilter::ATTRIBUTE_WET] = 1.0f;
        m_parameters[FreeverbFilter::ATTRIBUTE_DRY] = 0.0f;
        m_parameters[FreeverbFilter::ATTRIBUTE_ROOM_SIZE] = parent->_roomSize;
        m_parameters[FreeverbFilter::ATTRIBUTE_DAMP] = parent->_damp;
        m_parameters[FreeverbFilter::ATTRIBUTE_WIDTH] = parent->_width;
        m_parameters[FreeverbFilter::ATTRIBUTE_MODE] = parent->_mode;
    }

    FreeverbFilterInstance::~FreeverbFilterInstance()
    {
        delete _model;
    }

    void FreeverbFilterInstance::Process(
        AmAudioSampleBuffer buffer, AmUInt64 frames, AmUInt64 bufferSize, AmUInt16 channels, AmUInt32 sampleRate)
    {
        if (m_numParamsChanged > 0)
        {
            _model->SetDamp(m_parameters[FreeverbFilter::ATTRIBUTE_DAMP]);
            _model->SetMode(m_parameters[FreeverbFilter::ATTRIBUTE_MODE]);
            _model->SetRoomSize(m_parameters[FreeverbFilter::ATTRIBUTE_ROOM_SIZE]);
            _model->SetWidth(m_parameters[FreeverbFilter::ATTRIBUTE_WIDTH]);
            _model->SetWet(m_parameters[FreeverbFilter::ATTRIBUTE_WET]);
            _model->SetDry(m_parameters[FreeverbFilter::ATTRIBUTE_DRY]);
            m_numParamsChanged = 0;
        }

        auto* input = buffer;
        auto* output = buffer;

        _model->ProcessReplace(input, input + (channels - 1), output, output + (channels - 1), frames, channels);
    }
} // namespace SparkyStudios::Audio::Amplitude
