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

#include <sstream>

#include <android/log.h>

#include <SparkyStudios/Audio/Amplitude/IO/Android/LogcatLogger.h>

/**
 * @brief Defines an output streams that logs to logcat.
 */
class AndroidLogcatStream final : public std::stringbuf
{
public:
    /**
     * @brief Creates a new output stream for logcat.
     *
     * @param priority The priority channel.
     * @param logTag The log tag to output.
     */
    explicit AndroidLogcatStream(android_LogPriority priority, const char* logTag)
        : _logTag(logTag)
        , _priority(priority)
    {}

protected:
    int sync() override
    {
        __android_log_print(_priority, _logTag, "%s", str().c_str());
        str("");
        return 0;
    }

private:
    const char* _logTag;
    android_LogPriority _priority;
};

namespace SparkyStudios::Audio::Amplitude
{
    android_LogPriority getPriority(eLogMessageLevel level)
    {
        switch (level)
        {
        case eLogMessageLevel_Debug:
            return ANDROID_LOG_DEBUG;
        case eLogMessageLevel_Info:
            return ANDROID_LOG_INFO;
        case eLogMessageLevel_Warning:
            return ANDROID_LOG_WARN;
        case eLogMessageLevel_Error:
            return ANDROID_LOG_ERROR;
        case eLogMessageLevel_Critical:
            return ANDROID_LOG_FATAL;
        case eLogMessageLevel_Success:
            return ANDROID_LOG_UNKNOWN;
        default:
            return ANDROID_LOG_DEFAULT;
        }
    }

    LogcatLogger::LogcatLogger(const char* logTag)
        : m_logTag(logTag)
    {}

    void LogcatLogger::Log(eLogMessageLevel level, const char* file, int line, const AmString& message)
    {
        AndroidLogcatStream stream(getPriority(level), m_logTag);
        std::ostream out(&stream);

        out << message << std::endl;
    }
} // namespace SparkyStudios::Audio::Amplitude

#endif // AM_PLATFORM_ANDROID