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

#include <atomic>
#include <cstring>
#include <string>

#include <SparkyStudios/Audio/Amplitude/IO/Log.h>

namespace SparkyStudios::Audio::Amplitude
{
    static std::atomic<Logger*> gLogger{ nullptr };

    // Set while the thread is inside a Logger::ScopedAudioThread.
    static thread_local bool gIsAudioThread = false;

    Logger::ScopedAudioThread::ScopedAudioThread()
        : _previous(gIsAudioThread)
    {
        gIsAudioThread = true;
    }

    Logger::ScopedAudioThread::~ScopedAudioThread()
    {
        gIsAudioThread = _previous;
    }

    Logger::Logger()
        : _logEntries(std::make_unique<MPSCQueue<LogEntry, kQueueCapacity>>())
        , _droppedEntries(0)
    {}

    Logger::~Logger()
    {
        // A destroyed logger must not stay the global one: SetLogger() and the macros would reach it.
        Logger* self = this;
        gLogger.compare_exchange_strong(self, nullptr);
    }

    void Logger::SetLogger(Logger* loggerInstance)
    {
        // Whatever the audio thread queued for the logger going away is written now, while it is still alive.
        Logger* previous = gLogger.exchange(loggerInstance);

        if (previous != nullptr && previous != loggerInstance)
            previous->Flush();
    }

    Logger* Logger::GetLogger()
    {
        return gLogger.load();
    }

    void Logger::Emit(eLogMessageLevel level, const char* file, int line, const char* message, AmSize length)
    {
        if (Logger* logger = gLogger.load())
            logger->Write(level, file, line, message, length);
    }

    void Logger::Debug(const char* file, int line, const AmString& message)
    {
        Emit(eLogMessageLevel_Debug, file, line, message.data(), message.size());
    }

    void Logger::Info(const char* file, int line, const AmString& message)
    {
        Emit(eLogMessageLevel_Info, file, line, message.data(), message.size());
    }

    void Logger::Warning(const char* file, int line, const AmString& message)
    {
        Emit(eLogMessageLevel_Warning, file, line, message.data(), message.size());
    }

    void Logger::Error(const char* file, int line, const AmString& message)
    {
        Emit(eLogMessageLevel_Error, file, line, message.data(), message.size());
    }

    void Logger::Critical(const char* file, int line, const AmString& message)
    {
        Emit(eLogMessageLevel_Critical, file, line, message.data(), message.size());
    }

    void Logger::Success(const char* file, int line, const AmString& message)
    {
        Emit(eLogMessageLevel_Success, file, line, message.data(), message.size());
    }

    void Logger::Write(eLogMessageLevel level, const char* file, int line, const char* message, AmSize length)
    {
#ifndef AM_DEBUG
        if (level == eLogMessageLevel_Debug)
            return;
#endif

        if (gIsAudioThread)
        {
            // No lock, no allocation, no I/O: copy the message into the queue.
            LogEntry entry;
            entry.level = level;
            entry.line = line;
            entry.file = file;
            entry.length = static_cast<AmUInt32>(length < kMaxQueuedMessageLength ? length : kMaxQueuedMessageLength);
            std::memcpy(entry.message, message, entry.length);
            entry.message[entry.length] = '\0';

            if (!_logEntries->TryEnqueue(entry))
                _droppedEntries.fetch_add(1, std::memory_order_relaxed);

            return;
        }

        // Not the audio thread: what it queued comes first, then this message, written now.
        std::lock_guard lock(_writeMutex);
        Drain();
        Log(level, file, line, AmString(message, length));
    }

    void Logger::Flush()
    {
        std::lock_guard lock(_writeMutex);
        Drain();
    }

    void Logger::Drain()
    {
        LogEntry entry;
        while (_logEntries->TryDequeue(entry))
            Log(entry.level, entry.file, entry.line, AmString(entry.message, entry.length));

        if (const AmUInt32 dropped = _droppedEntries.exchange(0, std::memory_order_relaxed); dropped > 0)
        {
            const AmString report =
                std::to_string(dropped) + " log message(s) logged from the audio thread were dropped: the log queue was full.";
            Log(eLogMessageLevel_Warning, __FILE__, __LINE__, report);
        }
    }
} // namespace SparkyStudios::Audio::Amplitude
