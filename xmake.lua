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

add_rules("mode.debug", "mode.release", "mode.coverage")
add_rules("plugin.compile_commands.autoupdate")

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
  set_default(false)
  set_showmenu(true)
  set_description("Build official CLI tools")
option_end()

option("unit_tests")
  set_default(false)
  set_showmenu(true)
  set_description("Enable Unit Testing")
option_end()

option("coverage_min_threshold")
  set_default(0)
  set_showmenu(true)
  set_description("Minimum coverage percentage required (0 to disable)")
option_end()

option("as_package")
  set_default(false)
  set_showmenu(true)
  set_description("Configure as a package. This is useful when using Amplitude from sources instead of SDK installation.")
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
add_requires("flatbuffers ^25.2.10")
add_requires("dylib ^2.2.1")
add_requires("xsimd ^13.2.0")
add_requires("miniaudio ^0.11.22")
add_requires("eigen ^3.4.0")
add_requires("lz4 ^1.9.4")

-- Feature-specific dependencies
if has_config("build_samples") then
  add_requires("libsdl2", { configs = { sdlmain = true } })
  set_config("build_assets", true)
end

if has_config("unit_tests") then
  set_config("build_assets", true)
end

if has_config("build_tools") and not is_plat("android") and not is_plat("iphoneos") then
  add_requires("cli11")
  add_requires("libmysofa")
end

if has_config("as_package") then
  set_config("build_samples", false)
  set_config("unit_tests", false)
  set_config("build_assets", false)
  set_config("build_tools", false)
end

-- Apply debug/release specific defines
if is_mode("release") then
  add_defines("AM_NO_MEMORY_STATS", "AM_NO_ASSERTS", { public = true })
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
add_packages("flatbuffers", "xsimd", "eigen", "dylib", "miniaudio", "lz4")

