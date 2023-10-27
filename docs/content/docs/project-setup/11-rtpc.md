---
title: Real-Time Parameter Control
description: A RTPC allows you to synchronize a value between your game and Amplitude. This page will explain you how to setup RTPC assets.
menu:
  docs:
    parent: project-setup
weight: 211
toc: true
---

A RTPC allows you to synchronize a value between your game and Amplitude. With Amplitude, you can update several values automatically through RTPC, such as sound object gain and priority, as well as effect parameters.

An RTPC object is described by the following properties:

## id

`uint64` `required`

A unique value across RTPC assets that represents the ID of this object. It may be reused later to get the instance of this RTPC from the engine at runtime.

## name

`string` `required`

A unique value across RTPC assets that represents the name of this object. It may be reused later to get the instance of this RTPC from the engine at runtime.

## min_value

`double` `required`

The minimum value the RTPC is allowed to have.

## max_value

`double` `required`

The maximum value the RTPC is allowed to have.

## default_value

`double` `default: 0.0`

The default value assigned to the RTPC at initialization. If this value is outer of the range `[min_value, max_value]`, it will be clamped in that range.

## fade_settings

`RtpcFadingSettings` `required`

This property allows you to customize the fade transition for when the RTPC value increase or decrease. It takes as value an object with the following properties:

### enabled

`bool` `default: false`

Defines whether the RTPC value should fade on change. If `false` the new value will be set without transitions.

### fade_attack

`FadeTransitionSettings`

Settings for the fade transition to apply when RTPC value is increased. The value should match the schema of a [FadeTransitionSettings] object.

### fade_release

`FadeTransitionSettings`

Settings for the fade transition to apply when RTPC value is decreased. The value should match the schema of a [FadeTransitionSettings] object.

## Example

```json
{
  "id": 1,
  "name": "rtpc_player_height",
  "min_value": 0,
  "max_value": 100,
  "fade_settings": {
    "enabled": true,
    "fade_attack": {
      "duration": 1,
      "fader": "ExponentialSmooth"
    },
    "fade_release": {
      "duration": 1,
      "fader": "ExponentialSmooth"
    }
  }
}
```

[FadeTransitionSettings]: {{< relref "99-api/#fade-transition-settings" >}}
