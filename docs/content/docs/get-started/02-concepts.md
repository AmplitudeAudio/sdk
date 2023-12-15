---
title: Concepts
description: Learn the different elements and terms used in the SDK, and see how to bring them together in your project. Start here if you are new to Amplitude.
menu:
  docs:
    parent: get-started
weight: 102
toc: true
mermaid: true
---

For new users of Amplitude, it's important to know the different elements which are part of the library and to understand how they are interacting all together.

{{< mermaid >}}
%%{init: {'theme': 'base', 'themeVariables': { 'primaryColor': '#74D0F1', 'fontFamily': 'Open Sans' }}}%%
flowchart TB
  subgraph sources[Audio Files]
    source1(gun_fire_01.ams)
  end
  subgraph codecs[Audio Codecs]
    codec1[[Codec]]
    codec2[[Codec]]
    codec3[[Codec]]
  end
  subgraph game[Game]
    switch1[[SwitchState]]
    switch2[[SwitchState]]
    rtpc[[RTPC]]
    entity[[Entity]]
    listener[[Listener]]
  end
  subgraph sounds[Sound Objects]
    direction LR
    sound1[[Sound]]
    subgraph sound_objects[Containers]
      sound_object1[[Collection]]
      sound_object2[[SwitchContainer]]
    end
  end
  subgraph channels[Channels]
    channel1[[Channel]]
    channel2[[Channel]]
    channel3[[Channel]]
  end
  subgraph sound_instances[Sound Instances]
    sound_instance1[[SoundInstance]]
    sound_instance2[[SoundInstance]]
    sound_instance3[[SoundInstance]]
    sound_instance4[[SoundInstance]]
    sound_instance5[[SoundInstance]]
    sound_instance6[[SoundInstance]]
  end
  subgraph buses[Buses]
    bus1[[Bus]]
    bus2[[Bus]]
  end
  subgraph amplimix[Amplimix]
    mixer_layer1[[Mixer Layer]]
    mixer_layer2[[Mixer Layer]]
    mixer_layer3[[Mixer Layer]]
    mixer_layer4[[Mixer Layer]]
    mixer_layer5[[Mixer Layer]]
    mixer_layer6[[Mixer Layer]]
    mixer[[Mixer]]
    pipeline[[Pipeline]]
  end
  subgraph drivers[Audio Drivers]
    driver1[[Driver]]
    driver2[[Driver]]
    driver3[[Driver]]
  end
  source1 -.-> codec1
  codec1 --> sound1
  rtpc -.-> sound1
  rtpc -.-> sound_object1
  rtpc -.-> sound_object2
  switch1 -.-> sound_object2
  switch2 -.-> sound_object2
  sound1 --> sound_object1
  sound1 --> sound_object2
  sound_object1 --> channel1
  sound_object2 --> channel2
  sound1 --> channel3
  channel1 --> sound_instance1
  channel1 --> sound_instance2
  channel2 --> sound_instance3
  channel2 --> sound_instance4
  channel2 --> sound_instance5
  channel3 --> sound_instance6
  sound_instance6 -.-> bus1
  sound_instance2 -.-> bus1
  sound_instance1 -.-> bus1
  sound_instance4 -.-> bus1
  sound_instance5 -.-> bus2
  sound_instance3 -.-> bus2
  bus1 -.->|RealChannel| mixer_layer1
  bus1 -.->|RealChannel| mixer_layer2
  bus1 -.->|RealChannel| mixer_layer3
  bus1 -.->|RealChannel| mixer_layer4
  bus2 -.->|RealChannel| mixer_layer5
  bus2 -.->|RealChannel| mixer_layer6
  mixer_layer1 --> pipeline
  mixer_layer2 --> pipeline
  mixer_layer3 --> pipeline
  mixer_layer4 --> pipeline
  mixer_layer5 --> pipeline
  mixer_layer6 --> pipeline
  pipeline --> mixer
  mixer --> driver2
  driver2 -.-> device[/Audio Device/]
{{< /mermaid >}}

## Audio Files and Codecs

