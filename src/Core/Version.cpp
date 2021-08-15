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

#include <SparkyStudios/Audio/Amplitude/Core/Version.h>

namespace SparkyStudios::Audio::Amplitude
{
#define SA_VERSION_MAJOR 1
#define SA_VERSION_MINOR 0
#define SA_VERSION_REVISION 0

// Turn X into a string literal.
#define SA_STRING_EXPAND(X) #X
#define SA_STRING(X) SA_STRING_EXPAND(X)

    /// @var kVersion
    /// @brief String which identifies the current version of SparkAudio.
    ///
    /// @ref kVersion is used by Google developers to identify which applications
    /// uploaded to Google Play are using this library. This allows the development
    /// team at Google to determine the popularity of the library.
    /// How it works: Applications that are uploaded to the Google Play Store are
    /// scanned for this version string. We track which applications are using it
    /// to measure popularity. You are free to remove it (of course) but we would
    /// appreciate if you left it in.
    // clang-format off
    static const struct Version kVersion = {
        SA_VERSION_MAJOR,
        SA_VERSION_MINOR,
        SA_VERSION_REVISION,
        "SparkAudio "
        SA_STRING(SA_VERSION_MAJOR) "."
        SA_STRING(SA_VERSION_MINOR) "."
        SA_STRING(SA_VERSION_REVISION)
    };
    // clang-format on

    const struct Version& Version()
    {
        return kVersion;
    }
} // namespace SparkyStudios::Audio::Amplitude