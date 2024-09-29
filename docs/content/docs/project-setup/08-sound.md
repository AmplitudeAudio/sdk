---
title: Sound
description: A Sound is the most basic sound object handled by the engine. This page helps you create sound assets for your Amplitude project.
menu:
  docs:
    parent: project-setup
weight: 208
toc: true
---

Sound is where you define sources for any sound objects that will be used in the runtime. They also represent the most basic form of sound objects, allowing you to trigger audio directly from a file.

{{< alert >}}
The flatbuffers schema of this file can be found [here](https://github.com/AmplitudeAudio/sdk/blob/main/schemas/sound_definition.fbs).
{{< /alert >}}

Apart from the common properties of [sound object]({{< relref "07-sound-object" >}}) assets, a sound asset contains the following properties:

## stream

`bool` `default: false`

Whether this sound should be streamed from disk or entirely loaded into memory. This allows you to optimize the memory consumed by the engine. In general, sounds like background music or cinematic voices are streamed, and sound effects like gunfire or footsteps are loaded in memory. The choice can also be made to optimize the amount of time the engine will need to access/play the sound, as sounds loaded in memory play faster than streamed sounds.

## loop

`object` `required`

Configures how the engine should loop the sound. The property stores an object with the following keys.

### enabled

`bool` `required`

Defines if the engine should loop the sound or not. If this value is `false`, the sound will be played once each time the engine will receive a play request for it.

### loop_count

`uint32` `default: 0`

Specifies the number of times the sound should loop until it is automatically stopped by the engine. Set this value to `0` to loop the sound until manually stopped.

## path

`string` `required`

This value points to the source file of the sound. The file may be of any format supported by the engine (MP3, OGG, FLAC, WAV, or AMS), or from a format you have created a Codec plugin for.

## Example

```json {title="forest.ambient.json"}
{
  "id": 100,
  "name": "AMB_Forest",
  "effect": 0,
  "gain": {
    "kind": "Static",
    "value": 1
  },
  "bus": 2,
  "priority": {
    "kind": "Static",
    "value": 1.0
  },
  "stream": true,
  "loop": {
    "enabled": true,
    "loop_count": 0
  },
  "spatialization": "None",
  "attenuation": 0,
  "fader": "SCurveSmooth",
  "path": "environment/AMB_Forest_ID_0100_BSB.wav"
}
```
