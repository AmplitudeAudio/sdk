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

#if AM_PLATFORM_IOS

#include <Foundation/Foundation.h>
#include <os/log.h>

#include <SparkyStudios/Audio/Amplitude/IO/iOS/NSLogger.h>

#include <cstring>

namespace SparkyStudios::Audio::Amplitude {
/**
 * @brief Converts a log message level to an os_log_type_t.
 *
 * Maps Amplitude's log levels to the unified logging system's types:
 * - Debug -> OS_LOG_TYPE_DEBUG
 * - Info -> OS_LOG_TYPE_INFO
 * - Warning -> OS_LOG_TYPE_DEFAULT
 * - Error -> OS_LOG_TYPE_ERROR
 * - Critical -> OS_LOG_TYPE_FAULT
 * - Success -> OS_LOG_TYPE_INFO
 *
 * @param[in] level The log message level.
 *
 * @return The corresponding os_log_type_t.
 */
os_log_type_t getLogType(eLogMessageLevel level) {
  switch (level) {
  case eLogMessageLevel_Debug:
    return OS_LOG_TYPE_DEBUG;
  case eLogMessageLevel_Info:
    return OS_LOG_TYPE_INFO;
  case eLogMessageLevel_Warning:
    return OS_LOG_TYPE_DEFAULT;
  case eLogMessageLevel_Error:
    return OS_LOG_TYPE_ERROR;
  case eLogMessageLevel_Critical:
    return OS_LOG_TYPE_FAULT;
  case eLogMessageLevel_Success:
    return OS_LOG_TYPE_INFO;
  default:
    return OS_LOG_TYPE_DEFAULT;
  }
}

/**
 * @brief Converts a log message level to a human-readable string prefix.
 *
 * @param[in] level The log message level.
 *
 * @return A string representation of the log level.
 */
const char *getLevelPrefix(eLogMessageLevel level) {
  switch (level) {
  case eLogMessageLevel_Debug:
    return "DEBUG";
  case eLogMessageLevel_Info:
    return "INFO";
  case eLogMessageLevel_Warning:
    return "WARNING";
  case eLogMessageLevel_Error:
    return "ERROR";
  case eLogMessageLevel_Critical:
    return "CRITICAL";
  case eLogMessageLevel_Success:
    return "SUCCESS";
  default:
    return "UNKNOWN";
  }
}

NSLogger::NSLogger(const char *logTag) : m_log(nullptr) {
  m_log = os_log_create("com.amplitudeaudiosdk", logTag);
}

NSLogger::~NSLogger() { m_log = nullptr; }

void NSLogger::Log(eLogMessageLevel level, const char *file, int line,
                   const AmString &message) {
  if (file == nullptr)
    file = "<unknown>";

  const char *filename = strrchr(file, '/');
  if (filename)
    filename++; // Skip the '/'
  else
    filename = file;

  // Get the appropriate os_log_type for this log level
  os_log_type_t logType = getLogType(level);
  const char *levelPrefix = getLevelPrefix(level);

  // Use os_log for unified logging
  os_log_with_type(m_log, logType, "%{public}s %{public}s:%d %{public}s",
                   levelPrefix, filename, line, message.c_str());
}
} // namespace SparkyStudios::Audio::Amplitude

#endif // AM_PLATFORM_IOS
