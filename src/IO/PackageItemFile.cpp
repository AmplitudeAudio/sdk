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

#include <lz4.h>

namespace SparkyStudios::Audio::Amplitude
{
    PackageItemFile::PackageItemFile(const PackageFileItemDescription* item, std::shared_ptr<File> packageFile, AmSize headerSize)
        : _description(item)
        , _isCompressed(item->m_CompressedBlockSize > 0)
        , _headerSize(headerSize)
        , _packageFile(packageFile)
    {
        Seek(0, eFileSeekOrigin_Start);
    }

    PackageItemFile::~PackageItemFile()
    {
        _packageFile.reset();
    }

    AmOsString PackageItemFile::GetPath() const
    {
        return AM_STRING_TO_OS_STRING(_description->m_Name);
    }

    bool PackageItemFile::Eof() const
    {
        return Position() >= Length();
    }

    AmSize PackageItemFile::Read(AmUInt8Buffer dst, AmSize bytes) const
    {
        const AmSize remainingBytes = _description->m_Size - _currentPosition;
        bytes = AM_MIN(bytes, remainingBytes);

        if (bytes == 0)
            return 0;

        if (_isCompressed)
        {
            AmSize readSize = 0;

            const AmSize firstChunk = _currentPosition / _description->m_CompressedBlockSize;
            const AmSize lastChunk = (_currentPosition + bytes - 1) / _description->m_CompressedBlockSize;

            AmSize remaining = bytes;
            for (AmSize ci = firstChunk; ci <= lastChunk; ++ci)
            {
                const auto& ch = _description->m_CompressedChunks[ci];
                std::vector<AmUInt8> compressed(ch.m_CompressedSize);

                _packageFile->Seek(GetBasePosition() + ch.m_Offset, eFileSeekOrigin_Start);
                _packageFile->Read(compressed.data(), ch.m_CompressedSize);

                std::vector<char> decompressed(ch.m_Size);
                LZ4_decompress_safe(reinterpret_cast<char*>(compressed.data()), decompressed.data(), ch.m_CompressedSize, ch.m_Size);

                AmSize chunkStartOffset = (ci == firstChunk) ? _currentPosition % _description->m_CompressedBlockSize : 0;
                AmSize copyLen = std::min(ch.m_Size - chunkStartOffset, remaining);

                std::memcpy(dst, decompressed.data() + chunkStartOffset, copyLen);

                remaining -= copyLen;
            }

            _currentPosition += bytes - remaining;
            return bytes;
        }

        _packageFile->Seek(GetBasePosition() + _currentPosition, eFileSeekOrigin_Start);

        const auto distance = Length() - Position();
        bytes = AM_MIN(bytes, distance);
        const AmSize readBytes = bytes == 0 ? 0 : _packageFile->Read(dst, bytes);

        _currentPosition += readBytes;
        return readBytes;
    }

    AmSize PackageItemFile::Write(AmConstUInt8Buffer src, AmSize bytes)
    {
        // Writing is disabled for package items
        return 0;
    }

    AmSize PackageItemFile::Length() const
    {
        return _description->m_Size;
    }

    void PackageItemFile::Seek(AmInt64 offset, eFileSeekOrigin origin)
    {
        const AmSize currentPosition = Position();
        const AmSize fileSize = Length();

        if (origin == eFileSeekOrigin_Start && currentPosition == offset)
            return;

        if (origin == eFileSeekOrigin_End && currentPosition == (fileSize + offset))
            return;

        if (origin == eFileSeekOrigin_Current && offset == 0)
            return;

        AmInt64 finalOffset = 0;

        switch (origin)
        {
        case eFileSeekOrigin_Start:
            finalOffset = offset;
            break;
        case eFileSeekOrigin_End:
            finalOffset = fileSize + offset;
            break;
        case eFileSeekOrigin_Current:
            finalOffset = currentPosition + offset;
            break;
        }

        _currentPosition = AM_CLAMP(finalOffset, 0, fileSize);
    }

    AmSize PackageItemFile::Position() const
    {
        return _currentPosition;
    }

    AmVoidPtr PackageItemFile::GetPtr() const
    {
        return nullptr;
    }

    bool PackageItemFile::IsValid() const
    {
        return _packageFile != nullptr && _packageFile->IsValid();
    }

    void PackageItemFile::Close()
    {
        if (_packageFile == nullptr)
            return;

        _packageFile->Close();
        _packageFile.reset();
    }

    AmSize PackageItemFile::GetBasePosition() const
    {
        return _headerSize + _description->m_Offset;
    }
} // namespace SparkyStudios::Audio::Amplitude
