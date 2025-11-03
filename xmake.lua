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

-- Project definition
set_project("Amplitude")
set_version("1.0.0")
set_license("Apache-2.0")
set_languages("c++20")
set_description("A powerful and cross-platform audio engine, optimized for games.")
set_xmakever("3.0.0")

add_repositories("repo xmake/repo", { rootdir = os.scriptdir() })

add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate")

-- Options
option("build_assets")
  set_default(false)
  set_showmenu(true)
  set_description("Build example assets")
option_end()

option("build_samples")
  add_deps("build_assets")
  set_default(false)
  set_showmenu(true)
  set_description("Build samples")

  after_check(function (option)
    if option:enabled() then
      option:dep("build_assets"):enable(true)
    end
  end)
option_end()

option("build_tools")
  set_default(false)
  set_showmenu(true)
  set_description("Build official CLI tools")
option_end()

option("unit_tests")
  add_deps("build_assets")
  add_deps("build_tools")
  set_default(false)
  set_showmenu(true)
  set_description("Enable Unit Testing")

  after_check(function (option)
    if option:enabled() then
      option:dep("build_assets"):enable(true)
      option:dep("build_tools"):enable(true)
    end
  end)
option_end()

option("as_package")
  add_deps("build_assets")
  add_deps("build_samples")
  add_deps("build_tools")
  add_deps("unit_tests")
  set_default(false)
  set_showmenu(true)
  set_description("Configure as a package. This is useful when using Amplitude from sources instead of SDK installation.")

  after_check(function (option)
    if option:enabled() then
      option:dep("build_assets"):enable(false)
      option:dep("build_samples"):enable(false)
      option:dep("build_tools"):enable(false)
      option:dep("unit_tests"):enable(false)
    end
  end)
option_end()

_ARCH_CACHE = {}

-- Hooks
on_config(function(target)
  import("xmake.cpu")
  import("xmake.platform")
  import("core.project.config")

  if _ARCH_CACHE == nil or #_ARCH_CACHE == 0 then
    _ARCH_CACHE = cpu.am_get_supported_archs()
  end

  for _, arch in ipairs(_ARCH_CACHE) do
    local flags, defines, _ = cpu.am_get_arch_info(arch)

    for _, flag in ipairs(flags) do
      target:add("cxxflags", flag.value, { force = true, tools = flag.tools })
    end

    for _, define in ipairs(defines) do
      target:add("defines", define)
    end
  end

  platform.am_apply_detected_platform_defines(target)
end)

-- Dependencies
add_requireconfs("*", { debug = is_mode("debug") })
add_requires("flatbuffers ^25.2.10")
add_requires("xsimd ^13.2.0")
add_requires("miniaudio ^0.11.22")
add_requires("eigen ^5.0.0")
add_requires("lz4 ^1.9.4")

-- Feature-specific dependencies
if has_config("build_samples") then
  add_requires("libsdl2", { configs = { sdlmain = true } })
end

if has_config("build_tools") and not is_plat("android") and not is_plat("iphoneos") then
  add_requires("cli11")
  add_requires("libmysofa")
end

-- Apply debug/release specific defines
if is_mode("release") then
  add_defines("AM_NO_MEMORY_STATS", "AM_NO_ASSERTS", { public = true })
end

-- Thread support for UNIX platforms
if is_plat("macosx") or is_plat("iphoneos") or is_plat("linux") then
  add_syslinks("pthread", "dl")
elseif is_plat("android") then
  add_syslinks("dl")
end

-- FFT and Accelerate Framework for Apple platforms
if is_plat("macosx") or is_plat("iphoneos") then
  add_frameworks("Accelerate", "CoreAudio", "AudioToolbox", "CoreFoundation")
  add_defines("AM_FFT_APPLE_ACCELERATE")
end

-- Android-specific libraries
if is_plat("android") then
  add_syslinks("android", "log")
end

-- Plugins disabled on iOS/Android due to:
-- 1. App Store dynamic code loading restrictions
-- 2. Platform limitations on dlopen/dlsym
if not is_plat("iphoneos") and not is_plat("android") then
  add_requires("dylib ^3.0.1")
  add_packages("dylib")
else
  add_defines("AM_PLUGINS_UNSUPPORTED")
end

-- Add packages
add_packages("flatbuffers", "xsimd", "eigen", "miniaudio", "lz4")

-- Generate FlatBuffers schema files
target("generate_includes")
  set_kind("phony")
  set_default(false)
  set_policy("build.fence", true)

  on_build(function(target)
    import("core.project.config")
    import("lib.detect.find_tool")

    local flatc = find_tool("flatc", { paths = { "$(env PATH)", "$(projectdir)/bin" } })

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

      print("FlatBuffers headers generated successfully.")
    else
      raise("flatc not found. Please install flatbuffers.")
    end
  end)
target_end()

-- Build binary schemas
target("build_binary_schemas")
  set_kind("phony")
  set_default(false)
  set_policy("build.fence", true)

  on_build(function(target)
    import("core.project.config")
    import("lib.detect.find_program")
    import("lib.detect.find_tool")

    local python = find_program("python3") or find_program("python")
    local scripts_dir = path.join(os.projectdir(), "scripts")
    local schemas_dir = path.join(os.projectdir(), "schemas")
    local flatc = find_tool("flatc", { paths = { "$(env PATH)", "$(projectdir)/bin" } })

    if python then
      os.exec("%s %s/build_schemas.py -p %s -b %s -f %s", python, scripts_dir, schemas_dir, schemas_dir, flatc.program)
    else
      raise("Python not found. Cannot build binary schemas.")
    end
  end)
