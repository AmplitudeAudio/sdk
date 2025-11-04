// Copyright (c) 2025-present Sparky Studios. All rights reserved.
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

#if AM_PLATFORM_ANDROID

#include <SparkyStudios/Audio/Amplitude/IO/Android/AssetManagerFile.h>

namespace SparkyStudios::Audio::Amplitude
{
    AssetManagerFile::AssetManagerFile(AAsset* asset, const AmOsString& path)
        : m_asset(asset)
        , m_path(path)
    {}

    AssetManagerFile::~AssetManagerFile()
    {
        Close();
    }

    AmOsString AssetManagerFile::GetPath() const
    {
        return m_path;
    }

    bool AssetManagerFile::Eof() const
    {
        if (!IsValid())
            return true;

        const off_t totalLength = Length();
        const off_t currentPosition = Position();

        return currentPosition >= totalLength;
    }

    AmSize AssetManagerFile::Read(AmUInt8Buffer dst, AmSize bytes) const
    {
        if (!IsValid())
            return 0;

        const int32_t bytesRead = AAsset_read(m_asset, dst, bytes);

        // AAsset_read returns -1 on error, so we need to handle that case
        return bytesRead < 0 ? 0 : static_cast<AmSize>(bytesRead);
    }

    AmSize AssetManagerFile::Write(AmConstUInt8Buffer src, AmSize bytes)
    {
        return 0;
    }

    AmSize AssetManagerFile::Length() const
    {
        if (!IsValid())
            return 0;

        return AAsset_getLength(m_asset);
    }

    void AssetManagerFile::Seek(AmInt64 offset, eFileSeekOrigin origin)
    {
        if (!IsValid())
            return;

        AAsset_seek(m_asset, offset, origin);
    }

    AmSize AssetManagerFile::Position() const
    {
        if (!IsValid())
            return 0;

        return AAsset_getLength(m_asset) - AAsset_getRemainingLength(m_asset);
    }

    AmVoidPtr AssetManagerFile::GetPtr() const
    {
        return m_asset;
    }

    bool AssetManagerFile::IsValid() const
    {
        return m_asset != nullptr;
    }

    void AssetManagerFile::Close()
    {
        if (!IsValid())
            return;

        AAsset_close(m_asset);
        m_asset = nullptr;
    }
} // namespace SparkyStudios::Audio::Amplitude

#endif // AM_PLATFORM_ANDROID
