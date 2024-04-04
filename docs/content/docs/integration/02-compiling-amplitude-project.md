---
title: Compiling Amplitude Projects
description: Before to use the engine in your, you will need to compile your Amplitude project into binaries.
menu:
  docs:
    parent: integration
weight: 302
---

As explained in the [project architecture]({{< relref "../project-setup/01-project-architecture.md" >}}) documentation,
an Amplitude project in mainly based on JSON files. But when it's time to integrate the engine with your game, it's
required to compile these JSON files into binaries, through the `flatc` compiler.

The SDK comes with python scripts you can use to build an entire project, or clean any previously compiled binaries.
To use them you will need to download the flatbuffers compiler (`flatc`) corresponding to your platform, and make sure
[Python](https://python.org) is installed and accessible through your `PATH` environment variable.

## Building a project

The `build_project.py` script is used to build an Amplitude project. The script can take the following command line
arguments:

- `-f`, `--flatc`: The path to the flatc executable. This argument is optional and if not provided, the script will look
in the `PATH` environment variable and in the `bin` directory of your Amplitude Audio SDK installation.
- `-p`, `--project-path`: The path to the Amplitude project. You should specify the full path to the directory containing
Amplitude assets. This argument is required.
- `-b`, `--build-path`: The path to the directory where built files will be created. The build directory structure will
be the same as the project directory. If the given path doesn't exist it will be automatically created. This argument is
required.

```bash
python3 $AM_SDK_PATH/scripts/build_project.py -p "/path/to/amplitude/project" -b "/path/to/build"
```

## Cleaning a build

You may want to clean all the build files generated in the previous step. For that purpose, the SDK comes with a `clean_project.py`
script. Its usage is the same as the `build_project.py` script, except that it's not needed to provide the path to the
`flatc` executable.

```bash
python3 $AM_SDK_PATH/scripts/clean_project.py -p "/path/to/amplitude/project" -b "/path/to/build"
```

## Additional command line arguments

Both `build_project.py` and `clean_project.py` scripts provides the following additional arguments:

- `-h`, `--help`: Displays help/documentation about the usage of the running script.
- `--no-logo`: Used to not display the copyright header.
- `-v`, `--version`: Shows the version of the running script.
