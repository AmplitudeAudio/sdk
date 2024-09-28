---
title: Introduction
description: Browse the main features of the Amplitude Audio SDK and discover why this open-source library will be useful for your next game.
menu:
  docs:
    parent: get-started
weight: 101
toc: true
---

{{< callout context="note" title="Note" icon="info-circle" >}}
Amplitude is still in active development. While it already provides a lot of stable features, the documentation for them is not complete. If you have questions, we encourage you to join our [Discord server](https://discord.gg/QR2uBpzJ5f) to ask them in the forum.
{{< /callout >}}

Amplitude is a cross-platform audio engine for games. It provides a wide set of features allowing you to play audio files from any sources in any environments (2D, 3D, VR/AR) with an immersive spatial rendering. This page will give you an overview of some core features of the library.

## Spatial Audio, HRTF, and Ambisonic Rendering

Amplitude can play audio sources in 2D environments, as well in 3D environments too. For 3D environments, you can use the traditional stereo-panning, or use the advanced binaural rendering using your HRTF assets. With the later the listener will be able to feel the exact position of the played source.

## Data-driven Development

Uses `.json` files to define everything, from a simple sound file to a complex switch container. Each `.json` file is in a dedicated directory according to its usage (`events`, `effects`, `sounds`, `collections`, etc.). Amplitude will compile them into binary files, which can now be registered into [sound banks](#soundbanks-management) and loaded in the game at runtime.

## Plugin Architecture

You are able to create plugins and add custom codecs, filters, resampling algorithms, drivers, fading algorithms, and more. Each plugin can be built as a separate library and loaded in the game at runtime.

## Drivers and Codecs

You have total control over how to communicate with the physical audio device (using a [Driver] implementation) and how to decode/encode audio samples (using [Codec] implementations). The library provides you a default Driver implementation using [MiniAudio](http://miniaud.io) for cross-platform support, and by default decodes FLAC, MP3, OGG, and WAV audio samples.

## Customizable Pipeline

Amplitude provides a customizable pipeline graph that you can set up per runtime platforms. Through the graph, you decide how and when Amplitude processes filters, room effects, or advanced spatialization. You also have the ability to create custom nodes (using plugins) and use them in the graph.

## Sound banks Management

A sound bank is a unit where all the data you need for your game is loaded. The choice remains to you on how sound banks are organized. For example, it can be per level (`level1.ambank`) or per categories (`weapon-fires.ambank`, `explosions.ambank`). Each sound object loaded from a sound bank is reference-counted, so even if you load more than one sound bank having a reference to the same sound object, that one will be loaded only once.

## Sound Objects

Amplitude supports a variety of sound objects which have different properties and use cases:
- **Sound**: The most basic sound object. It represents a single audio file played in the game.
- **Collection**: The collection is, like its name says, a collection of Sounds. The collection has the particularity to pick and play a single Sound through its [Scheduler] each time it receives a play request.
- **SwitchContainer**: The switch container mixes and play Sounds or Collections according to switch states. Sound objects registered inside a switch container for a specific switch state are played only when this switch state is active. Unlike collections, switch containers always play all sound objects registered for the active switch state at the same time.

## Game and Engine Synchronization

You can synchronize the Amplitude engine with your game state using two approaches:
- **RTPC**: RTPC stands for Real-Time Parameter Control. It allows you to adjust values in Amplitude (`gain`, `priority`, `effect parameters`, etc.) automatically from the game at runtime. For example, think about a character who climbs a mountain. You can create an RTPC which synchronizes the player altitude (the distance from the ground) with Amplitude to slightly fade between environment sounds, from a calm river (at the ground) to strong winds (at the summit).
- **Switch**: A switch is helpful in cases where you want to let Amplitude know about one state at a time. For example, your character is walking in a forest. A switch can be applied to the terrain to let Amplitude know about the surface the character is walking on, and then play the right footstep sound.

## Game Entities

Amplitude has been designed for games and game engines in mind. For that approach, the engine can link a sound object to an entity of your game, so each entity, even if they are triggering the same sound object, can behave differently and maintain a different state.

## Listeners

A Listener is a special game entity allowing Amplitude to properly render positional sounds. When positional audio is playing, its volume depends on the Listener the sound get attached on, and the configured [attenuation model](#customizable-attenuation-models). If there are no active Listeners, then all positional audio will be silent.

## Customizable Attenuation Models

You can create an infinite variety of attenuation models for spatial sounds in Amplitude, to adjust the gain of sound sources according to the distance from the Listener. You have total control over the attenuation curve settings, including curve transitions. Each attenuation model is based on one attenuation shape between the following list:
- Cone
- Sphere
- Box
- Capsule

## Sound Effects

Amplitude lets you apply effects via DSP filters to your sound objects. The SDK comes by default with a various set of DSP filters, and you are able to add custom filters by creating a plugin.

## Sound Events

Sound events are a way to trigger a set of actions in the game at runtime. For example, you could trigger a set of sound to play and pause after the player has entered a room. Events can run actions sequentially (run one action after another), or simultaneously (run all actions at once). You can add many actions in your event as needed, and each action is of a specific type from the list below:
- **None**: Just do nothing.
- **Wait**: Wait for the specified amount of time.
- **Play**: Play the specified sound objects.
- **Pause**: Pause the specified sound objects.
- **Resume**: Resume the specified sound objects if previously paused.
- **Stop**: Stop the specified sound objects. Already stopped sound objects will be skipped.
- **Seek**: Seeks the specified sound object playback to the specified position.
- **MuteBus**: Completely mute the specified buses.
- **UnmuteBus**: Unmute previously muted buses.

## Buses Management and Auto-ducking

When you play a sound object, a channel is produced. That channel is sent to a bus and then processed by the mixer. Buses can interact together in a way that a bus affect the gain of another one. If there is an important sound or voice over that needs to be heard, audio playing on less important buses can be ducked to ensure the player hears what they need.

## Channel Priority

In your game you may have multiple sound objects playing at the same time, which can be more than what the game can handle. Amplitude will manage the priority of each audio channel and only drop the least important channels when too many simultaneous streams play at once. The number of simultaneous active and virtual channels can be configured for the Engine.

## CLI Tools

The SDK comes shipped with CLI tools of various usage:
- **amac**: (**Am**plitude **A**udio **C**ompressor) allows you to compress an audio sample with a high-quality ADPCM compression, and can optionally convert the sample rate. It's highly recommended to use **amac** when releasing a project running Amplitude.
- **ampk**: (**Am**plitude **P**ac**k**age Builder) allows you to create a release package of your Amplitude project. It packs all your project data, audio assets included, into a single file (`.ampk`) that you can easily ship in your game.
- **amir**: (**Am**plitude **IR** Sphere Builder) allows you to create an IR (Impulse Response) sphere asset to use for HRTF and Ambisonic rendering at runtime. It supports various IR dataset models such as the [MIT (KEMAR) dataset](http://sound.media.mit.edu/resources/KEMAR.html), the [IRCAM (LISTEN) dataset](http://recherche.ircam.fr/equipes/salles/listen/download.html), and the [SADIE II dataset](https://www.york.ac.uk/sadie-project/database.html).
