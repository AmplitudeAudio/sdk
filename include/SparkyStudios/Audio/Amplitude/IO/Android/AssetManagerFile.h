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

#pragma once

#if AM_PLATFORM_ANDROID

#ifndef _AM_IO_ANDROID_ASSET_MANAGER_FILE_H
#define _AM_IO_ANDROID_ASSET_MANAGER_FILE_H

#include <android/asset_manager.h>

#include <SparkyStudios/Audio/Amplitude/IO/File.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief A @c File implementation that reads and writes files from an Android AssetManager.
     *
     * @ingroup io
     */
    class AM_API_PUBLIC AssetManagerFile : public File
    {
    public:
        /**
         * @brief Constructs a new @c AssetManagerFile instance.
         *
         * @param[in] asset The wrapper Android asset.
         * @param[in] path The path to the file.
         */
        AssetManagerFile(AAsset* asset, const AmOsString& path);

        /**
         * @brief Destroys the @c AssetManagerFile instance and release resources.
         */
        ~AssetManagerFile() override;

        /**
         * @inherit
         */
        [[nodiscard]] AmOsString GetPath() const override;

        /**
         * @inherit
         */
        [[nodiscard]] bool Eof() const override;

        /**
         * @inherit
         */
        AmSize Read(AmUInt8Buffer dst, AmSize bytes) const override;

        /**
         * @inherit
         */
        AmSize Write(AmConstUInt8Buffer src, AmSize bytes) override;

        /**
         * @inherit
         */
        [[nodiscard]] AmSize Length() const override;

        /**
         * @inherit
         */
        void Seek(AmInt64 offset, eFileSeekOrigin origin) override;

        /**
         * @inherit
         */
        [[nodiscard]] AmSize Position() const override;

        /**
         * @inherit
         */
        [[nodiscard]] AmVoidPtr GetPtr() const override;

        /**
         * @inherit
         */
        [[nodiscard]] bool IsValid() const override;

        /**
         * @brief Closes the file.
         */
        void Close();

    private:
        AAsset* m_asset;
        AmOsString m_path;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IO_ANDROID_ASSET_MANAGER_FILE_H

#endif // AM_PLATFORM_ANDROID
