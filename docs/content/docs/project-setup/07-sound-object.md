---
title: Sound Objects
description: Sound objects are the core assets of an Amplitude project. They define the various audio samples to be played and how to play them.
menu:
  docs:
    parent: project-setup
weight: 207
toc: true
---

Sound objects are the core assets of an Amplitude project. They define the various audio samples to be played and how to play them.

Amplitude supports 03 (three) kinds of sound objects:

- [Sound]({{< relref "08-sound" >}}): The most basic sound object. It references a single audio file.
- [Collection]({{< relref "09-collection" >}}): A container sound object. It manages a set of sounds and decides how and when to play them through a [scheduler]({{< relref "09-collection#scheduler" >}}).
- [Switch Container]({{< relref "10-switch-container" >}}): A container sound object. It can automatically play or schedule the playback of a sound object (Sound or Collection), according to the active state of a specific [switch]({{< relref "06-switch" >}}).

Each of those sound objects has in common the following set of properties:

## id

`uint64` `required`

A unique identifier for the sound object. This will be used later by the engine and other sound objects to get a reference to this one. This value should be different from `0`.

## name

`string` `required`

A unique name for the sound object. This may be used in runtime to access the sound object's instance from the engine.

## effect

`uint64` `optional`

With this property you can specify a special [effect]({{< relref "05-effect" >}}) to apply to the sound object when played, by giving the `id` of that effect.

## gain

`RtpcCompatibleValue` `required`

The `gain` property stores the value of the gain (the volume) of the sound object. The value should match the schema of a [RtpcCompatibleValue] object.

## bus

`uint64` `required`

This property stores the `id` of a bus object, on which this sound should be played. A sound object can only be played by one bus at a time.

## priority

`RtpcCompatibleValue` `required`

This property affects how the engine will prioritize this sound object relative to all others. When the engine is out of active channels, sound objects with low priority are muted when a play request is made with a sound object with higher priority. The value of this property should match the schema of an [RtpcCompatibleValue] object.

## spatialization

`Spatialization` `default: None`

The `spatialization` property specifies how the sound object's gain should be adjusted based on its distance from the attached listener. It can have the following values:

| ID  | Name                | Description                                                                                                                                                                          |
| --- | ------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| 0   | None                | No spatialization is made, sound objects are played at their regular gain.                                                                                                           |
| 1   | Position            | The gain is adjusted based on the distance to the attached listener.                                                                                                                 |
| 2   | PositionOrientation | The gain is adjusted based on the distance to the attached listener and the entity's orientation. This means that the [scope](#scope) of this sound object should be set to `Entity` |

## attenuation

`uint64` `default: 0`

Specifies the ID of the [Attenuation Model]({{< relref "04-attenuation-model" >}}) to use on the sound object. This property takes effect only if the sound is spatialized (the `spatialization` property is set to a value different from `None`). A value of `0` means that no attenuation will be performed, even if the sound is spatialized.

## scope

`Scope` `default: World`

With the `scope` property, you can control how the playback data is shared between each sound instance. The allowed values are:

| ID  | Name   | Description                                                                                                           |
| --- | ------ | --------------------------------------------------------------------------------------------------------------------- |
| 0   | World  | All sound instances will be treated as one object, so they will share the same sound data.                            |
| 1   | Entity | Each sound instance will be treated as one object per entity, and will only share sound data within that same entity. |

## fader

`string` `required`

The `fader` property can be used to specify the fading animation to apply when the playback starts and stops. Its value should be a string containing the name of the animation. Amplitude comes shipped with a various set of faders:

- `Constant`
- `ExponentialSharp`
- `ExponentialSmooth`
- `Linear`
- `SCurveSharp`
- `SCurveSmooth`

{{< alert context="info" >}}
You can create more faders as you wish and register them in the engine as plugins. Refer to the [Custom Fader]({{< relref "/guide/plugins/custom-fader" >}}) guide to learn more.
{{< /alert >}}

[RtpcCompatibleValue]: {{< relref "99-api/#rtpc-compatible-value" >}}