target_end()

-- Amplitude library
local function _setup(kind)
  set_kind(kind)
  set_default(is_kind(kind))
  set_basename("Amplitude")
  set_targetdir("$(builddir)/$(plat)/$(arch)/$(mode)/" .. kind)

  if not has_config("as_package") then
    set_prefixdir("/", { libdir = "lib/$(arch)-$(plat)/" .. kind, bindir = "lib/$(arch)-$(plat)/" .. kind })
  end

  if is_mode("debug") then
    set_suffixname("_d")
  end

  add_deps("generate_includes", "build_binary_schemas", { inherit = false })

  -- Include paths
  add_includedirs("src", { public = false })
  add_includedirs("include", { public = true })
  add_includedirs("$(builddir)/include", { public = false })

  add_defines("AM_BUILDSYSTEM_BUILDING_AMPLITUDE", { public = false })

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

  if is_plat("iphoneos") then
    add_files("src/**.mm")
    add_files("src/Utils/miniaudio/miniaudio.cpp", { cxxflags = "-x objective-c++" })
  end

  -- Config files
  set_configdir("$(projectdir)")
  add_configfiles("xmake/config/(**.in)")

  -- INSTALLATION
  -- ----------------------------------------

  add_headerfiles("include/(**.h)")

  add_installfiles("(schemas/**.bfbs)")
  add_installfiles("(scripts/*.py)")
  add_installfiles("(cmake/*.cmake)")

  remove_installfiles("scripts/build_schemas.py", "scripts/__pycache__/*.pyc")
end

if not has_config("as_package") then
  namespace("Amplitude")
    target("Shared")
      _setup("shared")
      add_defines("AM_BUILDSYSTEM_SHARED", { public = true })
    target_end()

    target("Static")
      _setup("static")
      add_defines("AM_BUILDSYSTEM_STATIC", { public = true })
    target_end()
  namespace_end()
else
  target("Amplitude")
    _setup("$(kind)")

    if is_kind("shared") then
      add_defines("AM_BUILDSYSTEM_SHARED", { public = true })
    elseif is_kind("static") then
      add_defines("AM_BUILDSYSTEM_STATIC", { public = true })
    end
  target_end()
end

-- Build tools if enabled
if has_config("build_tools") and not is_plat("android") and not is_plat("iphoneos") then
  includes("tools/**/xmake.lua")
end

-- Build sample assets if enabled
if has_config("build_assets") then
  target("build_sample_project")
    set_kind("phony")

    on_build(function(target)
      import("core.project.config")
      import("lib.detect.find_program")
      import("lib.detect.find_tool")

      local python = find_program("python3") or find_program("python")
      local scripts_dir = path.join(os.projectdir(), "scripts")
      local sample_project_dir = path.join(os.projectdir(), "sample_project")
      local output_dir = path.join(config.builddir(), "samples/assets")
      local flatc_path = find_tool("flatc", { paths = { "$(env PATH)", "$(projectdir)/bin" } })
      local schemas_dir = path.join(os.projectdir(), "schemas")

      -- Create output directory
      os.mkdir(output_dir)

      if python then
        os.exec("%s %s/build_project.py -p %s -b %s -f %s -s %s",
          python, scripts_dir, sample_project_dir, output_dir, flatc_path.program, schemas_dir)

        -- Copy assets to output directory
        os.cp(path.join(os.projectdir(), "assets"), path.join(config.builddir(), "samples"))
      else
        raise("Python not found. Cannot build sample project.")
      end
    end)

    add_deps("build_binary_schemas")
  target_end()
end

-- Build samples if enabled
if has_config("build_samples") then
  includes("samples/**/xmake.lua")
end

-- Build unit tests if enabled
if has_config("unit_tests") then
  includes("tests/xmake.lua")

  -- Add code coverage for non-MSVC compilers
  if not is_plat("windows") then
    target("coverage_generate_test_report")
      set_kind("phony")
      set_default(false)

      on_build(function(target)
        import("lib.detect.find_program")

        -- Find required tools
        local kcov = find_program("kcov")

        if not kcov then
          raise("kcov not found. Please install kcov package.")
        end

        local coverage_dir = path.join(os.projectdir(), "coverage")
        local merged_dir = path.join(coverage_dir, "merged")
        local split_dir = path.join(coverage_dir, "split_*")

        local sources = {}
        for _, dir in ipairs(os.dirs(split_dir)) do
          table.insert(sources, dir)
        end

        -- Generate HTML report
        print("Generating HTML coverage report...")
        os.execv(kcov, {
          "--merge", merged_dir, unpack(sources)
        })

        print("Coverage report generated at: %s", merged_dir)
        print("Open %s to view the report", path.join(merged_dir, "index.html"))
      end)
    target_end()

    target("coverage_clean")
      set_kind("phony")
      set_default(false)

      on_build(function(target)
        import("core.project.config")

        local coverage_dir = path.join(os.projectdir(), "coverage")

        print("Cleaning coverage data...")
        os.rm(coverage_dir)

        -- Remove gcov data files
        local gcda_files = os.files(path.join(config.builddir(), "**.gcda"))
        local gcno_files = os.files(path.join(config.builddir(), "**.gcno"))

        for _, file in ipairs(gcda_files) do
          os.rm(file)
        end

        for _, file in ipairs(gcno_files) do
          os.rm(file)
        end

        print("Coverage data cleaned")
      end)
    target_end()
  end
end
