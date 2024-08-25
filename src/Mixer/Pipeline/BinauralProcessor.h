// Copyright (c) 2024-present Sparky Studios. All rights reserved.
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

#ifndef SS_AMPLITUDE_AUDIO_BINAURAL_PROCESSOR_H
#define SS_AMPLITUDE_AUDIO_BINAURAL_PROCESSOR_H

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Core/Playback/ChannelInternalState.h>
#include <HRTF/HRIRSphere.h>
#include <Math/FaceBSPTree.h>
#include <Mixer/RealChannel.h>
#include <Utils/Utils.h>

#include "sound_definition_generated.h"

namespace SparkyStudios::Audio::Amplitude
{
    constexpr AmUInt32 kInterpolationSteps = 16;
    constexpr AmUInt32 kInterpolationBlockSize = 128;

    struct HRTFSphereVertex
    {
        AmVec3 m_Position;
        SplitComplex m_LeftHRTF;
        SplitComplex m_RightHRTF;
    };

    class HRTFSphere
    {
    public:
        HRTFSphere() = default;

        ~HRTFSphere()
        {
            for (auto* vertex : _vertices)
            {
                ampooldelete(MemoryPoolKind::Filtering, HRTFSphereVertex, vertex);
            }
        }

        void Init(const HRIRSphere* hrir, AmSize blockLength)
        {
            const AmSize padLength = hrir->GetIRLength() + blockLength - 1;
            const AmSize fftSize = NextPowerOf2(padLength);

            const FFT fft;
            fft.Initialize(fftSize);
            _length = hrir->GetIRLength();

            std::vector<AmVec3> vertices(hrir->GetVertexCount());
            std::ranges::transform(
                hrir->GetVertices(), vertices.begin(),
                [](const HRIRSphereVertex& v)
                {
                    return v.m_Position;
                });

            _vertices.reserve(hrir->GetVertexCount());
            for (AmSize i = 0, l = hrir->GetVertexCount(); i < l; ++i)
            {
                auto* vertex = ampoolnew(MemoryPoolKind::Filtering, HRTFSphereVertex);
                const auto& v = hrir->GetVertex(i);

                vertex->m_Position = v.m_Position;
                MakeHRTF(fft, v.m_LeftIR, padLength, vertex->m_LeftHRTF);
                MakeHRTF(fft, v.m_RightIR, padLength, vertex->m_RightHRTF);

                _vertices.push_back(vertex);
            }

            _tree.Build(vertices, hrir->GetFaces());
        }

        /**
         * @brief Samples the HRIR sphere for the given direction using bilinear interpolation.
         * See more info here http://www02.smt.ufrj.br/~diniz/conf/confi117.pdf.
         *
         * @param direction The sound to listener direction.
         * @param leftHRTF The left HRIR data.
         * @param rightHRTF The right HRIR data.
         */
        void SampleBilinear(const AmVec3& direction, SplitComplex& leftHRTF, SplitComplex& rightHRTF) const
        {
            const auto& dir = AM_Mul(direction, 10.0f);
            const auto* face = _tree.Query(dir);

            if (face == nullptr)
                return;

            const auto& vertexA = _vertices[face->m_A];
            const auto& vertexB = _vertices[face->m_B];
            const auto& vertexC = _vertices[face->m_C];

            BarycentricCoordinates barycenter;
            if (!BarycentricCoordinates::RayTriangleIntersection(
                    AM_V3(0.0f, 0.0f, 0.0f), dir, { vertexA->m_Position, vertexB->m_Position, vertexC->m_Position }, barycenter))
            {
                return;
            }

            const AmSize length = vertexA->m_LeftHRTF.GetSize();

            leftHRTF.Resize(length);
            rightHRTF.Resize(length);

            for (AmSize i = 0; i < length; ++i)
            {
                auto l = vertexA->m_LeftHRTF[i] * barycenter.m_U + vertexB->m_LeftHRTF[i] * barycenter.m_V +
                    vertexC->m_LeftHRTF[i] * barycenter.m_W;
                auto r = vertexA->m_RightHRTF[i] * barycenter.m_U + vertexB->m_RightHRTF[i] * barycenter.m_V +
                    vertexC->m_RightHRTF[i] * barycenter.m_W;

                leftHRTF.re()[i] = l.real();
                leftHRTF.im()[i] = l.imag();

                rightHRTF.re()[i] = r.real();
                rightHRTF.im()[i] = r.imag();
            }
        }

