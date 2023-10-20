---
title: Buses Configuration
description: Buses are the places where sound object are routed before to be processed by the mixer. This page contains details about how to configure buses for your project.
menu:
  docs:
    parent: project-setup
weight: 203
toc: true
---

This configuration file allows you to register the list of buses Amplitude will use in the engine. For each bus, you can configure settings for auto-ducking between them.

{{< alert >}}
The flatbuffers schema of this file can be found [here](https://github.com/SparkyStudios/AmplitudeAudioSDK/blob/main/schemas/buses_definition.fbs).
{{< /alert >}}

When configuring buses, you are allowed to create as much buses as you want, but there is should be at least one `master` bus. The master bus is a special bus that controls all the other buses. It should have an `id` attribute set to `1` and a `name` attribute set to `master`.

{{< alert context="warning" text="If at the initialization of the engine no master bus has been found, Amplitude will stop and you won't be able to play any audio." />}}

## buses

`array` `required`

The `buses` property is the only root property of this file. It should store an array of buses objects describing the settings for each bus Amplitude will use at runtime. Each object of this array should have the following properties:

### id

`uint64` `required` `default: 0`

The unique identifier of the bus as a 64-bit unsigned integer. This value should be unique per bus, and should have a value different than `0`. There
is a special rule for the `master` bus which should have this value set to `1`.

### name

`string` `required`

The unique name of the bus. this value is required and should be unique per bus. There is a special rule for the `master` bus which should have this value set to `master`.

### gain

`float` `default: 1.0`

The linear gain that is applied to all the sound objects playing on the bus. If set to `0.0`, all the sound objects of this bus will be muted.

The final gain of a bus is calculated by multiplying this value with the parent bus' gain, an user-specified gain provided at runtime, and this bus ducked gain.

### child_buses

`array` `optional`

The `child_buses` property stores a list of integer values, each value representing the `id` of a bus that should be considered as child of this one. Child buses are affected by the parent bus' final gain. So if the parent bus is muted (gain = 0.0), then all the children of this bus will be muted too.

For the `master` bus to work properly, you may want to feed his `child_buses` property with all the root buses (buses with no parent) of your configuration. That way, the `master` bus will correctly control the gain of all the other buses.

### duck_buses

`array` `optional`

`duck_buses` is an array of object allowing you to setup a ducking behavior between two or more buses that play sound objects simultaneously. Each object of this array should have the following properties:

- **id**: This is the ID of the bus to control between the list of declared buses.
- **target_gain**: This specifies the target gain the controlled bus should have after the ducking is done.
- **fade_in**: This is the fader settings used when the controlled bus is being ducked to the target gain.
- **fade_out**: This is the fader settings used when the controlled bus is being restored to its original gain.

### fader

`string` `required`

This property stores the name of the fader algorithm that will be used when the gain of this bus will be manually updated at runtime.

{{< alert context="info" >}}
Fader and fader settings helps you to control how a property should move from one value to another. You can learn more about faders in the [Faders] guide.
{{< /alert >}}

## Example

An example of a bus configuration file may looks like:

```json
{
  "buses": [
    {
      "id": 1,
      "name": "master",
      "gain": 1.0,
      "child_buses": [
        2,
        7678456242523
      ],
      "fader": "Linear"
    },
    {
      "id": 2,
      "name": "voices",
      "gain": 1,
      "duck_buses": [
        {
          "id": 7678456242523,
          "target_gain": 0.25,
          "fade_in": {
            "duration": 3,
            "fader": "SCurveSmooth"
          },
          "fade_out": {
            "duration": 3,
            "fader": "Linear"
          }
        }
      ],
      "fader": "Linear"
    },
    {
      "id": 7678456242523,
      "name": "environment",
      "gain": 0.75,
      "fader": "Linear"
    }
  ]
}
```
