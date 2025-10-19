#!/usr/bin/python
# Copyright (c) 2021-present Sparky Studios. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Builds all Amplitude flatbuffer schemas to binary modules.

Generates binary schema files (.bfbs) bundled in the SDK release.
"""

import common
import glob
import os
import sys


# The project root directory, which is one level up from this script's
# directory.
PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), os.path.pardir))


def compile_flatbuffer_binary_schema(flatc: str, schema: str, out_dir: str) -> None:
    """Run the flatbuffer compiler on the given schema file.

    Args:
      flatc: Path to the flatc binary.
      schema: The path to the schema.
      out_dir: The directory to write the flatbuffer binary schema.

    Raises:
      BuildError: Process return code was nonzero.
    """

    command = [flatc, "-o", out_dir]
    command.extend(["-I", os.path.join(PROJECT_ROOT, "schemas")])
    command.extend(["-b", schema, "--schema"])
    common.run_subprocess(command)


def generate_flatbuffer_binaries(flatc: str, target_directory: str) -> None:
    """Run the flatbuffer compiler on the all the flatbuffer schema files.

    Args:
      flatc: Path to the flatc binary.
      target_directory: Path to the target assets directory.
    """

    for schema in glob.glob(
        os.path.join(PROJECT_ROOT, "schemas", "*.fbs"), recursive=False
    ):
        compile_flatbuffer_binary_schema(flatc, schema, target_directory)


def main(argv: list[str]):
    """Builds binary schemas for Amplitude assets.

    Returns:
      Returns 0 on success.
    """

    options = common.CommandOptions(argv, "build_schemas", "1.0.0")

    try:
        generate_flatbuffer_binaries(options.flatc_path, options.build_path)
        print("Amplitude binary schemas generated successfully.")
    except common.BuildError as error:
        common.handle_build_error(error)
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
