---
title: Switch
description: A switch maintains a single state at a time during the runtime. Changes to that state are tracked by switch containers, allowing them to update playing sounds accordingly.
menu:
  docs:
    parent: project-setup
weight: 206
toc: true
---

A switch is an object maintaining a single state at a time during runtime. Its main purpose is to send feedback to [Switch Container] objects, so they can play the right sounds for the current state. For example, you could create a switch for a terrain or any walkable area, with states like grass, ice, or wood, and link it to a switch container playing footstep sounds according to the current state.

{{< alert >}}
The flatbuffers schema of this file can be found [here](https://github.com/SparkyStudios/AmplitudeAudioSDK/blob/main/schemas/switch_definition.fbs).
{{< /alert >}}

A switch asset is described by the following attributes:

## id

`uint64` `required`

A unique value across switch assets that represents the ID of this object. It may be reused later to get the instance of this switch from the engine at runtime.

## name

`string` `required`

A unique value across switch assets that represents the name of this object. It may be reused later to get the instance of this switch from the engine at runtime.

## states

`SwitchStateDefinition` `required`

An array of switch states. Each state is represented by an object containing the following attributes:

### id

`uint64` `required`

A unique value between the states of this switch. It represents the identifier of this state and may be reused later in switch containers to link a sound object with this state, or in the engine at runtime to get the instance of this state.

### name

`string` `required`

The name of this state. This value should be unique between the states of this switch.

## Example

```json
{
  "id": 1,
  "name": "surface_type",
  "states": [
    {
      "id": 1,
      "name": "wood"
    },
    {
      "id": 2,
      "name": "water"
    },
    {
      "id": 3,
      "name": "metal"
    },
    {
      "id": 4,
      "name": "grass"
    },
    {
      "id": 5,
      "name": "snow"
    }
  ]
}
```

[Switch Container]: {{< relref "10-switch-container" >}}
