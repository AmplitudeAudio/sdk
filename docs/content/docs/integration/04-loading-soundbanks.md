---
title: Loading Soundbanks
description: Soundbanks are units storing the data your game will need to play audio. The page will show you how to load soundbanks from an Amplitude project.
menu:
  docs:
    parent: integration
weight: 304
toc: true
---

When the engine is [fully initialized]({{< relref "03-initializing-the-engine" >}}), you have to load a
[soundbank]({{< relref "../project-setup/13-soundbank" >}}) to be able to play any sound or interact with your project.

By loading a soundbank, the engine will also load all the associated data (environments, effects, attenuation, etc.).

```cpp
AmBankID bankId = kAmInvalidObjectId;

if (!amEngine->LoadSoundBank(AM_OS_STRING("init.ambank"), bankId))
  return 1;
```

At this point, the engine has only preloaded sound objects associated to the soundbank. The process of loading sound files
is done manually, in another thread, so the main thread is ensured to not hang during this time.

During this process, only sound files with streaming enabled will be read from the filesystem, other files will be read
and stored in memory.

```cpp
// Load audio files
amEngine->StartLoadSoundFiles();
while (!m_pEngine->TryFinalizeLoadSoundFiles())
  Thread::Sleep(1);
```

Audio data loaded in memory are shared across every sound instances.
