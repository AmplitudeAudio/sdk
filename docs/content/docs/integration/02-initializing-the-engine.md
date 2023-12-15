---
title: Initializing the Engine
description: Learn how to initialize the Amplitude Engine with a given project at runtime through this tutorial.
menu:
  docs:
    parent: integration
weight: 302
toc: true
---

{{< callout context="note" title="Note" icon="info-circle" >}}
You will need to have an Amplitude project to follow these instructions. Please read the
[project setup]({{< relref "../project-setup/01-project-architecture" >}}) documentation to create a project. You can
also get the [sample project](https://github.com/SparkyStudios/AmplitudeAudioSDK/tree/develop/sample_project) provided
in the GitHub repository.
{{< /callout >}}

Amplitude is built with several different components, which should be initialized separately before to start playing
audio.

## Logger

Amplitudes comes with a very simple logger that you can customize to output messages in the console, a file, or any
other output that suits your needs.

```cpp
// Create your custom logger function, may be something similar to this...
static void my_logger(const char* fmt, va_list args)
{
#if defined(AM_WCHAR_SUPPORTED)
    vfwprintf(stdout, AM_STRING_TO_OS_STRING(fmt), args);
#else
    vfprintf(stdout, fmt, args);
#endif
}

// Register your logger to Amplitude
RegisterLogFunc(my_logger);
```

Initializing the logger is optional, as it is not a required component. But if you want to use it, it is usually better
to initialize it first, as it is used in every part of the SDK.

## MemoryManager

The memory manager is the first **required** component to initialize. It is responsible for all the allocations
occurring in the SDK (even the ones due to other components initialization), and in your application.

While initializing the memory manager, you can customize the allocation functions through the `MemoryManagerConfig`
structure.

A typical memory manager initialization code will look like this:

```cpp
#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

using namespace SparkyStudios::Audio::Amplitude;

int main(int argc, char* argv[])
{
  MemoryManager::Initialize(MemoryManagerConfig()); // Using the default memory manager configuration.

  // ... your code ...

  MemoryManager::Deinitialize();

  return 0;
}
```

## FileSystem

The filesystem component is responsible to read/write resources as needed by the SDK. Amplitude comes shipped with a
`DiskFileSystem` implementation allowing you to access files on disk. You can easily create your own filesystem
implementation to fit your needs (eg: accessing assets from an archive or a package).

The used filesystem implementation should be set as the default one in the engine after the initialization. For
example, if you use the `DiskFileSystem` implementation, a typical usage will look like:

```cpp
DiskFileSystem fs;

fs.SetBasePath(AM_OS_STRING("./my_project")); // Set the base path of the filesystem. Usually the path to your Amplitude project.
amEngine->SetFileSystem(&fs); // Set the filesystem implementation to use in the engine.
```

According to the implementation, the filesystem may be opened in a background thread (eg: unpacking an archive). If
it's the case for you, it is necessary to wait for the filesystem to load before to continue. You can do this using the
following code:

```cpp
// Wait for the filesystem to complete loading.
amEngine->StartOpenFileSystem();
while (!amEngine->TryFinalizeOpenFileSystem()) // While the filesystem is still loading
    Thread::Sleep(1); // Wait for the filesystem to load
```

## Plugins

Plugins are external dynamic libraries that allows you to extend the functionalities of the engine (eg: adding codecs,
filters, faders, etc...), and therefore, they should be loaded before the engine itself is initialized.

The SDK allows you to set the paths in which to search for plugins:

```cpp
// The path is relative to the working directory, which is usually the same path as the executable.
Engine::AddPluginSearchPath(AM_OS_STRING("./my_project/plugins"));
```

{{< alert context="info" >}}
By default, the engine will search first in the working directory **before** to look in the search paths.
{{< /alert >}}

Once the search paths have been set, the engine can now load your plugins:

```cpp
Engine::LoadPlugin(AM_OS_STRING("AmplitudeVorbisCodecPlugin")); // Official plugin for Vorbis/OGG codec
Engine::LoadPlugin(AM_OS_STRING("MyCustomPlugin")); // Any other awesome plugin you will build
```

{{< alert context="info" >}}
Note that the plugin is loaded using his canonical name, without prefix (eg: `lib` on UNIX platforms) and without
extension (eg: `.dll` on Windows platforms). Amplitude will add them for you automatically.
{{< /alert >}}

## Amplitude Engine

The Amplitude engine is initialized with a specific configuration, from the ones available in you Amplitude project:

```cpp
// The path to the configuration file is relative to the base path of the filesystem
if (!amEngine->Initialize(AM_OS_STRING("./audio_config.amconfig")))
  return 1; // There is usually nothing more to do if the engine is not initialized...
```

Once the engine is initialized, you can start to interact wit audio files and your loaded Amplitude project.

## Wrapping Up

A full example of the SDK initialization may look like this:

```cpp
#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

using namespace SparkyStudios::Audio::Amplitude;

// Create your custom logger function, may be something similar to this...
static void my_logger(const char* fmt, va_list args)
{
#if defined(AM_WCHAR_SUPPORTED)
    vfwprintf(stdout, AM_STRING_TO_OS_STRING(fmt), args);
#else
    vfprintf(stdout, fmt, args);
#endif
}

int main(int argc, char* argv[])
{
  // Register your logger to Amplitude
  RegisterLogFunc(my_logger);

  // Initialize the memory manager
  MemoryManager::Initialize(MemoryManagerConfig()); // Using the default memory manager configuration.

  DiskFileSystem fs;

  fs.SetBasePath(AM_OS_STRING("./my_project")); // Set the base path of the filesystem. Usually the path to your Amplitude project.
  amEngine->SetFileSystem(&fs); // Set the filesystem implementation to use in the engine.

  // Wait for the filesystem to complete loading.
  amEngine->StartOpenFileSystem();
  while (!amEngine->TryFinalizeOpenFileSystem()) // While the filesystem is still loading
      Thread::Sleep(1); // Wait for the filesystem to load

  // The path is relative to the working directory, which is usually the same path as the executable.
  Engine::AddPluginSearchPath(AM_OS_STRING("./my_project/plugins"));

  Engine::LoadPlugin(AM_OS_STRING("AmplitudeVorbisCodecPlugin")); // Official plugin for Vorbis/OGG codec
  Engine::LoadPlugin(AM_OS_STRING("MyCustomPlugin")); // Any other awesome plugin you will build

  // The path to the configuration file is relative to the base path of the filesystem
  if (amEngine->Initialize(AM_OS_STRING("./audio_config.amconfig")))
  {
    // ... Can now play audio files and access project data ...

    // Deinitialize the Amplitude engine
    amEngine->Deinitialize();
  }

  // Wait for the file system to complete unloading.
  amEngine->StartCloseFileSystem();
  while (!amEngine->TryFinalizeCloseFileSystem())
    Thread::Sleep(1);

  // Destroy the Amplitude engine instance
  amEngine->DestroyInstance();

  // Deinitialize the memory manager
  MemoryManager::Deinitialize();

  return 0;
}
```
