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

target("ampm")
  set_kind("binary")
  set_targetdir("$(builddir)/bin")
  set_group("tools")

  -- Link to Amplitude Static library
  add_deps("Amplitude::Static")

  -- Add packages
  add_packages("cli11")

  -- Include main project include directories
  add_includedirs("$(projectdir)/src", "$(projectdir)/tools/common", "$(builddir)/include")

  -- Source files
  add_files("main.cpp")

  on_load(function(target)
    -- Generate resources.cpp and resources.h files with embedded binary content
    local resources_dir = path.join(target:scriptdir(), "resources")
    local resources_file = path.join(target:scriptdir(), "gen.resources.cpp")
    local resources_header = path.join(target:scriptdir(), "gen.resources.h")

    if os.isdir(resources_dir) then
      local cpp_content = { "#include \"gen.resources.h\"", "" }
      local header_content = { "#ifndef GEN_RESOURCES_H", "#define GEN_RESOURCES_H", "", "#include <cstddef>", "#include <cstdint>",
        "" }

      -- Find all files in resources directory
      local files = os.files(path.join(resources_dir, "**"))

      for _, filepath in ipairs(files) do
        local filename = path.filename(filepath)
        local varname = "resource_" .. filename:gsub("[^%w]", "_")

        -- Read file as binary
        local file_content = io.readfile(filepath, { encoding = "binary" })
        if file_content then
          local bytes = {}
          for i = 1, #file_content do
            table.insert(bytes, string.format("0x%02x", string.byte(file_content, i)))
          end

          -- Add to cpp file
          table.insert(cpp_content, "// Embedded resource: " .. filename)
          table.insert(cpp_content, "const unsigned char " .. varname .. "_data[] = {")
          table.insert(cpp_content, "  " .. table.concat(bytes, ", "))
          table.insert(cpp_content, "};")
          table.insert(cpp_content, "const size_t " .. varname .. "_size = " .. #file_content .. ";")
          table.insert(cpp_content, "")

          -- Add declarations to header file
          table.insert(header_content, "// Embedded resource: " .. filename)
          table.insert(header_content, "extern const unsigned char " .. varname .. "_data[];")
          table.insert(header_content, "extern const size_t " .. varname .. "_size;")
          table.insert(header_content, "")
        end
      end

      -- Close header guard
      table.insert(header_content, "#endif // RESOURCES_H")

      -- Write the generated content to files
      io.writefile(resources_file, table.concat(cpp_content, "\n"))
      io.writefile(resources_header, table.concat(header_content, "\n"))

      -- Add the generated files to the target
      target:add("files", resources_file)
    end
  end)

  on_clean(function (target)
    local resources_file = path.join(target:scriptdir(), "gen.resources.cpp")
    local resources_header = path.join(target:scriptdir(), "gen.resources.h")

    -- Delete generated resource files
    os.rm(resources_file)
    os.rm(resources_header)
  end)
target_end()
