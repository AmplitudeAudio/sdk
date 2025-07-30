-- Copyright (c) 2021-present Sparky Studios. All rights reserved.
--
-- Licensed under the Apache License, Version 2.0 (the "License");
-- you may not use this file except in compliance with the License.
-- You may obtain a copy of the License at
--
--     http://www.apache.org/licenses/LICENSE-2.0
--
-- Unless required by applicable law or agreed to in writing, software
-- distributed under the License is distributed on an "AS IS" BASIS,
-- WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
-- See the License for the specific language governing permissions and
-- limitations under the License.

-- Detect version from header file
function am_detect_amplitude_version()
    local header_path = path.join(os.projectdir(), "include/SparkyStudios/Audio/Amplitude/Core/Version.h")
    local content = io.readfile(header_path)
    if not content then
        print("[detect_amplitude_version] Failed to read file: " .. header_path)
        return nil
    end

    local major = content:match("#define%s+AM_VERSION_MAJOR%s+(%d+)")
    local minor = content:match("#define%s+AM_VERSION_MINOR%s+(%d+)")
    local patch = content:match("#define%s+AM_VERSION_PATCH%s+(%d+)")

    if not (major and minor and patch) then
        print("[detect_amplitude_version] Failed to extract version macros from: " .. header_path)
        return nil
    end

    local version = string.format("%s.%s.%s", major, minor, patch)
    print("Detected Amplitude Audio SDK Version - " .. version)
    return version
end
