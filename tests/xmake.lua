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

target("generate_test_package")
  set_kind("phony")

  add_deps("ampk", "build_sample_project")

  on_build(function(target)
    import("core.project.config")
    import("core.project.project")
    import("lib.detect.find_tool")

    local ampk = project.target("ampk")

    local program = ampk:targetfile()
    if program then
      local assets_dir = path.join(path.absolute(config.builddir()), "samples/assets")
      local output_uncompressed_dir = path.join(path.absolute(config.builddir()), "samples/assets_uncompressed.ampk")
      local output_compressed_dir = path.join(path.absolute(config.builddir()), "samples/assets_compressed.ampk")

      os.exec("%s -q -c 0 %s %s", program, assets_dir, output_uncompressed_dir)
      os.exec("%s -q -c 1 %s %s", program, assets_dir, output_compressed_dir)
    else
      print("ampk not found.")
    end
  end)
target_end()

target("test_plugin")
  set_kind("shared")
  set_targetdir("$(builddir)/$(plat)/$(arch)/$(mode)/shared")
  add_defines("AM_BUILDSYSTEM_BUILDING_PLUGIN")

  add_deps("Amplitude::Shared")

  add_files("test_plugin/*.cpp")
target_end()

target("common_test_static")
  set_kind("object")
  set_policy("build.fence", true)

  add_deps("Amplitude::Static", "build_sample_project", "generate_test_package", "ampk")

  add_files("common/*.cpp")

  add_includedirs("common", { public = true })
  add_includedirs("$(projectdir)/src", { public = true })
  add_includedirs("$(builddir)/include", { public = true })
target_end()

target("common_test_shared")
  set_kind("object")
  set_policy("build.fence", true)

  add_deps("Amplitude::Shared", "build_sample_project", "generate_test_package", "ampk")

  add_files("common/*.cpp")

  add_includedirs("common", { public = true })
  add_includedirs("$(projectdir)/src", { public = true })
  add_includedirs("$(builddir)/include", { public = true })
target_end()

for _, filepath in ipairs(os.dirs("**")) do
  if filepath == "common" then
    -- Skip common directory
    goto continue
  end

  local name = path.basename(filepath)

  local parts = {}
  local first, rest = name:match("([^_]+)_(.+)")

  if first and rest then
    parts[1] = first
    parts[2] = rest
  else
    parts[1] = name
  end

  local group = parts[1] or name
  local target_name = parts[2] or name

  for _, test_file in ipairs(os.files(name .. "/test_*.cpp")) do
    local test_name = path.basename(test_file):gsub("^test_", "")
    target(target_name .. "_" .. test_name)
      set_kind("binary")
      set_default(false)
      set_group("test_" .. group)
      set_rundir("$(builddir)")

      if path.basename(test_file):sub(-8) == "__shared" then
        set_targetdir("$(builddir)/$(plat)/$(arch)/$(mode)/shared")
        add_deps("common_test_shared")
      else
        set_targetdir("$(builddir)/$(plat)/$(arch)/$(mode)/static")
        add_deps("common_test_static")
      end

      add_files(test_file)

      add_tests("test")

      on_test(function (target, opt)
        import("lib.detect.find_tool")

        local kcov = find_tool("kcov")

        if not kcov then
          return false, "kcov not found. Please install kcov."
        end

        local project_dir = os.projectdir()
        local coverage_dir = path.join(project_dir, "coverage/split_"..target:name())
        local target_file = path.join(project_dir, target:targetfile())
        local src_dir = path.join(project_dir, "src")
        local include_dir = path.join(project_dir, "include")

        os.mkdir(coverage_dir)

        os.cd("$(builddir)")
        local ok, err = os.execv(kcov.program, {
          "--include-path=" .. src_dir .. "," .. include_dir,
          "--exclude-path=" .. path.join(src_dir, "Utils"),
          "--strip-path=" .. project_dir,
          coverage_dir,
          target_file
        })

        if ok == 0 then
            return true
        end

        return false
      end)
    target_end()
  end

  ::continue::
end
