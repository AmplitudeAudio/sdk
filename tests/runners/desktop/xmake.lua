-- Copyright (c) 2026-present Sparky Studios. All rights reserved.
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

-- Desktop Test Runner
-- This target builds a CLI test runner for desktop platforms (Windows, Linux, macOS).
-- It uses the TestRegistry for test discovery and supports filtering, grouping, and verbose output.
--
-- Usage:
--   xmake build amplitude_tests
--   ./amplitude_tests -l                    # List all tests
--   ./amplitude_tests -v                    # Run all tests with verbose output
--   ./amplitude_tests -g core_engine        # Run tests in core_engine group
--   ./amplitude_tests --filter memory       # Run tests containing 'memory'

-- Test common library (object library for linking)
target("common_test_static")
    set_kind("object")
    set_policy("build.fence", true)

    add_deps("Amplitude::Static", "build_sample_project", "generate_test_package", "ampk")

    -- Platform-specific source files
    add_files("$(projectdir)/tests/common/TestRegistry.cpp")
    add_files("$(projectdir)/tests/common/TestUtils.cpp")
    add_files("$(projectdir)/tests/common/PlatformTestCase_Desktop.cpp")

    add_includedirs("$(projectdir)/tests/common", { public = true })
    add_includedirs("$(projectdir)/src", { public = true })
    add_includedirs("$(builddir)/include", { public = true })
target_end()

target("common_test_shared")
    set_kind("object")
    set_policy("build.fence", true)

    add_deps("Amplitude::Shared", "build_sample_project", "generate_test_package", "ampk")

    -- Platform-specific source files
    add_files("$(projectdir)/tests/common/TestRegistry.cpp")
    add_files("$(projectdir)/tests/common/TestUtils.cpp")
    add_files("$(projectdir)/tests/common/PlatformTestCase_Desktop.cpp")

    add_includedirs("$(projectdir)/tests/common", { public = true })
    add_includedirs("$(projectdir)/src", { public = true })
    add_includedirs("$(builddir)/include", { public = true })
target_end()

-- Unified desktop test runner
target("amplitude_tests")
    set_kind("binary")
    set_default(false)
    set_group("test_runner")
    set_targetdir("$(builddir)/$(plat)/$(arch)/$(mode)/static")
    set_rundir("$(builddir)")

    add_deps("common_test_static")

    -- All test files (they self-register via static initialization)
    add_files("$(projectdir)/tests/ambisonics_*/test_*.cpp")
    add_files("$(projectdir)/tests/core_*/test_*.cpp")
    add_files("$(projectdir)/tests/dsp_*/test_*.cpp")
    add_files("$(projectdir)/tests/fs_*/test_*.cpp")
    add_files("$(projectdir)/tests/hrtf_*/test_*.cpp")
    add_files("$(projectdir)/tests/math_*/test_*.cpp")
    add_files("$(projectdir)/tests/mixer_*/test_*.cpp")
    add_files("$(projectdir)/tests/threading_*/test_*.cpp")

    -- Runner main
    add_files("main.cpp")

    -- Exclude shared library tests (they need different linking)
    remove_files("$(projectdir)/tests/**/*__shared.cpp")

    add_tests("all", {run_timeout = 120000})

    on_test(function (target, opt)
        import("core.project.config")

        local project_dir = os.projectdir()
        local target_file = path.join(project_dir, target:targetfile())
        local ok, err

        os.cd("$(builddir)")

        if is_mode("coverage") then
            local coverage_dir = path.join(project_dir, "coverage")
            os.mkdir(coverage_dir)
            local profraw_file = path.join(coverage_dir, target:name() .. "-%p.profraw")
            os.setenv("LLVM_PROFILE_FILE", profraw_file)
        end

        ok, err = os.execv(target_file, {"-v"})

        if ok == 0 then
            return true
        end

        return false, err
    end)
target_end()
