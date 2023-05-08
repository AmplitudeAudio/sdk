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

"""Builds all assets under samples/rawassets/, writing the results to assets/.

Finds the flatbuffer compiler then uses it to convert the JSON files to
flatbuffer binary files.  If you would like to clean all generated files, you
can call this script with the argument 'clean'.
"""

import argparse
import distutils.spawn
import glob
import os
import platform
import shutil
import subprocess
import sys

# The project root directory, which is one level up from this script's
# directory.
PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__),
                                            os.path.pardir))

# Directories that may contain the FlatBuffers compiler.
FLATBUFFERS_PATHS = [
    os.path.join(PROJECT_ROOT, 'bin'),
    os.path.join(PROJECT_ROOT, 'bin', 'Release'),
    os.path.join(PROJECT_ROOT, 'bin', 'Debug')
]

# Directory to place processed assets.
ASSETS_PATH = os.path.join(PROJECT_ROOT, 'assets')

# Directory where unprocessed assets can be found.
RAW_ASSETS_PATH = os.path.join(PROJECT_ROOT, 'samples', 'rawassets')

# Directory where unprocessed sound flatbuffer data can be found.
RAW_SOUND_PATH = os.path.join(RAW_ASSETS_PATH, 'sounds')

# Directory where unprocessed collection flatbuffer data can be found.
RAW_COLLECTION_PATH = os.path.join(RAW_ASSETS_PATH, 'collections')

# Directory where unprocessed sound bank flatbuffer data can be found.
RAW_SOUND_BANK_PATH = os.path.join(RAW_ASSETS_PATH, 'soundbanks')

# Directory where unprocessed event flatbuffer data can be found.
RAW_EVENT_PATH = os.path.join(RAW_ASSETS_PATH, 'events')

# Directory where unprocessed attenuation flatbuffer data can be found.
RAW_ATTENUATION_PATH = os.path.join(RAW_ASSETS_PATH, 'attenuators')

# Directory where unprocessed switch flatbuffer data can be found.
RAW_SWITCHES_PATH = os.path.join(RAW_ASSETS_PATH, 'switches')

# Directory where unprocessed switch containers flatbuffer data can be found.
RAW_SWITCH_CONTAINERS_PATH = os.path.join(RAW_ASSETS_PATH, 'switch_containers')

# Directory where unprocessed rtpc flatbuffer data can be found.
RAW_RTPC_CONTAINERS_PATH = os.path.join(RAW_ASSETS_PATH, 'rtpc')

# Directory where unprocessed effect flatbuffer data can be found.
RAW_EFFECT_CONTAINERS_PATH = os.path.join(RAW_ASSETS_PATH, 'effects')

# Directory where unprocessed assets can be found.
SCHEMA_PATHS = [os.path.join(PROJECT_ROOT, 'schemas')]

# Windows uses the .exe extension on executables.
EXECUTABLE_EXTENSION = '.exe' if platform.system() == 'Windows' else ''

# Name of the flatbuffer executable.
FLATC_EXECUTABLE_NAME = 'flatc' + EXECUTABLE_EXTENSION


class FlatbuffersConversionData(object):
    """Holds data needed to convert a set of json files to flatbuffer binaries.

    Attributes:
      schema: The path to the flatbuffer schema file.
      input_files: A list of input files to convert.
    """

    def __init__(self, schema, input_files):
        """Initializes this object's schema and input_files."""
        self.schema = schema
        self.input_files = input_files


def find_in_paths(name, paths):
    """Searches for a file with named `name` in the given paths and returns it."""
    for path in paths:
        full_path = os.path.join(path, name)
        if os.path.isfile(full_path):
            return full_path
    # If not found, just assume it's in the PATH.
    return name


