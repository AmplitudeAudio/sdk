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

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>
#include <SparkyStudios/Audio/Amplitude/IO/PackageItemFile.h>

namespace SparkyStudios::Audio::Amplitude
{
    PackageItemFile::PackageItemFile(const PackageFileItemDescription* item, const std::filesystem::path& packageFile, AmSize headerSize)
        : _description(item)
        , _headerSize(headerSize)
    {
        Open(packageFile);
        PackageItemFile::Seek(0, eFSO_START);
    }

    AmOsString PackageItemFile::GetPath() const
    {
        return _description->m_Name;
    }

    bool PackageItemFile::Eof()
    {
        return Position() >= Length();
    }

    AmSize PackageItemFile::Read(AmUInt8Buffer dst, AmSize bytes)
    {
        bytes = AM_MIN(bytes, Length() - Position());
        return bytes == 0 ? 0 : DiskFile::Read(dst, bytes);
    }

    AmSize PackageItemFile::Write(AmConstUInt8Buffer src, AmSize bytes)
    {
        // Writing is disabled for package items
        return 0;
    }

    AmSize PackageItemFile::Length()
    {
        return _description->m_Size;
    }

    void PackageItemFile::Seek(AmSize offset, FileSeekOrigin origin)
    {
        if (origin == eFSO_START && Position() == offset)
            return;

        AmSize finalOffset;

        switch (origin)
        {
        case eFSO_START:
            finalOffset = _headerSize + _description->m_Offset + AM_MIN(offset, Length());
            break;
        case eFSO_END:
            finalOffset = AM_MIN(offset, Length()) + (DiskFile::Length() - (_headerSize + _description->m_Offset + Length()));
            break;
        case eFSO_CURRENT:
            finalOffset = AM_MIN(offset, Length() - Position());
        default:
            finalOffset = offset;
            break;
        }

        DiskFile::Seek(finalOffset, origin);
    }

    AmSize PackageItemFile::Position()
    {
        return DiskFile::Position() - (_headerSize + _description->m_Offset);
    }
} // namespace SparkyStudios::Audio::Amplitude
