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

target("amplitude_fidelity_lib")
  set_kind("static")
  set_group("tools")

  add_deps("Amplitude::Static")

  add_includedirs("$(projectdir)/tools/fidelity/src", { public = true })
  add_includedirs("$(projectdir)/src", "$(builddir)/include")

  add_files("src/**.cpp")
target_end()

target("amplitude_fidelity")
  set_kind("binary")
  set_targetdir("$(builddir)/bin")
  set_group("tools")
  set_rundir("$(builddir)")

  add_deps("amplitude_fidelity_lib")
  add_packages("cli11")

  add_includedirs("$(projectdir)/src", "$(builddir)/include")

  add_files("main.cpp")
target_end()

if has_config("build_assets") then
  target("build_fidelity_project")
    set_kind("phony")

    add_deps("amplitude_fidelity", "build_binary_schemas")

    on_build(function (target)
      import("core.project.config")
      import("core.project.project")
      import("lib.detect.find_program")
      import("lib.detect.find_tool")

      local python = find_program("python3") or find_program("python")
      if not python then
        raise("Python not found. Cannot build the fidelity project.")
      end

      local flatc = find_tool("flatc", { paths = { "$(env PATH)", "$(projectdir)/bin" } })
      if not flatc then
        raise("flatc not found. Cannot build the fidelity project.")
      end

      local build_dir = path.absolute(config.builddir())
      local project_dir = path.join(build_dir, "fidelity", "project")
      local assets_dir = path.join(build_dir, "fidelity", "assets")
      local source_dir = path.join(os.projectdir(), "tools", "fidelity", "project")

      os.mkdir(project_dir)
      os.mkdir(assets_dir)
      os.cp(path.join(source_dir, "**"), project_dir, { rootdir = source_dir })

      local tool = path.absolute(project.target("amplitude_fidelity"):targetfile())
      os.execv(tool, { "--generate-assets", "--project", project_dir, "--assets", assets_dir })

      os.exec("%s %s -p %s -b %s -f %s -s %s", python, path.join(os.projectdir(), "scripts", "build_project.py"),
        project_dir, assets_dir, flatc.program, path.join(os.projectdir(), "schemas"))
    end)
  target_end()
end