Through its [Codec] API, Amplitude is able to read any audio file and to convert it to a [Sound object]. An audio file can be read only if the Engine have a registered codec supporting it.

According to the [sound object definition], audio files can be streamed at runtime or preloaded. Each codec implementation must always read audio samples as **32-bit floating integer** (`AmReal32`), since is the default sample format used in the Engine.

At the moment, Amplitude comes shipped with the following codecs:
- [FLAC](https://xiph.org/flac/), a lossless audio codec.
- [OGG](https://www.xiph.org/ogg/), a lossy audio codec.
- [WAV](https://en.wikipedia.org/wiki/WAV), a lossy audio codec.
- [MP3](https://en.wikipedia.org/wiki/MP3), a lossy audio codec.
- [AMS] (Amplitude Audio Sample), a WAV-like codec built for Amplitude. It uses a high quality [ADPCM](https://en.wikipedia.org/wiki/Adaptive_differential_pulse-code_modulation) compression.

{{< alert context="warning" text="The AMS codec implementation is subject to changes." />}}

You can implement your own codec and register it to the Engine by following [this tutorial](/tutorials/custom-codecs).

## Game Objects

Amplitude can synchronize with the game using game objects. The list of available game objects are:
- [SwitchState]: a game object that represents the state of a [SwitchContainer].
- [RTPC]: a game object that represents a real-time parameter control.
- [Entity]: a game object that represents an entity in the game. An Entity is used to place a [sound object](#sound-objects) in the 3D environment of the game, and then provide position, velocity, orientation and other information to the sound object.
- [Listener]: a game object that represents a listener in the game. A listener is an object that is able to hear the sound objects in the 3D environment of the game. A game should have at least one Listener to let Amplitude render spatial audio.

## Sound Objects

A sound object in Amplitude is the source of audio samples. A sound object can be played, paused, stopped, or seeked during playback. Amplitude have two categories of sound objects to fit many use cases:
- **Standalone Sound Objects**: A standalone sound object is a sound object which is not part of a Container. They can create a [SoundInstance] directly, and are directly linked to a Codec.
- **Containers**: A Container is a special sound object which can contain other sound objects. It can overwrite some of the properties of contained sound objects (like gain, or priority) and is responsible to manage which of the contained sound objects are playing, and when.

There are many sound objects, and each behave differently:
- [Sound]: The most basic sound object. A Sound is a **standalone sound object**, then, it is directly linked to a Codec, and it is responsible to loading and streaming sound data.
- [Collection]: A Collection is a **container sound object**, and is like its name says, a collection of Sounds. The Collection has the particularity to pick and play a single Sound through its [Scheduler] each time it receives a play request. A Collection cannot play more than one Sound at the same time.
- [SwitchContainer]: A SwitchContainer is a **container sound object** which can contains Sounds and Collections. It mixes and play contained sound objects according to [SwitchState]s. Sound objects registered inside a switch container for a specific switch state are played only when this switch state is active. Unlike Collections, SwitchContainers can play more than one sound object at the same time.

Each sound object is created through a JSON configuration file which should match a specific [flatbuffer schema](../schema/) for the sound object.

## Channels

A channel is the user interface between sound objects and [Amplimix](#amplimix). When playing a sound object, the Engine will return a Channel. That Channel will then internally instantiate a [SoundInstance], and will be linked to a [Bus]. It's through a Channel that you can play, pause and stop a sound object, and also control its properties, like the gain, or the location (for world scoped sound objects).

In Amplitude, the number of channels is determined at the Engine initialization. The total number of channels is the sum of the number of **active channels** and the number of **virtual channels**, which are both defined in the engine configuration file.

Amplitude will manage the prioritization of sound objects and will automatically drop the least important channels when too many sound objects are playing simultaneously. This is why the concept of active and virtual channels. The active channels are the channels which are actually playing and rendering audio, while the virtual channels are also playing audio but are not rendering it, and are only used to track low priority sounds which are still playing.

When requesting to play a sound object, the Engine will pick a channel in the list of free active channels. If there is no free channel, the Engine will pick a channel in the list of free virtual channels. If there is still no free channel, the Engine will drop the least important sound object. If the sound object to play is actually the one with the lowest priority, it will not be played.

## Sound Instances

The [SoundInstance] is the real place where the Engine is consuming audio data for mixing. When playing the same sound object multiple times, multiple sound instances will be created for each play request, but all of them will share the same data since it belong to the same sound object. Sound instances only share audio data, and not properties like gain, pan, pitch, location, or priority which are instead managed through the Channel in which they are playing.

It's only in cases of streamed audio that each sound instances own the audio data.

## Buses

The [Bus] is used to adjust the gain of a group of sound instances in tandem. They are created before the Engine initialization with the [buses configuration file](../guide/project-architecture/#busesjson). Any number of buses can be created.

It's inside the sound object definition file that you specify on which bus they are to be played on. Example buses might be `music`, `ambient_sound`, `sound_effects` or `voice_overs`.

A bus have the particularity to optionally have other buses as child buses, which means that all changes to that bus' gain level affect all children of that bus as well. There should always be a single master bus, with name `master`, and with ID `1`. This is the root bus from which all other buses descend. The engine will fail to initialize if the master bus is not found.

A bus may also define `duck_buses`, which are buses that will lower in volume when a sound is played on that bus. For example, a designer might want to have background sound effects and music lower in volume when an important dialog is playing. To do this the `sound_effect` and `music` buses would be added in the list of `duck_buses` children of the `dialog` bus.

## Real Channels

Real Channels are internally managed channels which communicate directly with Amplimix. They are created for each **active channel** and used only when the channel is rendering audio.

## Amplimix

Amplimix is the part of Amplitude doing sound mixing. It is composed of two (02) components linked together to render audio: the mixer layers, and the pipeline.

### Mixer Layers

A Mixer Layer is responsible to consume audio data from sound instances and provide it to the Amplimix Pipeline. A mixer layer stores, for a unique sound instance, the related state for its playback, like the current position of the cursor, the current gain, the current pan, the current pitch, and the current sample rate converter.

Amplimix have a maximum of 4096 mixer layers, which is the maximum number of sound instances that can be played simultaneously.

### Pipeline

The Pipeline is used to process the sound before the mixer get the final output to render on the device. It's in the pipeline that Amplitude process effects, environmental sounds, obstruction, occlusion and more.

The pipeline is built like a node-graph where each node is a [SoundProcessor] instance. Each sound processor takes as input the sound data and outputs the processed audio to the next node.

The default pipeline is the following:

{{< mermaid >}}
%%{init: {'theme': 'base', 'themeVariables': { 'primaryColor': '#74D0F1', 'fontFamily': 'Open Sans' }}}%%
graph LR
  subgraph Amplimix Pipeline
    A[[EffectProcessor]] --> B
    B[[OcclusionProcessor]] --> C
    B --> D
    C[[ObstructionProcessor]] -->|Dry Mix| E[[MixerProcessor]]
    D[[EnvironmentProcessor]] -->|Wet Mix| E
  end
  0(Sound Data) -.-> A
  E -.-> 1[/Audio Device/]
{{< /mermaid >}}

You can create your own set of custom [SoundProcessor]s and register them in the Engine to build a custom Pipeline. The Pipeline is configured through the [engine configuration file](../guide/project-architecture/#audio_configjson).

## Audio Drivers

The Driver is used to render the audio data on the device. It should assume the entire responsibility to convert the input data (the processed data from Amplimix) to the output format required by the device. That output format includes the number of channels, the sample rate, the sample format, and the number of samples per output.

The default driver provided by Amplitude is built on top of the [MiniAudio](https://miniaud.io) library.

## Audio Device

The audio Device renders audio. Amplitude have a copy of the current audio device description in the [DeviceDescription] stored in Amplimix. The device description gives the user requested audio format, and the actual audio format that the device is using. It's to the Driver implementation to fill the device description in Amplimix with the obtained values from the audio device.

Amplimix will process the audio using the user requested format, and let to the driver the responsibility to convert the data to the device audio format.
