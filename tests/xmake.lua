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

  add_deps("ampk")

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

target("common_test")
  set_kind("object")
  set_policy("build.fence", true)

  add_deps("Amplitude::Static", "build_sample_project", "generate_test_package", "ampk")

  add_files("common/*.cpp")

  add_includedirs("common", { public = true })
  add_includedirs("$(projectdir)/src", { public = true })
  add_includedirs("$(builddir)/include", { public = true })
target_end()

for _, filepath in ipairs(os.filedirs("**")) do
  if os.isfile(filepath) or filepath == "common" then
    -- Skip files
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

      add_deps("common_test")

      add_files(test_file)

      add_tests("test")
    target_end()
  end

  ::continue::
end
