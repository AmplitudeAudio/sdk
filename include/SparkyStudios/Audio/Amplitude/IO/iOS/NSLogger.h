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

#if AM_PLATFORM_IOS

#ifndef _AM_IO_IOS_NS_LOGGER_H
#define _AM_IO_IOS_NS_LOGGER_H

#include <Foundation/Foundation.h>

#include <SparkyStudios/Audio/Amplitude/IO/Log.h>

// Forward declare os_log_t to avoid including os/log.h in the header
typedef struct os_log_s *os_log_t;

namespace SparkyStudios::Audio::Amplitude {
/**
 * @brief A logger that writes to iOS's unified logging system.
 *
 * This class logs output to the iOS unified logging system with a custom
 * subsystem and category. Messages will appear in the device console, Xcode's
 * debug console, and can be viewed with the Console.app and log command-line
 * tool.
 *
 * @ingroup io
 */
class AM_API_PUBLIC NSLogger final : public Logger {
public:
  /**
   * @brief Constructs a new NSLogger.
   *
   * @param[in] logTag A custom category to use when logging. This is used along
   *                   with the subsystem identifier for organizing and
   * filtering logs. Defaults to @b AM.
   */
  explicit NSLogger(const char *logTag = "AM");

  /**
   * @brief Destructor that releases allocated resources.
   */
  ~NSLogger() override;

protected:
  /**
   * @inherit
   */
  void Log(eLogMessageLevel level, const char *file, int line,
           const AmString &message) override;

private:
  os_log_t m_log;
  NSString *m_logTag;
};
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IO_IOS_NS_LOGGER_H

#endif // AM_PLATFORM_IOS