# A list of json files and their schemas that will be converted to binary files
# by the flatbuffer compiler.
FLATBUFFERS_CONVERSION_DATA = [
    FlatbuffersConversionData(
        schema=find_in_paths('engine_config_definition.fbs', SCHEMA_PATHS),
        input_files=[os.path.join(RAW_ASSETS_PATH, 'audio_config.json')]),
    FlatbuffersConversionData(
        schema=find_in_paths('buses_definition.fbs', SCHEMA_PATHS),
        input_files=[os.path.join(RAW_ASSETS_PATH, 'buses.json')]),
    FlatbuffersConversionData(
        schema=find_in_paths('sound_bank_definition.fbs', SCHEMA_PATHS),
        input_files=glob.glob(os.path.join(RAW_SOUND_BANK_PATH, '**/*.json'), recursive=True)),
    FlatbuffersConversionData(
        schema=find_in_paths('collection_definition.fbs', SCHEMA_PATHS),
        input_files=glob.glob(os.path.join(RAW_COLLECTION_PATH, '**/*.json'), recursive=True)),
    FlatbuffersConversionData(
        schema=find_in_paths('sound_definition.fbs', SCHEMA_PATHS),
        input_files=glob.glob(os.path.join(RAW_SOUND_PATH, '**/*.json'), recursive=True)),
    FlatbuffersConversionData(
        schema=find_in_paths('event_definition.fbs', SCHEMA_PATHS),
        input_files=glob.glob(os.path.join(RAW_EVENT_PATH, '**/*.json'), recursive=True)),
    FlatbuffersConversionData(
        schema=find_in_paths('attenuation_definition.fbs', SCHEMA_PATHS),
        input_files=glob.glob(os.path.join(RAW_ATTENUATION_PATH, '**/*.json'), recursive=True)),
    FlatbuffersConversionData(
        schema=find_in_paths('switch_definition.fbs', SCHEMA_PATHS),
        input_files=glob.glob(os.path.join(RAW_SWITCHES_PATH, '**/*.json'), recursive=True)),
    FlatbuffersConversionData(
        schema=find_in_paths('switch_container_definition.fbs', SCHEMA_PATHS),
        input_files=glob.glob(os.path.join(RAW_SWITCH_CONTAINERS_PATH, '**/*.json'), recursive=True)),
    FlatbuffersConversionData(
        schema=find_in_paths('rtpc_definition.fbs', SCHEMA_PATHS),
        input_files=glob.glob(os.path.join(RAW_RTPC_CONTAINERS_PATH, '**/*.json'), recursive=True)),
    FlatbuffersConversionData(
        schema=find_in_paths('effect_definition.fbs', SCHEMA_PATHS),
        input_files=glob.glob(os.path.join(RAW_EFFECT_CONTAINERS_PATH, '**/*.json'), recursive=True)),
]

# Location of FlatBuffers compiler.
FLATC = find_in_paths(FLATC_EXECUTABLE_NAME, FLATBUFFERS_PATHS)


class BuildError(Exception):
    """Error indicating there was a problem building assets."""

    def __init__(self, argv, error_code, message=None):
        Exception.__init__(self)
        self.argv = argv
        self.error_code = error_code
        self.message = message if message else ''


def run_subprocess(argv):
    try:
        process = subprocess.Popen(argv)
    except OSError as e:
        raise BuildError(argv, 1, message=str(e))
    process.wait()
    if process.returncode:
        raise BuildError(argv, process.returncode)


def convert_json_to_flatbuffer_binary(flatc, json, schema, out_dir):
    """Run the flatbuffer compiler on the given json file and schema.

    Args:
      flatc: Path to the flatc binary.
      json: The path to the json file to convert to a flatbuffer binary.
      schema: The path to the schema to use in the conversion process.
      out_dir: The directory to write the flatbuffer binary.

    Raises:
      BuildError: Process return code was nonzero.
    """
    command = [flatc, '-o', out_dir]
    for path in SCHEMA_PATHS:
        command.extend(['-I', path])
    command.extend(['-b', schema, json])
    run_subprocess(command)


