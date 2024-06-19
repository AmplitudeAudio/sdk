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

#include <catch2/catch_test_macros.hpp>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

#include <Core/EntityInternalState.h>

using namespace SparkyStudios::Audio::Amplitude;

TEST_CASE("AlignedReal32Buffer Tests", "[common][core][amplitude]")
{
    GIVEN("an empty aligned real32 buffer")
    {
        AmAlignedReal32Buffer buffer;
        buffer.Init(1024);

        THEN("its pointer is valid")
        {
            REQUIRE(buffer.GetBuffer() != nullptr);
            REQUIRE(buffer.GetPointer() != nullptr);
        }

        THEN("it returns the correct size")
        {
            REQUIRE(buffer.GetSize() == 1024);
        }

        THEN("it can be filled")
        {
            for (AmUInt32 i = 0; i < 1024; ++i)
                buffer[i] = 1.0f;

            AND_THEN("filled values are properly stored")
            {
                for (AmUInt32 i = 0; i < 1024; ++i)
                    REQUIRE(buffer[i] == 1.0f);
            }

            AND_THEN("it can be cleared")
            {
                buffer.Clear();

                for (AmUInt32 i = 0; i < 1024; ++i)
                    REQUIRE(buffer[i] == 0.0f);
            }
        }

        THEN("it can be resized")
        {
            buffer.Resize(2048);

            AND_THEN("its pointer is valid")
            {
                REQUIRE(buffer.GetBuffer() != nullptr);
                REQUIRE(buffer.GetPointer() != nullptr);
            }

            AND_THEN("it returns the correct size")
            {
                REQUIRE(buffer.GetSize() == 2048);
            }
        }

        THEN("it can be copied")
        {
            AmAlignedReal32Buffer buffer2;
            buffer2.Init(1024);

            for (AmUInt32 i = 0; i < 1024; ++i)
                buffer2[i] = 1.0f;

            buffer.CopyFrom(buffer2);

            AND_THEN("copied values are properly stored")
            {
                for (AmUInt32 i = 0; i < 1024; ++i)
                    REQUIRE(buffer[i] == 1.0f);
            }
        }

        THEN("it can be swapped")
        {
            AmAlignedReal32Buffer buffer2;
            buffer2.Init(1024);

            for (AmUInt32 i = 0; i < 1024; ++i)
            {
                buffer[i] = 1.0f;
                buffer2[i] = 2.0f;
            }

            AmAlignedReal32Buffer::Swap(buffer, buffer2);

            AND_THEN("swapped values are properly stored")
            {
                for (AmUInt32 i = 0; i < 1024; ++i)
                {
                    REQUIRE(buffer[i] == 2.0f);
                    REQUIRE(buffer2[i] == 1.0f);
                }
            }
        }

        THEN("it can be released")
        {
            buffer.Release();

            REQUIRE(buffer.GetBuffer() == nullptr);
            REQUIRE(buffer.GetPointer() == nullptr);
            REQUIRE(buffer.GetSize() == 0);
        }
    }
}