// Copyright (c) 2026-present Sparky Studios. All rights reserved.
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

#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

namespace SparkyStudios::Audio::Amplitude::Tests
{
    /// A logger that keeps what it is asked to write, to check when and in which order messages reach it.
    class CaptureLogger final : public Logger
    {
    public:
        [[nodiscard]] std::vector<std::pair<eLogMessageLevel, std::string>> Entries()
        {
            std::lock_guard lock(_mutex);
            return _entries;
        }

        [[nodiscard]] AmSize Count()
        {
            std::lock_guard lock(_mutex);
            return _entries.size();
        }

    protected:
        void Log(eLogMessageLevel level, const char* file, int line, const AmString& message) override
        {
            std::lock_guard lock(_mutex);
            _entries.emplace_back(level, message);
        }

    private:
        std::mutex _mutex;
        std::vector<std::pair<eLogMessageLevel, std::string>> _entries;
    };

    /// Installs a logger as the global one for the lifetime of the object, and restores the previous one after it.
    class ScopedGlobalLogger
    {
    public:
        explicit ScopedGlobalLogger(Logger* logger)
            : _previous(Logger::GetLogger())
        {
            Logger::SetLogger(logger);
        }

        ~ScopedGlobalLogger()
        {
            Logger::SetLogger(_previous);
        }

        ScopedGlobalLogger(const ScopedGlobalLogger&) = delete;
        ScopedGlobalLogger& operator=(const ScopedGlobalLogger&) = delete;

    private:
        Logger* _previous;
    };
} // namespace SparkyStudios::Audio::Amplitude::Tests
