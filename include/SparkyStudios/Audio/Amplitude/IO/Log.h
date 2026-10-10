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

#ifndef _AM_IO_LOG_H
#define _AM_IO_LOG_H

#include <atomic>
#include <memory>
#include <mutex>

#include <SparkyStudios/Audio/Amplitude/Core/Common.h>
#include <SparkyStudios/Audio/Amplitude/Core/MPSCQueue.h>

/**
 * @brief The global logger instance.
 *
 * @ingroup io
 */
#define amLogger SparkyStudios::Audio::Amplitude::Logger::GetLogger()

/**
 * @brief Logs a message with the given level.
 *
 * @param _level_ The level of the log message.
 * @param _message_ The message to log.
 * @param ... The arguments to format the message with.
 *
 * @ingroup io
 */
#define amLog(_level_, _message_, ...)                                                                                                     \
    if (amLogger != nullptr)                                                                                                               \
    {                                                                                                                                      \
        constexpr size_t bufferLen = 4096;                                                                                                 \
        char buffer[bufferLen];                                                                                                            \
        const int formatted = std::snprintf(buffer, bufferLen, _message_, ##__VA_ARGS__);                                                  \
        const size_t length =                                                                                                              \
            formatted < 0 ? 0 : (static_cast<size_t>(formatted) < bufferLen ? static_cast<size_t>(formatted) : bufferLen - 1);             \
        SparkyStudios::Audio::Amplitude::Logger::Emit(                                                                                     \
            SparkyStudios::Audio::Amplitude::eLogMessageLevel_##_level_, __FILE__, __LINE__, buffer, length);                              \
    }                                                                                                                                      \
    (void)0

/**
 * @brief Logs a debug message.
 *
 * @param _message_ The message to log.
 * @param ... The arguments to format the message with.
 *
 * @ingroup io
 */
#define amLogDebug(_message_, ...) amLog(Debug, _message_, ##__VA_ARGS__)

/**
 * @brief Logs an informational message.
 *
 * @param _message_ The message to log.
 * @param ... The arguments to format the message with.
 *
 * @ingroup io
 */
#define amLogInfo(_message_, ...) amLog(Info, _message_, ##__VA_ARGS__)

/**
 * @brief Logs a warning message.
 *
 * @param _message_ The message to log.
 * @param ... The arguments to format the message with.
 *
 * @ingroup io
 */
#define amLogWarning(_message_, ...) amLog(Warning, _message_, ##__VA_ARGS__)

/**
 * @brief Logs an error message.
 *
 * @param _message_ The message to log.
 * @param ... The arguments to format the message with.
 *
 * @ingroup io
 */
#define amLogError(_message_, ...) amLog(Error, _message_, ##__VA_ARGS__)

/**
 * @brief Logs a critical message.
 *
 * @param _message_ The message to log.
 * @param ... The arguments to format the message with.
 *
 * @ingroup io
 */
#define amLogCritical(_message_, ...) amLog(Critical, _message_, ##__VA_ARGS__)

/**
 * @brief Logs a success message.
 *
 * @param _message_ The message to log.
 * @param ... The arguments to format the message with.
 *
 * @ingroup io
 */
#define amLogSuccess(_message_, ...) amLog(Success, _message_, ##__VA_ARGS__)

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief The level of a log message.
     *
     * This is used to determine the importance of a log message.
     *
     * @ingroup io
     */
    enum eLogMessageLevel : AmUInt8
    {
        /**
         * @brief Debug messages.
         */
        eLogMessageLevel_Debug = 0,

        /**
         * @brief Informational messages.
         */
        eLogMessageLevel_Info = 1,

        /**
         * @brief Warning messages.
         */
        eLogMessageLevel_Warning = 2,

        /**
         * @brief Error messages.
         */
        eLogMessageLevel_Error = 3,

        /**
         * @brief Critical messages.
         */
        eLogMessageLevel_Critical = 4,

        /**
         * @brief Success messages.
         */
        eLogMessageLevel_Success = 5,
    };

    /**
     * @brief The logger class.
     *
     * Base class used to perform logging. Implementations of this class can display or store
     * log messages wherever they are needed.
     *
     * Messages logged from the audio thread (see @c Logger::ScopedAudioThread) are never written there: they are
     * copied into a lock-free queue, and written by the next @c Flush(), which the engine calls once per frame. Messages
     * logged from any other thread are written at once, after the ones still queued, so the order is kept.
     *
     * @warning The global logger is meant to be installed once during engine setup. It is not meant to be swapped, or
     * destroyed while the engine runs. The installed logger must be kept alive until the engine is deinitialized.
     *
     * @ingroup io
     */
    class AM_API_PUBLIC Logger
    {
    public:
        /**
         * @brief Marks the calling thread as an audio thread while it is alive.
         *
         * Logging from a marked thread never takes a lock, allocates or writes: the message is queued for @c Flush().
         * The mixer marks the thread that runs it. Scopes can be nested.
         */
        class AM_API_PUBLIC ScopedAudioThread
        {
        public:
            ScopedAudioThread();
            ~ScopedAudioThread();

            ScopedAudioThread(const ScopedAudioThread&) = delete;
            ScopedAudioThread& operator=(const ScopedAudioThread&) = delete;

        private:
            bool _previous;
        };

        /**
         * @brief The longest message, in characters, kept when it is logged from an audio thread. A longer one is cut.
         */
        static constexpr AmSize kMaxQueuedMessageLength = 192;

        /**
         * @brief Creates the logger.
         */
        Logger();

        /**
         * @brief Destroys the logger, and unregisters it if it is the global one.
         *
         * Messages still queued are lost: a logger that writes somewhere should call @c Flush() in its own destructor,
         * while it can still write.
         */
        virtual ~Logger();

        /**
         * @brief Sets the logger instance to use when calling @c amLogger
         *
         * Messages still queued on the logger being replaced are written first.
         *
         * Call it before initializing the engine, and again (with the previous logger, or @c nullptr) only after the
         * engine is deinitialized.
         *
         * @param[in] loggerInstance The logger instance.
         */
        static void SetLogger(Logger* loggerInstance);

        /**
         * @brief Gets the logger instance to use when calling @c amLogger
         *
         * @return The logger instance.
         */
        static Logger* GetLogger();

        /**
         * @brief Logs a message with the global logger, if there is one.
         *
         * This is what the @c amLog macros call: the global logger is read once, so it cannot go away between the test
         * and the call.
         *
         * @param[in] level The level of the log message.
         * @param[in] file The file where the message was logged.
         * @param[in] line The line where the message was logged.
         * @param[in] message The message, not necessarily null-terminated.
         * @param[in] length The length of @p message.
         */
        static void Emit(eLogMessageLevel level, const char* file, int line, const char* message, AmSize length);

        /**
         * @brief Logs a debug message.
         *
         * @param[in] file The file where the message was logged.
         * @param[in] line The line where the message was logged.
         * @param[in] message The message to log.
         */
        void Debug(const char* file, int line, const AmString& message);

        /**
         * @brief Logs an informational message.
         *
         * @param[in] file The file where the message was logged.
         * @param[in] line The line where the message was logged.
         * @param[in] message The message to log.
         */
        void Info(const char* file, int line, const AmString& message);

        /**
         * @brief Logs a warning message.
         *
         * @param[in] file The file where the message was logged.
         * @param[in] line The line where the message was logged.
         * @param[in] message The message to log.
         */
        void Warning(const char* file, int line, const AmString& message);

        /**
         * @brief Logs an error message.
         *
         * @param[in] file The file where the message was logged.
         * @param[in] line The line where the message was logged.
         * @param[in] message The message to log.
         */
        void Error(const char* file, int line, const AmString& message);

        /**
         * @brief Logs a critical message.
         *
         * @param[in] file The file where the message was logged.
         * @param[in] line The line where the message was logged.
         * @param[in] message The message to log.
         */
        void Critical(const char* file, int line, const AmString& message);

        /**
         * @brief Logs a success message.
         *
         * @param[in] file The file where the message was logged.
         * @param[in] line The line where the message was logged.
         * @param[in] message The message to log.
         */
        void Success(const char* file, int line, const AmString& message);

        /**
         * @brief Logs a message with this logger.
         *
         * Prefer the @c amLog macros. From an audio thread, the message is queued (and cut to
         * @c kMaxQueuedMessageLength characters); from any other thread, it is written at once.
         *
         * @param[in] level The level of the log message.
         * @param[in] file The file where the message was logged.
         * @param[in] line The line where the message was logged.
         * @param[in] message The message, not necessarily null-terminated.
         * @param[in] length The length of @p message.
         */
        void Write(eLogMessageLevel level, const char* file, int line, const char* message, AmSize length);

        /**
         * @brief Writes the messages queued from the audio thread.
         *
         * Safe to call from any thread but an audio thread, and from several at once: writes are serialized. If the queue
         * overflowed, one warning reports how many messages were dropped.
         */
        void Flush();

    protected:
        /**
         * @brief Logs a message with the given level.
         *
         * @param[in] level The level of the log message.
         * @param[in] file The file where the message was logged.
         * @param[in] line The line where the message was logged.
         * @param[in] message The message to log.
         */
        virtual void Log(eLogMessageLevel level, const char* file, int line, const AmString& message) = 0;

    private:
        struct LogEntry
        {
            eLogMessageLevel level;
            int line;
            const char* file;
            AmUInt32 length;
            char message[kMaxQueuedMessageLength + 1];
        };

        static constexpr AmSize kQueueCapacity = 256;

        void Drain();

        std::unique_ptr<MPSCQueue<LogEntry, kQueueCapacity>> _logEntries;
        std::atomic<AmUInt32> _droppedEntries;
        std::recursive_mutex _writeMutex;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // _AM_IO_LOG_H
