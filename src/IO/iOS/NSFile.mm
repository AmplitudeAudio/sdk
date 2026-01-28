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

#include <SparkyStudios/Audio/Amplitude/IO/iOS/NSFile.h>

namespace SparkyStudios::Audio::Amplitude {
NSFile::NSFile(const AmOsString &path, eFileOpenMode mode, eFileOpenKind kind)
    : m_path(path), m_isValid(false) {
  NSString *nsPath = [NSString stringWithUTF8String:path.c_str()];
  NSFileManager *fileManager = [NSFileManager defaultManager];

  if (![fileManager fileExistsAtPath:nsPath]) {
    if (mode == eFileOpenMode_Read || mode == eFileOpenMode_ReadWrite ||
        mode == eFileOpenMode_ReadAppend) {
      return; // File doesn't exist and we're trying to read
    }
    [fileManager createFileAtPath:nsPath contents:nil attributes:nil];
  }

  NSData *data = [NSData dataWithContentsOfFile:nsPath];
  if (!data) {
    return; // Failed to read file
  }

  switch (mode) {
  case eFileOpenMode_Read:
    m_fileHandle = [NSFileHandle fileHandleForReadingAtPath:nsPath];
    break;
  case eFileOpenMode_Write:
    m_fileHandle = [NSFileHandle fileHandleForWritingAtPath:nsPath];
    [m_fileHandle truncateFileAtOffset:0];
    break;
  case eFileOpenMode_Append:
    m_fileHandle = [NSFileHandle fileHandleForWritingAtPath:nsPath];
    [m_fileHandle seekToEndOfFile];
    break;
  case eFileOpenMode_ReadWrite:
    m_fileHandle = [NSFileHandle fileHandleForUpdatingAtPath:nsPath];
    break;
  case eFileOpenMode_ReadAppend:
    m_fileHandle = [NSFileHandle fileHandleForUpdatingAtPath:nsPath];
    [m_fileHandle seekToEndOfFile];
    break;
  }

  m_isValid = (m_fileHandle != nil);
}

NSFile::~NSFile() { Close(); }

AmOsString NSFile::GetPath() const { return m_path; }

bool NSFile::Eof() const {
  unsigned long long currentOffset = [m_fileHandle offsetInFile];
  unsigned long long length = [m_fileHandle seekToEndOfFile];
  [m_fileHandle seekToFileOffset:currentOffset];
  return currentOffset >= length;
}

AmSize NSFile::Read(AmUInt8Buffer dst, AmSize bytes) const {
  NSData *data = [m_fileHandle readDataOfLength:bytes];
  AmSize bytesRead = [data length];
  std::memcpy(dst, [data bytes], bytesRead);
  return bytesRead;
}

AmSize NSFile::Write(AmConstUInt8Buffer src, AmSize bytes) {
  NSData *data = [NSData dataWithBytes:src length:bytes];
  [m_fileHandle writeData:data];
  return bytes;
}

AmSize NSFile::Length() const {
  unsigned long long currentOffset = [m_fileHandle offsetInFile];
  unsigned long long length = [m_fileHandle seekToEndOfFile];
  [m_fileHandle seekToFileOffset:currentOffset];
  return static_cast<AmSize>(length);
}

void NSFile::Seek(AmInt64 offset, eFileSeekOrigin origin) {
  switch (origin) {
  case eFileSeekOrigin_Start:
    [m_fileHandle seekToFileOffset:offset];
    break;
  case eFileSeekOrigin_Current:
    [m_fileHandle seekToFileOffset:[m_fileHandle offsetInFile] + offset];
    break;
  case eFileSeekOrigin_End:
    [m_fileHandle seekToEndOfFile];
    [m_fileHandle seekToFileOffset:[m_fileHandle offsetInFile] + offset];
    break;
  }
}

AmSize NSFile::Position() const {
  return static_cast<AmSize>([m_fileHandle offsetInFile]);
}

AmVoidPtr NSFile::GetPtr() const { return (__bridge void *)m_fileHandle; }

bool NSFile::IsValid() const { return m_isValid; }

void NSFile::Close() {
  if (m_fileHandle) {
    [m_fileHandle closeFile];
    m_fileHandle = nil;
    m_isValid = false;
  }
}
} // namespace SparkyStudios::Audio::Amplitude