        [[nodiscard]] AmSize GetLength() const
        {
            return _length;
        }

        [[nodiscard]] const HRTFSphereVertex& GetVertex(const AmSize index) const
        {
            return *_vertices[index];
        }

    private:
        static void MakeHRTF(const FFT& fft, const std::vector<AmReal32>& ir, const AmSize padLength, SplitComplex& hrtf)
        {
            AmAlignedReal32Buffer scratch;
            scratch.Resize(NextPowerOf2(padLength));
            CopyAndPad(scratch, ir.data(), ir.size());
            fft.Forward(scratch.GetBuffer(), hrtf);
        }

        AmSize _length = 0;
        std::vector<HRTFSphereVertex*> _vertices;
        FaceBSPTree _tree;
    };

    class BinauralProcessorInstance final : public SoundProcessorInstance
    {
    public:
        explicit BinauralProcessorInstance(HRIRSphere* hrir)
            : SoundProcessorInstance()
            , _hrir(hrir)
        {
            _padLength = kInterpolationBlockSize + _hrir->GetIRLength() - 1;

            _hrtf.Init(_hrir, kInterpolationBlockSize);
            _fft.Initialize(NextPowerOf2(_padLength));
        }

        void Process(const AmplimixLayer* layer, const AudioBuffer& in, AudioBuffer& out) override
        {
            // TODO
            AmSize channels = in.GetChannelCount();
            auto* channel = layer->GetChannel().GetState();

            if (layer->GetSpatialization() == eSpatialization_HRTF)
            {
                AmUInt64 frames = in.GetFrameCount();
                AudioBuffer temp(frames, 1);

                // HRTF only works with mono
                {
                    AudioConverter converter;
                    converter.Configure({ 0, 0, static_cast<AmUInt16>(channels), 1 });
                    converter.Process(in, frames, temp, frames);
                }

                // Apply the HRTF
                auto segCount =
                    static_cast<AmSize>(std::ceil(static_cast<AmReal32>(frames) / static_cast<AmReal32>(kInterpolationBlockSize)));
                auto& context = channel->GetHRTFContext();

                const auto& hrtfVertex = _hrtf.GetVertex(0);
                SplitComplex leftHRTF(hrtfVertex.m_LeftHRTF.GetSize()), rightHRTF(hrtfVertex.m_RightHRTF.GetSize());
                leftHRTF.CopyFrom(hrtfVertex.m_LeftHRTF);
                rightHRTF.CopyFrom(hrtfVertex.m_RightHRTF);

                AmAlignedReal32Buffer inL, inR;
                inL.Resize(_padLength);
                inR.Resize(_padLength);

                SplitComplex scratch;

                const AmSize hrtfLength = _hrtf.GetLength() - 1;

                AudioBuffer outBuffer(frames + hrtfLength, 2);
                auto& outL = outBuffer[0];
                auto& outR = outBuffer[1];

                for (AmSize i = 0; i < segCount; ++i)
                {
                    const AmSize offset = i * kInterpolationBlockSize;
                    const AmSize next = i + 1;
                    const AmReal32 t = static_cast<AmReal32>(next) / static_cast<AmReal32>(segCount);

                    AmVec3 dir = AM_Lerp(context.m_PreviousDirection, t, context.m_CurrentDirection);
                    _hrtf.SampleBilinear(dir, leftHRTF, rightHRTF);

                    GetSource(
                        reinterpret_cast<AmConstAudioSampleBuffer>(temp.GetData().GetBuffer()) + offset, inL.GetBuffer() + hrtfLength,
                        inR.GetBuffer() + hrtfLength, kInterpolationBlockSize);

                    ConvolveOverlapSave(inL, scratch, leftHRTF, hrtfLength, context.m_PreviousSamplesL);
                    ConvolveOverlapSave(inR, scratch, rightHRTF, hrtfLength, context.m_PreviousSamplesR);

                    const AmReal32 gain = AM_Lerp(context.m_PreviousGain, t, context.m_CurrentGain);
                    const AmReal32 k = 1; // gain / static_cast<AmReal32>(_padLength);

                    for (AmSize j = 0; j < kInterpolationBlockSize; ++j)
                    {
                        outL[offset + j] += inL[hrtfLength + j] * k;
                        outR[offset + j] += inR[hrtfLength + j] * k;
                    }
                }

                // context.m_PreviousDirection = context.m_CurrentDirection;

                // HRTFInstance* mit = HRTF::Construct("mit-hrtf");
                // mit->Init(sampleRate);
                //
                // PolarPoint position(
                //     channelState->GetListener().GetLocation(), channelState->GetListener().GetDirection(), AM_V3(0.0f, 1.0f, 0.0f),
                //     channelState->GetLocation());
                // PolarPoint position(AM_V3(0.0f, 0.0f, 0.0), channelState->GetListener().GetDirection(), AM_V3(0.0f, 1.0f, 0.0f),
                // context.m_CurrentDirection);
                // mit->Process(position.m_Azimuth, position.m_Elevation, in, outL.GetBuffer(), outR.GetBuffer(), frames);
                //
                // HRTF::Destruct("mit-hrtf", mit);

                out[0] = outL;
                out[1] = outR;
                // std::memcpy(out[0].begin(), outL.GetBuffer(), frames * sizeof(AmReal32));
                // std::memcpy(out[1].begin(), outR.GetBuffer(), frames * sizeof(AmReal32));
            }

            else if (&out != &in)
                AudioBuffer::Copy(in, 0, out, 0, in.GetFrameCount());
        }

