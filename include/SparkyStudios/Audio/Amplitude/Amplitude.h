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

#ifndef _AM_AMPLITUDE_H
#define _AM_AMPLITUDE_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

#include <SparkyStudios/Audio/Amplitude/Core/Asset.h>
#include <SparkyStudios/Audio/Amplitude/Core/Codec.h>
#include <SparkyStudios/Audio/Amplitude/Core/Device.h>
#include <SparkyStudios/Audio/Amplitude/Core/Driver.h>
#include <SparkyStudios/Audio/Amplitude/Core/Engine.h>
#include <SparkyStudios/Audio/Amplitude/Core/Entity.h>
#include <SparkyStudios/Audio/Amplitude/Core/Environment.h>
#include <SparkyStudios/Audio/Amplitude/Core/Event.h>
#include <SparkyStudios/Audio/Amplitude/Core/Listener.h>
#include <SparkyStudios/Audio/Amplitude/Core/Log.h>
#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>
#include <SparkyStudios/Audio/Amplitude/Core/Playback/Bus.h>
#include <SparkyStudios/Audio/Amplitude/Core/Playback/Channel.h>
#include <SparkyStudios/Audio/Amplitude/Core/Playback/ChannelEventListener.h>
#include <SparkyStudios/Audio/Amplitude/Core/RefCounter.h>
#include <SparkyStudios/Audio/Amplitude/Core/Thread.h>
#include <SparkyStudios/Audio/Amplitude/Core/Version.h>

#include <SparkyStudios/Audio/Amplitude/DSP/AudioConverter.h>
#include <SparkyStudios/Audio/Amplitude/DSP/Filter.h>
#include <SparkyStudios/Audio/Amplitude/DSP/Resampler.h>

#include <SparkyStudios/Audio/Amplitude/HRTF/HRIRSphere.h>

#include <SparkyStudios/Audio/Amplitude/IO/DiskFile.h>
#include <SparkyStudios/Audio/Amplitude/IO/DiskFileSystem.h>
#include <SparkyStudios/Audio/Amplitude/IO/File.h>
#include <SparkyStudios/Audio/Amplitude/IO/FileSystem.h>
#include <SparkyStudios/Audio/Amplitude/IO/MemoryFile.h>
#include <SparkyStudios/Audio/Amplitude/IO/PackageFileSystem.h>
#include <SparkyStudios/Audio/Amplitude/IO/PackageItemFile.h>
#include <SparkyStudios/Audio/Amplitude/IO/Resource.h>

#include <SparkyStudios/Audio/Amplitude/Math/BarycentricCoordinates.h>
#include <SparkyStudios/Audio/Amplitude/Math/CartesianCoordinateSystem.h>
#include <SparkyStudios/Audio/Amplitude/Math/Curve.h>
#include <SparkyStudios/Audio/Amplitude/Math/FFT.h>
#include <SparkyStudios/Audio/Amplitude/Math/Geometry.h>
#include <SparkyStudios/Audio/Amplitude/Math/HandmadeMath.h>
#include <SparkyStudios/Audio/Amplitude/Math/Orientation.h>
#include <SparkyStudios/Audio/Amplitude/Math/Shape.h>
#include <SparkyStudios/Audio/Amplitude/Math/SphericalPosition.h>
#include <SparkyStudios/Audio/Amplitude/Math/SplitComplex.h>
#include <SparkyStudios/Audio/Amplitude/Math/Utils.h>

#include <SparkyStudios/Audio/Amplitude/Mixer/Amplimix.h>
#include <SparkyStudios/Audio/Amplitude/Mixer/Node.h>
#include <SparkyStudios/Audio/Amplitude/Mixer/Pipeline.h>

#include <SparkyStudios/Audio/Amplitude/Sound/Attenuation.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Collection.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Effect.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Fader.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Rtpc.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Sound.h>
#include <SparkyStudios/Audio/Amplitude/Sound/SoundBank.h>
#include <SparkyStudios/Audio/Amplitude/Sound/SoundObject.h>
#include <SparkyStudios/Audio/Amplitude/Sound/Switch.h>
#include <SparkyStudios/Audio/Amplitude/Sound/SwitchContainer.h>

#endif // _AM_AMPLITUDE_H
