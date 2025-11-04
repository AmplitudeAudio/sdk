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

#include <SparkyStudios/Audio/Amplitude/Core/Memory.h>
#include <SparkyStudios/Audio/Amplitude/IO/iOS/NSFile.h>
#include <SparkyStudios/Audio/Amplitude/IO/iOS/NSFileSystem.h>

namespace SparkyStudios::Audio::Amplitude {

NSFileSystem::NSFileSystem() : m_basePath(), m_fileManager(nil) {
  m_fileManager = [NSFileManager defaultManager];
}

NSFileSystem::~NSFileSystem() {
  // NSFileManager is a singleton, no need to release
  m_fileManager = nil;
}

void NSFileSystem::SetBasePath(const AmOsString &basePath) {
  if (basePath.empty()) {
    m_basePath.clear();
    return;
  }

  @autoreleasepool {
    NSString *nsBasePath = [NSString stringWithUTF8String:basePath.c_str()];
    if (nsBasePath) {
      // Normalize the path and ensure it's properly formatted
      NSString *standardizedPath = [nsBasePath stringByStandardizingPath];
      m_basePath = [standardizedPath UTF8String];
    } else {
      m_basePath.clear();
    }
  }
}

const AmOsString &NSFileSystem::GetBasePath() const { return m_basePath; }

AmOsString NSFileSystem::ResolvePath(const AmOsString &path) const {
  if (path.empty()) {
    return m_basePath;
  }

  @autoreleasepool {
    NSString *nsPath = [NSString stringWithUTF8String:path.c_str()];
    if (!nsPath) {
      return AmOsString();
    }

    // If path is absolute, return it as-is
    if ([nsPath isAbsolutePath]) {
      NSString *standardizedPath = [nsPath stringByStandardizingPath];
      return [standardizedPath UTF8String];
    }

    // If no base path is set, return the relative path
    if (m_basePath.empty()) {
      NSString *standardizedPath = [nsPath stringByStandardizingPath];
      return [standardizedPath UTF8String];
    }

    // Combine base path with relative path
    NSString *nsBasePath = [NSString stringWithUTF8String:m_basePath.c_str()];
    if (!nsBasePath) {
      return AmOsString();
    }

    NSString *resolvedPath = [nsBasePath stringByAppendingPathComponent:nsPath];
    NSString *standardizedPath = [resolvedPath stringByStandardizingPath];

    return [standardizedPath UTF8String];
  }
}

bool NSFileSystem::Exists(const AmOsString &path) const {
  if (path.empty()) {
    return false;
  }

  @autoreleasepool {
    AmOsString resolvedPath = ResolvePath(path);
    if (resolvedPath.empty()) {
      return false;
    }

    NSString *nsPath = [NSString stringWithUTF8String:resolvedPath.c_str()];
    if (!nsPath) {
      return false;
    }

    return [m_fileManager fileExistsAtPath:nsPath];
  }
}

bool NSFileSystem::IsDirectory(const AmOsString &path) const {
  if (path.empty()) {
    return false;
  }

  @autoreleasepool {
    AmOsString resolvedPath = ResolvePath(path);
    if (resolvedPath.empty()) {
      return false;
    }

    NSString *nsPath = [NSString stringWithUTF8String:resolvedPath.c_str()];
    if (!nsPath) {
      return false;
    }

    BOOL isDirectory = NO;
    BOOL exists =
        [m_fileManager fileExistsAtPath:nsPath isDirectory:&isDirectory];
    return exists && isDirectory;
  }
}

AmOsString NSFileSystem::Join(const std::vector<AmOsString> &parts) const {
  if (parts.empty()) {
    return AmOsString();
  }

  @autoreleasepool {
    NSString *result = nil;

    for (const auto &part : parts) {
      if (part.empty()) {
        continue; // Skip empty parts
      }

      NSString *nsPart = [NSString stringWithUTF8String:part.c_str()];
      if (!nsPart) {
        continue; // Skip invalid UTF-8 strings
      }

      if (result == nil) {
        result = nsPart;
      } else {
        result = [result stringByAppendingPathComponent:nsPart];
      }
    }

    if (result == nil) {
      return AmOsString();
    }

    // Standardize the final path
    NSString *standardizedPath = [result stringByStandardizingPath];
    return [standardizedPath UTF8String];
  }
}

std::shared_ptr<File> NSFileSystem::OpenFile(const AmOsString &path,
                                             eFileOpenMode mode) const {
  if (path.empty()) {
    return nullptr;
  }

  @autoreleasepool {
    AmOsString resolvedPath = ResolvePath(path);
    if (resolvedPath.empty()) {
      return nullptr;
    }

    // Create the NSFile with resolved path
    auto file = ampoolshared(eMemoryPoolKind_IO, NSFile, resolvedPath, mode, eFileOpenKind_Binary);

    // Verify the file was opened successfully
    if (file && !file->IsValid()) {
      return nullptr;
    }

    return file;
  }
}

void NSFileSystem::StartOpenFileSystem() {
  // No specific initialization needed for iOS file system
  // NSFileManager is available immediately
}

bool NSFileSystem::TryFinalizeOpenFileSystem() {
  // Always return true as there's no asynchronous initialization
  // NSFileManager is ready to use immediately
  return true;
}

void NSFileSystem::StartCloseFileSystem() {
  // No specific cleanup needed for iOS file system
  // NSFileManager is managed by the system
}

bool NSFileSystem::TryFinalizeCloseFileSystem() {
  // Always return true as there's no asynchronous cleanup
  // NSFileManager cleanup is handled by the system
  return true;
}

} // namespace SparkyStudios::Audio::Amplitude
