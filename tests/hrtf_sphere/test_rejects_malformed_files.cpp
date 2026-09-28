// Copyright (c) 2026-present Sparky Studios. All rights reserved.
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

#include <HRTF/HRIRSphere.h>

#include <filesystem>
#include <vector>

#include "ComponentTestCase.h"
#include "PlatformTestCase.h"
#include "TestRegistry.h"

using namespace SparkyStudios::Audio::Amplitude;

namespace SparkyStudios::Audio::Amplitude::Tests
{
    namespace
    {
        struct SphereFileLayout
        {
            AmUInt32 m_IRLength = 1;
            AmUInt32 m_VertexCount = 3;
            std::vector<AmUInt32> m_Indices = { 0, 1, 2 };
            AmUInt32 m_WrittenVertexCount = 3;
        };

        bool WriteSphereFile(const std::shared_ptr<FileSystem>& fs, const AmOsString& path, const SphereFileLayout& layout)
        {
            auto file = fs->OpenFile(path, eFileOpenMode_Write);
            if (file == nullptr)
                return false;

            const AmUInt8 tag[4] = { 'A', 'M', 'I', 'R' };
            file->Write(tag, 4);
            file->Write16(1); // version
            file->Write32(48000); // sample rate
            file->Write32(layout.m_IRLength);
            file->Write32(layout.m_VertexCount);
            file->Write32(static_cast<AmUInt32>(layout.m_Indices.size()));

            for (const AmUInt32 index : layout.m_Indices)
                file->Write32(index);

            const std::vector<AmReal32> ir(layout.m_IRLength, 0.5f);
            for (AmUInt32 v = 0; v < layout.m_WrittenVertexCount; ++v)
            {
                const AmVector3 position = { static_cast<AmReal32>(v), 1.0f, 0.0f };
                file->Write(reinterpret_cast<AmConstUInt8Buffer>(&position), sizeof(AmVector3));
                file->Write(reinterpret_cast<AmConstUInt8Buffer>(ir.data()), ir.size() * sizeof(AmReal32));
                file->Write(reinterpret_cast<AmConstUInt8Buffer>(ir.data()), ir.size() * sizeof(AmReal32));

                const AmReal32 delays[2] = { 0.0f, 0.0f };
                file->Write(reinterpret_cast<AmConstUInt8Buffer>(delays), sizeof(delays));
            }

            file->Close();
            return true;
        }

        bool LoadSphere(const std::shared_ptr<FileSystem>& fs, const AmOsString& path, const SphereFileLayout& layout)
        {
            if (!WriteSphereFile(fs, path, layout))
                return false;

            HRIRSphereImpl sphere;
            sphere.SetResource(path);
            sphere.Load(fs);

            return sphere.IsLoaded();
        }
    } // namespace

    AM_TEST_CASE(ComponentTestCase, hrtf_sphere, rejects_malformed_files)
    {
    public:
        void Run() override
        {
            auto fs = CreatePlatformFileSystem();
            const AmOsString path = AM_OS_STRING("./test_malformed_sphere.amir");

            // The well-formed baseline loads.
            AM_EXPECT(LoadSphere(fs, path, {}));

            SphereFileLayout outOfRangeFace;
            outOfRangeFace.m_Indices = { 0, 1, 7 };
            AM_EXPECT_NOT(LoadSphere(fs, path, outOfRangeFace));

            SphereFileLayout partialFace;
            partialFace.m_Indices = { 0, 1, 2, 0 };
            AM_EXPECT_NOT(LoadSphere(fs, path, partialFace));

            SphereFileLayout truncated;
            truncated.m_WrittenVertexCount = 2;
            AM_EXPECT_NOT(LoadSphere(fs, path, truncated));

            SphereFileLayout hugeCounts;
            hugeCounts.m_IRLength = 0x40000000u;
            hugeCounts.m_VertexCount = 0xFFFFFFFFu;
            hugeCounts.m_WrittenVertexCount = 0;
            AM_EXPECT_NOT(LoadSphere(fs, path, hugeCounts));

            std::filesystem::remove(std::filesystem::path(path));
        }
    };

    AM_REGISTER_TEST_DESKTOP_ONLY(hrtf_sphere, rejects_malformed_files);
} // namespace SparkyStudios::Audio::Amplitude::Tests
