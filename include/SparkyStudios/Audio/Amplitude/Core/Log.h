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

#ifndef SPARK_AUDIO_LOG_H
#define SPARK_AUDIO_LOG_H

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

/**
 * @brief The global logger instance.
 */
#define amLogger SparkyStudios::Audio::Amplitude::Logger::GetLogger()

/**
 * @brief Logs a message with the given level.
 *
 * @param _level_ The level of the log message.
 * @param _message_ The message to log.
 * @param ... The arguments to format the message with.
 */
#define amLog(_level_, _message_, ...)                                                                                                     \
    if (amLogger != nullptr)                                                                                                               \
    {                                                                                                                                      \
        constexpr size_t bufferLen = 2048;                                                                                                 \
        char buffer[bufferLen];                                                                                                            \
        int formatted = std::snprintf(buffer, bufferLen, _message_, ##__VA_ARGS__);                                                        \
        amLogger->_level_(__FILE__, __LINE__, AmString(buffer).substr(0, formatted));                                                      \
    }                                                                                                                                      \
    (void)0

/**
 * @brief Logs a debug message.
 *
 * @param _message_ The message to log.
 * @param ... The arguments to format the message with.
 */
#define amLogDebug(_message_, ...) amLog(Debug, _message_, ##__VA_ARGS__)

/**
 * @brief Logs an informational message.
 *
 * @param _message_ The message to log.
 * @param ... The arguments to format the message with.
 */
#define amLogInfo(_message_, ...) amLog(Info, _message_, ##__VA_ARGS__)

/**
 * @brief Logs a warning message.
 *
 * @param _message_ The message to log.
 * @param ... The arguments to format the message with.
 */
#define amLogWarning(_message_, ...) amLog(Warning, _message_, ##__VA_ARGS__)

/**
 * @brief Logs an error message.
 *
 * @param _message_ The message to log.
 * @param ... The arguments to format the message with.
 */
#define amLogError(_message_, ...) amLog(Error, _message_, ##__VA_ARGS__)

/**
 * @brief Logs a critical message.
 *
 * @param _message_ The message to log.
 * @param ... The arguments to format the message with.
 */
#define amLogCritical(_message_, ...) amLog(Critical, _message_, ##__VA_ARGS__)

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief The level of a log message.
     *
     * This is used to determine the importance of a log message.
     */
    enum LogMessageLevel : AmUInt8
    {
        eLML_DEBUG = 0,
        eLML_INFO = 1,
        eLML_WARNING = 2,
        eLML_ERROR = 3,
        eLML_CRITICAL = 4,
    };

    /**
     * @brief The logger class.
     *
     * Base class used to perform logging. Implementations of this class have the ability to display or store
     * log messages wherever they are needed.
     */
    class AM_API_PUBLIC Logger
    {
    public:
        virtual ~Logger() = default;

        /**
         * @brief Sets the logger instance to use when calling @c amLogger
         *
         * @param loggerInstance The logger instance.
         */
        static void SetLogger(Logger* loggerInstance);

        /**
         * @brief Gets the logger instance to use when calling @c amLogger
         *
         * @return The logger instance.
         */
        static Logger* GetLogger();

        /**
         * @brief Logs a debug message.
         *
         * @param file The file where the message was logged.
         * @param line The line where the message was logged.
         * @param message The message to log.
         */
        void Debug(const char* file, int line, const AmString& message);

        /**
         * @brief Logs an informational message.
         *
         * @param file The file where the message was logged.
         * @param line The line where the message was logged.
         * @param message The message to log.
         */
        void Info(const char* file, int line, const AmString& message);

        /**
         * @brief Logs a warning message.
         *
         * @param file The file where the message was logged.
         * @param line The line where the message was logged.
         * @param message The message to log.
         */
        void Warning(const char* file, int line, const AmString& message);

        /**
         * @brief Logs an error message.
         *
         * @param file The file where the message was logged.
         * @param line The line where the message was logged.
         * @param message The message to log.
         */
        void Error(const char* file, int line, const AmString& message);

        /**
         * @brief Logs a critical message.
         *
         * @param file The file where the message was logged.
         * @param line The line where the message was logged.
         * @param message The message to log.
         */
        void Critical(const char* file, int line, const AmString& message);

    protected:
        /**
         * @brief Logs a message with the given level.
         *
         * @param level The level of the log message.
         * @param file The file where the message was logged.
         * @param line The line where the message was logged.
         * @param message The message to log.
         */
        virtual void Log(LogMessageLevel level, const char* file, int line, const AmString& message) = 0;
    };

    /**
     * @brief The console logger class.
     *
     * This class logs messages to the console.
     */
    class AM_API_PUBLIC ConsoleLogger final : public Logger
    {
    public:
        explicit ConsoleLogger(bool displayFileAndLine = true);
        ~ConsoleLogger() override = default;

        /**
         * @copydoc Logger::Log
         */
        void Log(LogMessageLevel level, const char* file, int line, const AmString& message) override;

    private:
        bool m_displayFileAndLine = true;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SPARK_AUDIO_LOG_H