def needs_rebuild(source, target):
    """Checks if the source file needs to be rebuilt.

    Args:
      source: The source file to be compared.
      target: The target file which we may need to rebuild.

    Returns:
      True if the source file is newer than the target, or if the target file
      does not exist.
    """
    return not os.path.isfile(target) or (
            os.path.getmtime(source) > os.path.getmtime(target))


def processed_json_path(path, target_directory):
    """Take the path to a raw json asset and convert it to target bin path.

    Args:
      target_directory: Path to the target assets directory.
    """
    return path.replace(RAW_ASSETS_PATH, target_directory).replace(
        '.json', '.bin')


def generate_flatbuffer_binaries(flatc, target_directory):
    """Run the flatbuffer compiler on the all of the flatbuffer json files.

    Args:
      flatc: Path to the flatc binary.
      target_directory: Path to the target assets directory.
    """
    for element in FLATBUFFERS_CONVERSION_DATA:
        schema = element.schema
        for json in element.input_files:
            target = processed_json_path(json, target_directory)
            target_file_dir = os.path.dirname(target)
            if not os.path.exists(target_file_dir):
                os.makedirs(target_file_dir)
            if needs_rebuild(json, target) or needs_rebuild(schema, target):
                convert_json_to_flatbuffer_binary(flatc, json, schema, target_file_dir)


def copy_assets(target_directory):
    """Copy modified assets to the target assets directory.

    All files are copied from ASSETS_PATH to the specified target_directory if
    they're newer than the destination files.

    Args:
      target_directory: Directory to copy assets to.
    """
    assets_dir = target_directory
    source_dir = os.path.realpath(ASSETS_PATH)
    if source_dir != os.path.realpath(assets_dir):
        for dirpath, _, files in os.walk(source_dir):
            for name in files:
                source_filename = os.path.join(dirpath, name)
                relative_source_dir = os.path.relpath(source_filename, source_dir)
                target_dir = os.path.dirname(os.path.join(assets_dir,
                                                          relative_source_dir))
                target_filename = os.path.join(target_dir, name)
                if not os.path.exists(target_dir):
                    os.makedirs(target_dir)
                if (not os.path.exists(target_filename) or
                        (os.path.getmtime(target_filename) <
                         os.path.getmtime(source_filename))):
                    shutil.copy2(source_filename, target_filename)


def clean_flatbuffer_binaries(target_directory):
    """Delete all the processed flatbuffer binaries.

    Args:
      target_directory: Path to the target assets directory.
    """
    for element in FLATBUFFERS_CONVERSION_DATA:
        for json in element.input_files:
            path = processed_json_path(json, target_directory)
            if os.path.isfile(path):
                os.remove(path)


def clean():
    """Delete all the processed files."""
    clean_flatbuffer_binaries()



def handle_build_error(error):
    """Prints an error message to stderr for BuildErrors."""
    sys.stderr.write('Error running command `%s`. Returned %s.\n%s\n' % (
        ' '.join(error.argv), str(error.error_code), str(error.message)))


def main(argv):
    """Builds or cleans the assets needed for the game.

    This script will build all the assets in samples/rawassets.  To clean all
    converted files, call it with 'clean'.

    Returns:
      Returns 0 on success.
    """
    parser = argparse.ArgumentParser()
    parser.add_argument('--flatc', default=FLATC,
                        help='Location of the flatbuffers compiler.')
    parser.add_argument('--output', default=ASSETS_PATH,
                        help='Assets output directory.')
    parser.add_argument('args', nargs=argparse.REMAINDER)
    args = parser.parse_args()
    target = args.args[1] if len(args.args) >= 2 else 'all'
    if target != 'clean':
        copy_assets(args.output)
        try:
            generate_flatbuffer_binaries(args.flatc, args.output)
        except BuildError as error:
            handle_build_error(error)
            return 1
    else:
        try:
            clean()
        except OSError as error:
            sys.stderr.write('Error cleaning: %s' % str(error))
            return 1
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv))