-- Generate FlatBuffers schema files
target("generate_includes")
  set_kind("phony")
  set_default(false)
  set_policy("build.fence", true)

  on_build(function(target)
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
    local flatc = find_tool("flatc")

    if python then
      os.exec("%s %s/build_schemas.py --output %s --flatc %s", python, scripts_dir, schemas_dir, flatc.program)
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
      local flatc_path = find_tool("flatc")
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
  if not is_plat("windows") and is_mode("coverage") then
    target("coverage_generate_baseline_report")
      set_kind("phony")
      set_default(false)

      on_build(function(target)
        import("core.project.config")
        import("lib.detect.find_program")

        -- Find required tools
        local lcov = find_program("lcov")
        local geninfo = find_program("geninfo")

        if not lcov or not geninfo then
          raise("lcov not found. Please install lcov package.")
        end

        -- Get configuration
        local coverage_dir = path.join(config.builddir(), "coverage")
        local data_dir = path.join(coverage_dir, "data")

        -- Create directories
        os.mkdir(data_dir)

        print("Generating baseline coverage report...")

        -- Initialize coverage counters
        print("Initializing coverage data...")
        os.exec("%s --directory %s --zerocounters", lcov, config.builddir())

        local baseline_info = path.join(data_dir, "baseline.info")

        -- Use geninfo directly for better control
        local all_info_files = {}
        local search_dirs = os.dirs(path.join(config.builddir(), "**"))

        for i, dir in ipairs(search_dirs) do
          local gcno_files = os.files(path.join(dir, "*.gcno"))
          if #gcno_files > 0 then
            local info_file = path.join(data_dir, path.filename(dir) .. i .. ".info")
            os.exec(
              "%s %s --base-directory %s --initial --output-file %s --ignore-errors inconsistent,range,mismatch,source,count,negative",
              geninfo, dir, os.projectdir(), info_file)
            table.insert(all_info_files, info_file)
          end
        end

        -- Combine all info files
        os.exec(
        "%s -a %s --output-file %s --ignore-errors inconsistent,range,mismatch,source,count,negative,unused,corrupt", lcov,
          table.concat(all_info_files, " -a "), baseline_info)

        for _, file in ipairs(all_info_files) do
          os.rm(file)
        end

        print("Baseline coverage report generated at: %s", baseline_info)
      end)
    target_end()

    target("coverage_generate_test_report")
      set_kind("phony")
      set_default(false)

      on_build(function(target)
        import("core.project.config")
        import("lib.detect.find_program")
        import("core.base.option")

        -- Find required tools
        local lcov = find_program("lcov")
        local genhtml = find_program("genhtml")

        if not lcov or not genhtml then
          raise("lcov not found. Please install lcov package.")
        end

        -- Get configuration
        local coverage_dir = path.join(config.builddir(), "coverage")
        local data_dir = path.join(coverage_dir, "data")
        local html_dir = path.join(coverage_dir, "html")
        local min_threshold = config.get("coverage_min_threshold") or 0

        -- Create directories
        os.mkdir(data_dir)
        os.mkdir(html_dir)

        print("Generating test coverage report...")

        local baseline_info = path.join(data_dir, "baseline.info")

        -- Check if baseline info exists
        if not os.isfile(baseline_info) then
          raise("Baseline coverage file not found. Run 'xmake build coverage_generate_baseline_report' first.")
        end

        -- Check if test coverage data exists
        local gcda_files = os.files(path.join(config.builddir(), "**.gcda"))
        if #gcda_files == 0 then
          raise("No test coverage data found. Make sure to run 'xmake test' first.")
        end

        -- Capture test coverage
        print("Capturing test coverage data...")
        local testrun_info = path.join(data_dir, "testrun.info")
        os.exec(
          "%s --directory %s --capture --output-file %s --ignore-errors inconsistent,range,mismatch,source,count,negative,corrupt",
          lcov, config.builddir(), testrun_info)

        -- Combine baseline and test coverage
        print("Combining coverage data...")
        local combined_info = path.join(data_dir, "combined.info")
        os.exec("%s --add-tracefile %s --add-tracefile %s --output-file %s --ignore-errors inconsistent,range,mismatch,source,count,negative,corrupt",
          lcov, baseline_info, testrun_info, combined_info)

        -- Filter coverage data
        print("Filtering coverage data...")
        local filtered_info = path.join(data_dir, "filtered.info")
        local project_dir = os.projectdir()
        os.exec(
          "%s --remove %s '/usr/*' '*/tests/*' '*/.xmake/*' '*/build/*' '*/src/Utils/*' '*/samples/*' --output-file %s --ignore-errors inconsistent,range,mismatch,source,count,negative,unused,corrupt",
          lcov, combined_info, filtered_info)

        -- Extract coverage data to include only project sources
        os.exec(
          "%s --extract %s '%s/src/*' '%s/include/*' --output-file %s --ignore-errors inconsistent,range,mismatch,source,count,negative,unused,corrupt",
          lcov, filtered_info, project_dir, project_dir, filtered_info)

        -- Generate HTML report
        print("Generating HTML coverage report...")
        os.exec(
          "%s %s --output-directory %s --title 'Amplitude Audio SDK Coverage Report' --num-spaces 4 --legend --show-details --branch-coverage --ignore-errors inconsistent,range,mismatch,source,count,negative,unused,corrupt",
          genhtml, filtered_info, html_dir)

        -- Extract coverage summary
        local coverage_summary = os.iorun(
        "%s --summary %s --ignore-errors inconsistent,range,mismatch,source,count,negative,unused,corrupt", lcov,
          filtered_info)

        print("Coverage Summary:")
        print(coverage_summary)

        -- Check minimum threshold
        if min_threshold > 0 then
          local line_coverage = coverage_summary:match("lines%.*: ([%d%.]+)%%")
          if line_coverage then
            local coverage_percent = tonumber(line_coverage)
            if coverage_percent < min_threshold then
              raise("Coverage %g%% is below minimum threshold %g%%", coverage_percent, min_threshold)
            else
              print("Coverage %g%% meets minimum threshold %g%%", coverage_percent, min_threshold)
            end
          end
        end

        print("Coverage report generated at: %s", html_dir)
        print("Open %s to view the report", path.join(html_dir, "index.html"))
      end)
    target_end()

    target("coverage_clean")
      set_kind("phony")
      set_default(false)

      on_build(function(target)
        import("core.project.config")

        local coverage_dir = path.join(config.builddir(), "coverage")

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
