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

#if AM_PLATFORM_IOS

#ifndef _AM_IO_IOS_NS_FILE_H
#define _AM_IO_IOS_NS_FILE_H

#include <Foundation/Foundation.h>

#include <SparkyStudios/Audio/Amplitude/IO/File.h>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief A `File` implementation that reads and writes files using Apple's NSFileHandle.
     *
     * @ingroup io
     *
     * @note This class requires the iOS platform. To use it, you must include the CoreFoundation framework in your project.
     */
    class AM_API_PUBLIC NSFile : public File
    {
    public:
        /**
         * @brief Constructs a new `NSFile` instance.
         *
         * @param[in] path The path to the file.
         * @param[in] mode The mode to open the file in.
         * @param[in] kind The kind of file to open.
         */
        NSFile(const AmOsString& path, eFileOpenMode mode, eFileOpenKind kind);

        /**
         * @brief Destroys the `NSFile` instance and release resources.
         */
        ~NSFile() override;

        /**
         * @inherit
         */
        AmOsString GetPath() const override;

        /**
         * @inherit
         */
        bool Eof() override;

        /**
         * @inherit
         */
        AmSize Read(AmUInt8Buffer dst, AmSize bytes) override;

        /**
         * @inherit
         */
        AmSize Write(AmConstUInt8Buffer src, AmSize bytes) override;

        /**
         * @inherit
         */
        AmSize Length() override;

        /**
         * @inherit
         */
        void Seek(AmInt64 offset, eFileSeekOrigin origin) override;

        /**
         * @inherit
         */
        AmSize Position() override;

        /**
         * @inherit
         */
        AmVoidPtr GetPtr() override;

        /**
         * @inherit
         */
        bool IsValid() const override;

    private:
        NSFileHandle* m_fileHandle;
        AmOsString m_path;
        bool m_isValid;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IO_IOS_NS_FILE_H

#endif // AM_PLATFORM_IOS