---
title: Effect
description: Effects allows you to apply sound effects to any sound object. Effects are applied at the playback time through a pipeline processor.
menu:
  docs:
    parent: project-setup
weight: 205
toc: true
---

Effects allow you to apply sound effects to any sound object. Effects are applied at the playback time through a pipeline processor.

{{< alert >}}
The flatbuffers schema of this file can be found [here](https://github.com/SparkyStudios/AmplitudeAudioSDK/blob/main/schemas/effect_definition.fbs).
{{< /alert >}}

An effect file contains the following properties:

## id

`uint64` `required`

A unique identifier for the effect. This will be used later by the engine and other sound objects to get a reference to this effect. This value should be different from `0`.

## name

`string` `required`

A unique name for the effect. This may be used in runtime to access the effect instance from the engine.

## effect

`string` `required`

The name of the effect to apply to the sound object. That name should have been registered in the engine through the Effect API. By default, the engine comes shipper with built-in effects:

- [BassBoost]
- [Delay]
- [Equalizer]
- [Flanger]
- [Freeverb]
- [LoFi]
- [Robotize]

{{< alert context="info" >}}
You have the ability to create [custom effect]({{< relref "/docs/guide/custom-effect" >}}) as plugins and register them with the engine.
{{< /alert >}}

## parameters

`RtpcCompatibleValue[]` `required`

The list of parameters to pass to the effect when instantiated. These parameters are RTPC compatible, meaning they can be static or dynamic following RTPC changes from the game.

## Example

```json
{
  "id": 1,
  "name": "rvb_small_room",
  "effect": "Freeverb",
  "parameters": [
    {
      "kind": "Static",
      "value": 0.25
    },
    {
      "kind": "Static",
      "value": 1.0
    },
    {
      "kind": "Static",
      "value": 0.5
    },
    {
      "kind": "Static",
      "value": 0.2
    },
    {
      "kind": "Static",
      "value": 1.0
    },
    {
      "kind": "Static",
      "value": 0.0
    }
  ]
}
```
