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
        PackageItemFile::Seek(0, eFileSeekOrigin_Start);
    }

    AmOsString PackageItemFile::GetPath() const
    {
        return AM_STRING_TO_OS_STRING(_description->m_Name);
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

    void PackageItemFile::Seek(AmInt64 offset, eFileSeekOrigin origin)
    {
        const AmSize currentPosition = Position();
        const AmSize fileSize = Length();

        const AmInt64 minOffset = _headerSize + _description->m_Offset;
        const AmInt64 maxOffset = minOffset + fileSize;

        if (origin == eFileSeekOrigin_Start && currentPosition == offset)
            return;

        if (origin == eFileSeekOrigin_End && currentPosition == (fileSize + offset))
            return;

        if (origin == eFileSeekOrigin_Current && offset == 0)
            return;

        AmInt64 finalOffset = minOffset;

        switch (origin)
        {
        case eFileSeekOrigin_Start:
            finalOffset += offset;
            break;
        case eFileSeekOrigin_End:
            finalOffset += fileSize - 1 + offset;
            break;
        case eFileSeekOrigin_Current:
            finalOffset += currentPosition + offset;
            break;
        }

        finalOffset = AM_CLAMP(finalOffset, minOffset, maxOffset);

        DiskFile::Seek(finalOffset, eFileSeekOrigin_Start);
    }

    AmSize PackageItemFile::Position()
    {
        return DiskFile::Position() - (_headerSize + _description->m_Offset);
    }
} // namespace SparkyStudios::Audio::Amplitude
