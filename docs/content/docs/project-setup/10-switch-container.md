---
title: Switch Container
description: A switch container is a special container sound object that automatically play or schedule sounds when a specific switch changes his state. This page will help you create switch container assets for your Amplitude project.
menu:
  docs:
    parent: project-setup
weight: 210
toc: true
---

A switch container is a special container sound object that automatically play or schedule sounds when a specific [Switch] changes his state.

{{< alert >}}
The flatbuffers schema of this file can be found [here](https://github.com/SparkyStudios/AmplitudeAudioSDK/blob/main/schemas/switch_container_definition.fbs).
{{< /alert >}}

Apart from the common properties of [sound object]({{< relref "07-sound-object" >}}) assets, a switch container asset contains the following properties:

## switch_group

`uint64` `required`

Provides the ID of the [Switch] object to listen for changes in this switch container. This value cannot be changed at runtime.

## default_switch_state

`uint64` `required`

The initial state of the switch. This value should be one of the available states you defined in the [Switch] object.

## update_behavior

`SwitchContainerUpdateBehavior` `required`

With this property you can specify the behavior of the switch container when the tracked [Switch] object changes his state. The accepted values are:

| ID             | Description                                                                                                                                                                                                                                                                                                                                    |
| -------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| UpdateOnPlay   | The switch container will update the sounds on the next play request after the tracked switch has changed his state.                                                                                                                                                                                                                           |
| UpdateOnChange | The switch container will update the sounds at the time the tracked switch has changed his state. That means if a sound from the container was already playing, it will be stopped and sounds registered for the new state will be played. If the currently playing sound was registered for the old and new states, it will continue playing. |

## entries

`SwitchContainerEntry[]` `required`

The `entries` property is an array of items (sound objects) you want to register in this switch container. Each entry is an object with the following properties:

### object

`uint64` `required`

The ID of a sound object ([Sound] or [Collection]).

### switch_states

`uint64[]` `required`

The list of states where this entry is assigned. That means the sound object of this entry will play only when one of the assigned states will be active.

### continue_between_states

`bool` `required`

Defines whether the sound object should continue playing when the switch state changes, and both new and old states are assigned to this entry.

- If `true`, the sound object will continue to play when the state will change.
- If `false`, the sound object will stop and start over after the switch has changed his state.

### fade_in

`FadeTransitionSettings` `required`

The fade transition to apply on the sound object when one of the assigned states becomes active. It takes as value a [FadeTransitionSettings] object.

### fade_out

`FadeTransitionSettings` `required`

The fade transition to apply on the sound object when one all of the assigned states becomes inactive. It takes as value a [FadeTransitionSettings] object.

### gain

`RtpcCompatibleValue` `required`

A custom `gain` to apply on the sound object of this entry. The value should match the schema of an [RtpcCompatibleValue]({{< relref "99-api/#rtpc-compatible-value" >}}) object.

## Example

```json
{
  "id": 200,
  "name": "footsteps",
  "priority": {
    "kind": "Static",
    "value": 1.0
  },
  "gain": {
    "kind": "Static",
    "value": 0.0625
  },
  "bus": 2,
  "spatialization": "Position",
  "attenuation": 1,
  "scope": "Entity",
  "fader": "Linear",
  "update_behavior": "UpdateOnChange",
  "switch_group": 1,
  "default_switch_state": 4,
  "entries": [
  {
    "object": 1003,
    "continue_between_states": false,
    "fade_in": {
      "duration": 1,
      "fader": "Linear"
    },
    "fade_out": {
      "duration": 1,
      "fader": "Linear"
    },
    "gain": {
      "kind": "Static",
      "value": 1.0
    },
    "switch_states": [
    4
    ]
  },
  {
    "object": 1004,
    "continue_between_states": false,
    "fade_in": {
      "duration": 1,
      "fader": "Linear"
    },
    "fade_out": {
      "duration": 1,
      "fader": "Linear"
    },
    "gain": {
      "kind": "Static",
      "value": 1.0
    },
    "switch_states": [
    5
    ]
  },
  {
    "object": 1005,
    "continue_between_states": false,
    "fade_in": {
      "duration": 1,
      "fader": "Linear"
    },
    "fade_out": {
      "duration": 1,
      "fader": "Linear"
    },
    "gain": {
      "kind": "Static",
      "value": 1.0
    },
    "switch_states": [
    3
    ]
  }
  ]
}
```

[Sound]: {{< relref "08-sound" >}}
[Collection]: {{< relref "09-collection" >}}
[Switch]: {{< relref "06-switch" >}}
[FadeTransitionSettings]: {{< relref "99-api/#fade-transition-settings" >}}
