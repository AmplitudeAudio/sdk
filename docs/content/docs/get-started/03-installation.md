---
title: Installation
description: Step by step process to install Amplitude in your machine and make the SDK available for your projects.
menu:
  docs:
    parent: get-started
weight: 103
toc: true
---

Follow these steps to install Amplitude Audio SDK from official releases or by building from sources.

## Get the SDK from releases

{{< alert context="warning">}}
Amplitude Audio SDK is still in beta, so no official release is available yet. Please follow the **[Build the SDK from sources](#build-the-sdk-from-sources)** process instead.
{{< /alert >}}

## Build the SDK from sources

{{< callout context="caution" title="Caution" icon="alert-triangle" >}}
Amplitude is in active development, so the installation process here may be outdated. If you are facing issues, we encourage you to [join our Discord server](https://discord.gg/QR2uBpzJ5f) and ask for support.
{{< /callout >}}

To build Amplitude from sources, you will need to first install some dependencies:

- [CMake](https://cmake.org) - CMake is the build system used by the SDK. It will help you generate build files for your platform.
- [vcpkg](https://vcpkg.io) - vcpkg is a free C/C++ package manager that Amplitude uses for acquiring and managing libraries on which it depends.
- [flatc](https://google.github.io/flatbuffers/) (optional) - `flatc` is the compiler of the Flatbuffers serialization format used by Amplitude to generate assets. This is only needed at build time if you are going to compile Amplitude with the provided samples.

Once you have installed dependencies you can clone the repository from GitHub using `git` as follows:

```bash
git clone https://github.com/SparkyStudios/AmplitudeAudioSDK.git
```

This will create a directory named `AmplitudeAudioSDK` at the location you cloned the repository. Enter that directory and use CMake to generate build files for your system:

```bash
cd AmplitudeAudioSDK
cmake -DCMAKE_TOOLCHAIN_FILE:STRING=~/vcpkg/scripts/buildsystems/vcpkg.cmake  -B ./build
```

Windows users may generate build files using these commands:

```bash
cd AmplitudeAudioSDK
cmake -DCMAKE_TOOLCHAIN_FILE:STRING=C:/vcpkg/scripts/buildsystems/vcpkg.cmake  -B .\build
```

You may need to change the `CMAKE_TOOLCHAIN_FILE` CMake variable to point to your installation of vcpkg. Feel free to customize the command as needed, to include for example custom generators like `Ninja`.

{{< details "Build with samples" >}}
You can optionally build the SDK with the provided samples. Note that by enabling the samples, the sources will depend on `SDL2`, which will be automatically fetched by vcpkg for you.

To enable the samples, add `-DBUILD_SAMPLES:BOOL=TRUE` to the previous CMake command.
{{< /details >}}

Once the generation is done, you can build the SDK with the following command:

```bash
cmake --build build
```

And then install it with:

```bash
cmake --build build --target install
```

The SDK installs itself in the same folder as the sources, in the `sdk` directory.

## Install the SDK

Once you have a copy of the SDK, to install it you just need to add a `AM_SDK_PATH` environment variable in your system, that points to the path of the `sdk` directory.

{{< alert context="info" text="By installing the SDK through the upcoming Amplitude Studio, the environment variable will be automatically set." />}}

That's it! You can now use Amplitude Audio SDK in your projects!
