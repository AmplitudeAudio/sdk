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

target("ampk")
  set_kind("binary")
  set_targetdir("$(builddir)/bin")
  set_group("tools")

  -- Link to Amplitude Static library
  add_deps("Amplitude::Static")

  -- Add packages
  add_packages("cli11", "lz4")

  -- Include main project include directories
  add_includedirs("$(projectdir)/src", "$(projectdir)/tools/common", "$(builddir)/include")

  -- Source files
  add_files("main.cpp")
target_end()
