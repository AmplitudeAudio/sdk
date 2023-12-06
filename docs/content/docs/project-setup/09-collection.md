---
title: Collection
description: A collection is a container sound object. It plays sounds registered in it based on the specified scheduler.
menu:
  docs:
    parent: project-setup
weight: 209
toc: true
---

The Collection is the first container sound object and the simplest. It organizes and schedules a list of [Sound]s({{< relref "08-sound" >}}), which can then be played randomly or sequentially according to a scheduler.

{{< alert >}}
The flatbuffers schema of this file can be found [here](https://github.com/SparkyStudios/AmplitudeAudioSDK/blob/main/schemas/collection_definition.fbs).
{{< /alert >}}

Apart from the common properties of [sound object]({{< relref "07-sound-object" >}}) assets, a collection asset contains the following properties:

## scheduler

`SoundSchedulerSettings` `required`

The `scheduler` property defines the behavior of the sound scheduler of this collection. It takes as value an object with the following properties:

- **`mode`**: The scheduler mode can be either `Random` or `Sequence`.
- **`config`**: An object containing the configuration of the selected mode.

Amplitude supports 02 (two) scheduler modes for collections, and for each mode a specific configuration:

### Random Scheduler

The random scheduler will randomly pick and play a sound from the collection on each play request. It can be configured to avoid repetitions with the following properties:

#### avoid_repeat

`bool` `default: false`

When this property is set to `true`, the scheduler will avoid picking a sound from the collection before `n` different sounds are played, where `n` is the value defined by the `repeat_count` property.

#### repeat_count

`uint` `default: 1`

When a sound is played, this property specifies the number of other different sounds to play before it may be played again.


### Sequence Scheduler

The sequence scheduler will play the sounds of the collection sequentially in the order they have been registered. You can customize the behavior of the scheduler when it reaches the end of the sounds list using the `end_behavior` property.

#### end_behavior

`SequenceSoundSchedulerEndBehavior` `required`

When the sequence scheduler reaches the end of the collection, this property specifies how it should behave on the next play request. The available values are:

| ID      | Description                                                                                                                                    |
| ------- | ---------------------------------------------------------------------------------------------------------------------------------------------- |
| Restart | The scheduler will restart the sequence from the first sound of the collection.                                                                |
| Reverse | The scheduler will play the collection in the reverse order until the first sound is reached. The first and last sounds won't be played twice. |

## play_mode

`CollectionPlayMode` `default: PlayOne`

Specifies how the sounds in the collection are played when a play request is sent to the collection. The possible values include:

| ID      | Description                                                                                                                                                                                                          |
| ------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| PlayOne | On each play request, a single sound is picked by the [scheduler](#scheduler) and played                                                                                                                             |
| PlayAll | On each play request, the [scheduler](#scheduler) will pick and play each sound of the collection, one after the other, until each of the sounds has played. It's guaranteed that all sounds will play exactly once. |

## sounds

`CollectionEntry[]` `required`

This property contains the list of sounds registered to the collection. It's an array where each item is an object with the following properties:

### sound

`uint64` `required`

Provides the ID of a sound. That sound object should be defined as a [Sound]({{< relref "08-sound" >}}) asset first. This value should be different from `0`.

### gain

`RtpcCompatibleValue` `required`

This property is used to override the default gain of the sound object. It stores an [RtpcCompatibleValue]({{< relref "99-api/#rtpc-compatible-value" >}}) object.

### weight

`float` `default: 1.0`

This property is used only when the collection scheduler [mode](#scheduler) is set to `Random`. The value is a floating number in the range `[0, 1]`, representing the probability (relative to other sounds of the same collection) of the sound to be picked by the random scheduler. The default value is `1.0`.

## Example

```json
{
  "id": 1003,
  "name": "grass_footsteps",
  "priority": {
    "kind": "Static",
    "value": 1.0
  },
  "gain": {
    "kind": "Static",
    "value": 0.0625
  },
  "bus": 2,
  "sounds_type": [
    "Random",
    "Random",
    "Random",
    "Random",
    "Random",
    "Random",
    "Random",
    "Random"
  ],
  "sounds": [
    {
      "sound": 201,
      "gain": {
        "kind": "Static",
        "value": 1.0
      }
    },
    {
      "sound": 202,
      "gain": {
        "kind": "Static",
        "value": 1.0
      }
    },
    {
      "sound": 203,
      "gain": {
        "kind": "Static",
        "value": 1.0
      }
    },
    {
      "sound": 204,
      "gain": {
        "kind": "Static",
        "value": 1.0
      }
    },
    {
      "sound": 205,
      "gain": {
        "kind": "Static",
        "value": 1.0
      }
    },
    {
      "sound": 206,
      "gain": {
        "kind": "Static",
        "value": 1.0
      }
    },
    {
      "sound": 207,
      "gain": {
        "kind": "Static",
        "value": 1.0
      }
    }
  ],
  "spatialization": "Position",
  "attenuation": 1,
  "play_mode": "PlayOne",
  "scheduler": {
    "mode": "Random",
    "config_type": "Random",
    "config": {
      "avoid_repeat": true,
      "repeat_count": 4
    }
  },
  "scope": "Entity",
  "fader": "Linear"
}
```
