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

-- Custom coverage mode rule for llvm-cov
rule("mode.coverage.llvm")
  on_config(function(target)
    if is_mode("coverage") then
      -- Enable debug symbols and disable optimization
      if not target:get("symbols") then
        target:set("symbols", "debug")
      end
      if not target:get("optimize") then
        target:set("optimize", "none")
      end
      target:set("policy", "build.ccache", false)

      -- Only apply llvm-cov flags for Clang-based compilers
      local dominated = false
      if target:has_tool("cxx", "clang", "clangxx") then
        dominated = true
      elseif target:has_tool("cc", "clang") then
        dominated = true
      end

      -- macOS always uses Clang
      if is_plat("macosx", "iphoneos") then
        dominated = true
      end

      if dominated then
        target:add("cxflags", "-fprofile-instr-generate", "-fcoverage-mapping", { force = true })
        target:add("mxflags", "-fprofile-instr-generate", "-fcoverage-mapping", { force = true })
        target:add("ldflags", "-fprofile-instr-generate", "-fcoverage-mapping", { force = true })
        target:add("shflags", "-fprofile-instr-generate", "-fcoverage-mapping", { force = true })
      end
    end
  end)
rule_end()

-- Apply coverage rule to all targets
add_rules("mode.coverage.llvm")

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
  if is_plat("android") then
    -- Android: Use Gradle build system instead of XMake
    -- Build with: cd tests/android_runner && ./gradlew assembleDebug
    print("Note: Android tests use Gradle build system.")
    print("  Build: cd tests/android_runner && ./gradlew assembleDebug")
    print("  Install: ./gradlew installDebug")
  else
    -- Desktop and iOS: Use XMake build system
    includes("tests/xmake.lua")
  end

  -- Add code coverage for non-MSVC compilers (desktop only)
  if not is_plat("windows", "iphoneos", "android") then
    target("coverage_generate_test_report")
      set_kind("phony")
      set_default(false)

      on_build(function(target)
        import("lib.detect.find_program")
        import("core.project.config")
        import("core.project.project")

        -- Find required LLVM tools (try xcrun on macOS first)
        local llvm_profdata = find_program("llvm-profdata")
        local llvm_cov = find_program("llvm-cov")
        local use_xcrun = false

        -- On macOS, try xcrun if direct lookup fails
        if is_plat("macosx") then
          local xcrun = find_program("xcrun")
          if xcrun and (not llvm_profdata or not llvm_cov) then
            use_xcrun = true
            llvm_profdata = xcrun
            llvm_cov = xcrun
          end
        end

        if not llvm_profdata then
          raise("llvm-profdata not found. Please install LLVM toolchain.")
        end

        if not llvm_cov then
          raise("llvm-cov not found. Please install LLVM toolchain.")
        end

        local coverage_dir = path.join(os.projectdir(), "coverage")
        local merged_dir = path.join(coverage_dir, "merged")
        local profdata_file = path.join(coverage_dir, "coverage.profdata")
        local lcov_file = path.join(merged_dir, "coverage.lcov")

        -- Ensure output directories exist
        os.mkdir(coverage_dir)
        os.mkdir(merged_dir)

        -- Find all .profraw files generated by tests
        local profraw_files = os.files(path.join(coverage_dir, "*.profraw"))

        if #profraw_files == 0 then
          raise("No .profraw files found. Run tests first with coverage mode enabled.")
        end

        -- Merge all .profraw files into a single .profdata file
        print("Merging %d profile files...", #profraw_files)
        local merge_args = {}
        if use_xcrun then
          table.insert(merge_args, "llvm-profdata")
        end
        table.insert(merge_args, "merge")
        table.insert(merge_args, "-sparse")
        for _, f in ipairs(profraw_files) do
          table.insert(merge_args, f)
        end
        table.insert(merge_args, "-o")
        table.insert(merge_args, profdata_file)
        os.execv(llvm_profdata, merge_args)

        -- Collect all test binaries for coverage export
        local test_binaries = {}
        for _, t in pairs(project.targets()) do
          if t:get("group") and t:get("group"):startswith("test_") then
            local targetfile = t:targetfile()
            if targetfile and os.isfile(targetfile) then
              table.insert(test_binaries, targetfile)
            end
          end
        end

        if #test_binaries == 0 then
          raise("No test binaries found.")
        end

        -- Generate lcov format report
        print("Generating lcov coverage report...")
        local export_args = {}
        if use_xcrun then
          table.insert(export_args, "llvm-cov")
        end
        table.insert(export_args, "export")
        table.insert(export_args, test_binaries[1])
        for i = 2, #test_binaries do
          table.insert(export_args, "-object")
          table.insert(export_args, test_binaries[i])
        end
        table.insert(export_args, "-instr-profile=" .. profdata_file)
        table.insert(export_args, "-format=lcov")
        table.insert(export_args, "-ignore-filename-regex=.*/tests/.*")
        table.insert(export_args, "-ignore-filename-regex=.*/Utils/.*")
        table.insert(export_args, "-ignore-filename-regex=.*/build/.*")

        local lcov_content = os.iorunv(llvm_cov, export_args)
        io.writefile(lcov_file, lcov_content)

        -- Also generate HTML report
        print("Generating HTML coverage report...")
        local html_dir = path.join(merged_dir, "html")
        os.mkdir(html_dir)
        local show_args = {}
        if use_xcrun then
          table.insert(show_args, "llvm-cov")
        end
        table.insert(show_args, "show")
        table.insert(show_args, test_binaries[1])
        for i = 2, #test_binaries do
          table.insert(show_args, "-object")
          table.insert(show_args, test_binaries[i])
        end
        table.insert(show_args, "-instr-profile=" .. profdata_file)
        table.insert(show_args, "-format=html")
        table.insert(show_args, "-output-dir=" .. html_dir)
        table.insert(show_args, "-ignore-filename-regex=.*/tests/.*")
        table.insert(show_args, "-ignore-filename-regex=.*/Utils/.*")
        table.insert(show_args, "-ignore-filename-regex=.*/build/.*")
        os.execv(llvm_cov, show_args)

        print("Coverage report generated:")
        print("  lcov: %s", lcov_file)
        print("  HTML: %s", path.join(html_dir, "index.html"))
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

        -- Remove profraw files from build directory
        local profraw_files = os.files(path.join(config.builddir(), "**.profraw"))
        for _, file in ipairs(profraw_files) do
          os.rm(file)
        end

        print("Coverage data cleaned")
      end)
    target_end()
  end
end
