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

-- Set minimum xmake version
set_xmakever("2.7.0")

add_repositories("repo xmake/repo", {rootdir = os.scriptdir()})

-- Project definition
set_project("Amplitude")
set_version("1.0")
set_license("Apache-2.0")
set_languages("c++20")
set_description("A powerful and cross-platform audio engine, optimized for games.")
add_rules("mode.debug", "mode.release")

-- Options
option("build_assets")
    set_default(false)
    set_showmenu(true)
    set_description("Build example assets")
option_end()

option("build_samples")
    set_default(false)
    set_showmenu(true)
    set_description("Build samples")
option_end()

option("build_tools")
    set_default(true)
    set_showmenu(true)
    set_description("Build official CLI tools")
option_end()

option("unit_tests")
    set_default(false)
    set_showmenu(true)
    set_description("Enable Unit Testing")
option_end()

-- Hooks
on_config(function (target)
    import("xmake.cpu")
    import("xmake.platform")

    local archs = cpu.am_get_supported_archs()
    for _, arch in ipairs(archs) do
        local flags, define, suffix = cpu.am_get_arch_info(arch)

        for _, flag in ipairs(flags) do
            target:add("cflags", flag, {force = true})
        end

        target:add("defines", define)
    end

    platform.am_apply_detected_platform_defines(target)
end)

-- Dependencies
add_requires("flatbuffers >= 25.2.10")
add_requires("dylib >= 2.2.1")
add_requires("xsimd >= 13.2.0")
add_requires("miniaudio >= 0.11.22")
add_requires("eigen >= 3.4.0")

-- Feature-specific dependencies
if has_config("build_samples") then
    add_requires("sdl2")
    set_config("build_assets", true)
end

if has_config("unit_tests") then
    add_rules("mode.coverage")
    add_requires("catch2")
    set_config("build_assets", true)
end

if has_config("build_tools") and not is_plat("android") and not is_plat("iphoneos") then
    add_requires("cli11")
    -- add_requires("cmakerc")
    add_requires("libmysofa")
end

-- Apply debug/release specific defines
if is_mode("release") then
    add_defines("AM_NO_MEMORY_STATS", "AM_NO_ASSERTS", {public = true})
end

-- Thread support for UNIX platforms
if is_plat("macosx") or is_plat("iphoneos") or is_plat("linux") or is_plat("android") then
    add_syslinks("pthread", "dl")
end

-- FFT and Accelerate Framework for Apple platforms
if is_plat("macosx") or is_plat("iphoneos") then
    add_frameworks("Accelerate")
    add_defines("AM_FFT_APPLE_ACCELERATE")
end

-- Android-specific libraries
if is_plat("android") then
    add_syslinks("android", "log")
end

-- Add packages
add_packages("flatbuffers", "dylib", "xsimd", "miniaudio", "eigen")

-- Generate FlatBuffers schema files
target("generated_includes")
    set_kind("phony")
    set_default(false)

    on_build(function (target)
        import("core.project.config")
        import("lib.detect.find_tool")

        local flatc = find_tool("flatc")
        if flatc then
            local schemas_dir = path.join(os.projectdir(), "schemas")
            local output_dir = path.join(path.absolute(config.builddir()), "include")

            -- Create output directory
            os.mkdir(output_dir)

            -- Generate headers from schema files
            os.cd(schemas_dir)
            local schema_files = os.files("*.fbs")
            for _, schema in ipairs(schema_files) do
                os.exec("%s --cpp -o %s %s", flatc.program, output_dir, schema)
            end
        else
            print("flatc not found. Please install flatbuffers.")
        end
    end)
target_end()

-- Build binary schemas
target("build_binary_schemas")
    set_kind("phony")
    set_default(false)

    on_build(function (target)
        import("core.project.config")
        import("lib.detect.find_program")

        local python = find_program("python3") or find_program("python")
        local scripts_dir = path.join(os.projectdir(), "scripts")
        local schemas_dir = path.join(os.projectdir(), "schemas")

        if python then
            os.exec("%s %s/build_schemas.py --output %s", python, scripts_dir, schemas_dir)
        else
            print("Python not found. Cannot build binary schemas.")
        end
    end)
target_end()

-- Amplitude library
target("Amplitude")
    set_kind("$(kind)")
    set_default(true)
    set_basename("Amplitude")

    add_deps("generated_includes", "build_binary_schemas")

    -- Include paths
    add_includedirs("src", {public = false})
    add_includedirs("include", {public = true})
    add_includedirs("$(builddir)/include", {public = false})

    -- Define the build type
    if (is_kind("static")) then
        add_defines("AM_BUILDSYSTEM_STATIC", {public = true})
    elseif (is_kind("shared")) then
        add_defines("AM_BUILDSYSTEM_SHARED", {public = true})
    end

    add_defines("AM_BUILDSYSTEM_BUILDING_AMPLITUDE")

    -- Common sources
    add_files("src/**/*.cpp")

    -- Platform-specific sources,
    if not is_plat("android") then
        remove_files(
            "src/IO/Android/AssetManagerFile.cpp",
            "src/IO/Android/AssetManagerFileSystem.cpp",
            "src/IO/Android/LogcatLogger.cpp"
        )
    end

    -- INSTALLATION
    -- ----------------------------------------

    -- Header files
    add_headerfiles("include/(**.h)")

    -- Schema files
    add_installfiles("(schemas/**.bfbs)")

    -- Python files
    add_installfiles("(scripts/*.py)")
target_end()
 
-- Build tools if enabled
if has_config("build_tools") and not is_plat("android") and not is_plat("iphoneos") then
    includes("tools/amac/xmake.lua")
    includes("tools/ampk/xmake.lua")
    includes("tools/amir/xmake.lua")
    -- includes("tools/ampm/xmake.lua")
end

-- Build sample assets if enabled
if has_config("build_assets") then
    target("ss_amplitude_audio_sample_project")
        set_kind("phony")

        on_build(function (target)
            import("core.project.config")

            local python = os.find_program("python3") or os.find_program("python")
            local scripts_dir = path.join(os.projectdir(), "scripts")
            local sample_project_dir = path.join(os.projectdir(), "sample_project")
            local output_dir = path.join(config.builddir(), "samples/assets")
            local flatc_path = path.join(os.projectdir(), "bin/flatc")
            local schemas_dir = path.join(os.projectdir(), "schemas")

            -- Create output directory
            os.mkdir(output_dir)

            if python then
                os.exec("%s %s/build_project.py -p %s -b %s -f %s -s %s",
                    python, scripts_dir, sample_project_dir, output_dir, flatc_path, schemas_dir)

                -- Copy assets to output directory
                os.cp(path.join(os.projectdir(), "assets"), output_dir)
            else
                print("Python not found. Cannot build sample project.")
            end
        end)

        add_deps("build_binary_schemas")
    target_end()
end

-- Build samples if enabled
if has_config("build_samples") then
    includes("samples/xmake.lua")
end 

-- Build unit tests if enabled
if has_config("unit_tests") then
    includes("tests/xmake.lua")

    -- Add code coverage for non-MSVC compilers
    if not is_plat("windows") then
        -- Code coverage settings would go here
        -- In a real implementation, this would setup LCOV or similar tools
    end
end
