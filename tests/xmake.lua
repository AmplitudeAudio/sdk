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

    target(target_name)
        set_kind("binary")
        set_default(false)
        set_group(group)
        set_rundir("$(builddir)")

        add_deps("Amplitude", "build_sample_project")

        add_includedirs("common")
        add_includedirs("$(projectdir)/src")
        add_includedirs("$(builddir)/include")

        add_files("common/*.cpp")

        for _, test_file in ipairs(os.files(name .. "/test_*.cpp")) do
            local test_name = path.basename(test_file):gsub("^test_", "")
            add_tests(test_name, {files = test_file})
        end
    target_end()

    ::continue::
end
