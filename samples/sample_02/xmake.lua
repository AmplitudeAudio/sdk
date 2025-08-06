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

target("sample_02")
  set_kind("binary")
  set_targetdir("$(builddir)/samples")
  set_group("samples")

  -- Link to Amplitude Static library
  add_deps("Amplitude::Shared", "build_sample_project")

  -- Link to SDL2
  add_packages("libsdl2", { components = { "main" } })

  if is_plat("windows") then
    add_ldflags("-subsystem:windows")
  end

  -- Include main project include directories
  add_includedirs("$(projectdir)/src")

  -- Source files
  add_files("main.cpp")
target_end()