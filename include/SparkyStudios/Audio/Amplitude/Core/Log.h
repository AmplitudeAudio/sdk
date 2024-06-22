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

#include <format>

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>

#define CHECK_VARARGS_HAS_VALUES(...) ((sizeof((char[]){ #__VA_ARGS__ }) / sizeof(char)) > 1)

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
        if constexpr (CHECK_VARARGS_HAS_VALUES(__VA_ARGS__))                                                                               \
            amLogger->_level_(std::vformat(_message_, std::make_format_args(__VA_ARGS__)), __FILE__, __LINE__);                                                  \
        else                                                                                                                               \
            amLogger->_level_(_message_, __FILE__, __LINE__);                                                                              \
    }                                                                                                                                      \
    (void)0

/**
 * @brief Logs a debug message.
 *
 * @param _message_ The message to log.
 * @param ... The arguments to format the message with.
 */
#define amLogDebug(_message_, ...) amLog(Debug, _message_, __VA_ARGS__)

/**
 * @brief Logs an informational message.
 *
 * @param _message_ The message to log.
 * @param ... The arguments to format the message with.
 */
#define amLogInfo(_message_, ...) amLog(Info, _message_, __VA_ARGS__)

/**
 * @brief Logs a warning message.
 *
 * @param _message_ The message to log.
 * @param ... The arguments to format the message with.
 */
#define amLogWarning(_message_, ...) amLog(Warning, _message_, __VA_ARGS__)

/**
 * @brief Logs an error message.
 *
 * @param _message_ The message to log.
 * @param ... The arguments to format the message with.
 */
#define amLogError(_message_, ...) amLog(Error, _message_, __VA_ARGS__)

/**
 * @brief Logs a critical message.
 *
 * @param _message_ The message to log.
 * @param ... The arguments to format the message with.
 */
#define amLogCritical(_message_, ...) amLog(Critical, _message_, __VA_ARGS__)

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
         * @param message The message to log.
         * @param file The file where the message was logged.
         * @param line The line where the message was logged.
         */
        void Debug(const std::string& message, const char* file, int line);

        /**
         * @brief Logs an informational message.
         *
         * @param message The message to log.
         * @param file The file where the message was logged.
         * @param line The line where the message was logged.
         */
        void Info(const std::string& message, const char* file, int line);

        /**
         * @brief Logs a warning message.
         *
         * @param message The message to log.
         * @param file The file where the message was logged.
         * @param line The line where the message was logged.
         */
        void Warning(const std::string& message, const char* file, int line);

        /**
         * @brief Logs an error message.
         *
         * @param message The message to log.
         * @param file The file where the message was logged.
         * @param line The line where the message was logged.
         */
        void Error(const std::string& message, const char* file, int line);

        /**
         * @brief Logs a critical message.
         *
         * @param message The message to log.
         * @param file The file where the message was logged.
         * @param line The line where the message was logged.
         */
        void Critical(const std::string& message, const char* file, int line);

    protected:
        /**
         * @brief Logs a message with the given level.
         *
         * @param level The level of the log message.
         * @param message The message to log.
         * @param file The file where the message was logged.
         * @param line The line where the message was logged.
         */
        virtual void Log(LogMessageLevel level, const std::string& message, const char* file, int line) = 0;
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
        void Log(LogMessageLevel level, const std::string& message, const char* file, int line) override;

    private:
        bool m_displayFileAndLine = true;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SPARK_AUDIO_LOG_H
