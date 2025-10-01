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

#ifndef _AM_IO_FILESYSTEM_H
#define _AM_IO_FILESYSTEM_H

#include <SparkyStudios/Audio/Amplitude/IO/File.h>

#include <memory>
#include <vector>

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief Base class for files and resources loaders.
     *
     * The engine uses the @c FileSystem class as an interface to
     * load files and other resources. It provides basic functionalities
     * needed by a file system.
     *
     * You are able to implement your own @c FileSystem subclass to fit your needs, for example,
     * reading files from a network drive or a custom storage system.
     *
     * @ingroup io
     */
    class AM_API_PUBLIC FileSystem
    {
    public:
        /**
         * @brief Default destructor.
         */
        virtual ~FileSystem() = default;

        /**
         * @brief Changes the file system base path.
         *
         * That path is interpreted by the implementation and doesn't necessarily have to be a real
         * path on disk. It's just used as the base path for resolving relative paths from which the
         * engine will load resources.
         *
         * @param[in] basePath The file system base path.
         */
        virtual void SetBasePath(const AmOsString& basePath) = 0;

        /**
         * @brief Returns the base path of the file system.
         *
         * @return The base path for resolving relative paths from which the engine will load resources.
         */
        [[nodiscard]] virtual const AmOsString& GetBasePath() const = 0;

        /**
         * @brief Resolves a relative path from the file system base path.
         *
         * @return The resolved path.
         */
        [[nodiscard]] virtual AmOsString ResolvePath(const AmOsString& path) const = 0;

        /**
         * @brief Checks if an item (file or folder) exists on the file system.
         *
         * @param[in] path The path to the item.
         *
         * @return @c true if the file exists, @c false otherwise.
         */
        [[nodiscard]] virtual bool Exists(const AmOsString& path) const = 0;

        /**
         * @brief Checks if an item (file or folder) is a directory.
         *
         * @param path The path to the item.
         *
         * @return @c true if the file is a directory, @c false otherwise.
         */
        [[nodiscard]] virtual bool IsDirectory(const AmOsString& path) const = 0;

        /**
         * @brief Merge the given parts of the path into a single path by joining them with the file system's path separator.
         *
         * @param[in] parts The parts of the path.
         *
         * @return A path concatenated with the given parts and the file system path separator.
         */
        [[nodiscard]] virtual AmOsString Join(const std::vector<AmOsString>& parts) const = 0;

        /**
         * @brief Opens the file at the given path.
         *
         * @param[in] path The path to the file to open.
         * @param[in] mode The file open mode.
         *
         * @return The opened file. The returned @c File implementation depends on the @c FileSystem implementation.
         */
        [[nodiscard]] virtual std::shared_ptr<File> OpenFile(const AmOsString& path, eFileOpenMode mode = eFileOpenMode_Read) const = 0;

        /**
         * @brief Opens the @c FileSystem.
         *
         * This function __MUST__ be called before any other actions in the file system.
         * It is used to initialize the file system (e.g.: mounting an archive).
         *
         * @note
         * - For implementations, It is recommended to process the initialization in a separate thread.         *
         * - The implementation is free to ignore this if not needed.
         */
        virtual void StartOpenFileSystem() = 0;

        /**
         * @brief Checks if the @c FileSystem is loaded.
         *
         * Since the @ref StartOpenFileSystem "`StartOpenFileSystem()`" function is designed to be asynchronous, this function
         * is used to check if the @c FileSystem has been successfully initialized.
         *
         * @return @c true if the @c FileSystem has been fully loaded, @c false otherwise.
         */
        virtual bool TryFinalizeOpenFileSystem() = 0;

        /**
         * @brief Stops the @c FileSystem.
         *
         * This function __MUST__ be called when the @c FileSystem is no longer needed.
         * It is used to stop the file system (e.g.: unmounting an archive).
         *
         * @note
         * - For implementations, it is recommended to process the stopping in a separate thread.
         * - The implementation is free to ignore this if not needed.
         */
        virtual void StartCloseFileSystem() = 0;

        /**
         * @brief Checks if the @c FileSystem is stopped.
         *
         * Since the @ref StartCloseFileSystem "`StartCloseFileSystem()`" function is designed to be asynchronous, this function
         * is used to check if the @c FileSystem has been successfully stopped.
         *
         * @return @c true if the @c FileSystem has been fully stopped, @c false otherwise.
         */
        virtual bool TryFinalizeCloseFileSystem() = 0;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IO_FILESYSTEM_H