    private:
        static void GetSource(AmConstAudioSampleBuffer in, AmAudioSampleBuffer l, AmAudioSampleBuffer r, AmSize size)
        {
            for (AmSize i = 0; i < size; ++i)
            {
                l[i] = in[i] * AM_InvSqrtF(2);
                r[i] = in[i] * AM_InvSqrtF(2);
            }
        }

        void ConvolveOverlapSave(
            AmAlignedReal32Buffer& in, SplitComplex& scratch, const SplitComplex& hrtf, AmSize hrtfLength, AmAlignedReal32Buffer& prev)
        {
            // AMPLITUDE_ASSERT(hrtf.GetSize() == in.GetSize());

            CopyReplace(prev, in, hrtfLength);

            AmAlignedReal32Buffer temp;
            temp.Resize(NextPowerOf2(in.GetSize()));
            CopyAndPad(temp, in.GetBuffer(), in.GetSize());

            SplitComplex result;
            result.Resize(hrtf.GetSize(), true);

            // Forward FFT
            _fft.Forward(temp.GetBuffer(), scratch);

            // Multiply HRIR and input signal in frequency domain.
            ComplexMultiplyAccumulate(result, scratch, hrtf);

            // Backward FFT
            _fft.Backward(temp.GetBuffer(), result);

            std::memcpy(in.GetBuffer(), temp.GetBuffer(), in.GetSize() * sizeof(AmReal32));
        }

        static void CopyReplace(AmAlignedReal32Buffer& prev_samples, AmAlignedReal32Buffer& raw_buffer, AmSize segment_len)
        {
            if (prev_samples.GetSize() != segment_len)
            {
                prev_samples.Resize(segment_len, true);
            }

            // Copy samples from previous iteration in the beginning of the buffer.
            for (AmSize i = 0; i < segment_len; ++i)
            {
                raw_buffer[i] = prev_samples[i];
            }

            // Replace last samples by samples from end of the buffer for next iteration.
            const AmSize last_start = raw_buffer.GetSize() - segment_len;
            for (AmSize i = 0, l = segment_len; i < l; ++i)
            {
                prev_samples[i] = raw_buffer[last_start + i];
            }
        }

        HRIRSphere* _hrir;
        HRTFSphere _hrtf;

        AmSize _padLength = 0;
        FFT _fft;
    };

    class BinauralProcessor final : public SoundProcessor
    {
    public:
        BinauralProcessor()
            : SoundProcessor("BinauralProcessor")
        {}

        SoundProcessorInstance* CreateInstance() override
        {
            if (!_hrirSphere.IsLoaded())
            {
                _hrirSphere.SetResource("./data/mit.amir");
                _hrirSphere.Load(Engine::GetInstance()->GetFileSystem());
            }

            return ampoolnew(MemoryPoolKind::Amplimix, BinauralProcessorInstance, &_hrirSphere);
        }

        void DestroyInstance(SoundProcessorInstance* instance) override
        {
            ampooldelete(MemoryPoolKind::Amplimix, BinauralProcessorInstance, (BinauralProcessorInstance*)instance);
        }

    private:
        HRIRSphereImpl _hrirSphere;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif
