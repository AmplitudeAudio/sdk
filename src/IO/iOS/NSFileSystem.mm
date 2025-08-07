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

NSFileSystem::NSFileSystem() : m_basePath(nil), m_fileManager(nil) {
  m_fileManager = [NSFileManager defaultManager];
}

NSFileSystem::~NSFileSystem() {  }

void NSFileSystem::SetBasePath(const AmOsString &basePath) {
  m_basePath = basePath;
}

const AmOsString &NSFileSystem::GetBasePath() const {
  return m_basePath;
}

AmOsString NSFileSystem::ResolvePath(const AmOsString &path) const {
  NSString *basePath = [[NSString alloc] initWithUTF8String:m_basePath.c_str()];
  NSString *resolvedPath = [basePath
      stringByAppendingPathComponent:[NSString
                                         stringWithUTF8String:path.c_str()]];
  return [resolvedPath UTF8String];
}

bool NSFileSystem::Exists(const AmOsString &path) const {
  NSString *nsPath = [NSString stringWithUTF8String:ResolvePath(path).c_str()];
  return [m_fileManager fileExistsAtPath:nsPath];
}

bool NSFileSystem::IsDirectory(const AmOsString &path) const {
  NSString *nsPath = [NSString stringWithUTF8String:ResolvePath(path).c_str()];
  BOOL isDirectory = NO;
  [m_fileManager fileExistsAtPath:nsPath isDirectory:&isDirectory];
  return isDirectory;
}

AmOsString NSFileSystem::Join(const std::vector<AmOsString> &parts) const {
  NSMutableString *result = [NSMutableString string];
  for (const auto &part : parts) {
    [result appendString:[NSString stringWithUTF8String:part.c_str()]];
    [result appendString:@"/"];
  }
  if ([result length] > 0) {
    [result deleteCharactersInRange:NSMakeRange([result length] - 1, 1)];
  }
  return [result UTF8String];
}

std::shared_ptr<File> NSFileSystem::OpenFile(const AmOsString &path,
                                             eFileOpenMode mode) const {
  return AmSharedPtr<NSFile, eMemoryPoolKind_IO>::Make(ResolvePath(path), mode,
                                                       eFileOpenKind_Binary);
}

void NSFileSystem::StartOpenFileSystem() {
  // No specific initialization needed for iOS file system
}

bool NSFileSystem::TryFinalizeOpenFileSystem() {
  // Always return true as there's no asynchronous initialization
  return true;
}

void NSFileSystem::StartCloseFileSystem() {
  // No specific cleanup needed for iOS file system
}

bool NSFileSystem::TryFinalizeCloseFileSystem() {
  // Always return true as there's no asynchronous cleanup
  return true;
}

} // namespace SparkyStudios::Audio::Amplitude
