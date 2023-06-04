---
title: Attenuation Model
description: Attenuation models are settings that describe how the gain of a sound object should fade according to the distance from its listener and a given shape. Read this article to learn more about them.
lead: ""
date: 2023-06-01T01:45:07+01:00
lastmod: 2023-06-01T01:45:07+01:00
draft: false
images: []
menu:
  docs:
    parent: project-setup
weight: 240
toc: true
---

Attenuation models are the way to specify how the gain of a sound object is affected by its position in space (in case of spatialized sounds), and its distance from the attached listener.

{{< alert >}}
The flatbuffers schema of this file can be found [here](https://github.com/SparkyStudios/AmplitudeAudioSDK/blob/main/schemas/attenuation_definition.fbs).
{{< /alert >}}

An attenuation model is configured with the following properties:

## id

`uint64` `required`

An unique identifier for the attenuation model. This will be used later to specify the attenuation model in the sound objects. This value should be different than `0`.

## name

`string` `required`

An unique name for the attenuation model. This may be used in runtime to access the attenuation instance from the engine.

## max_distance

`double` `required`

The maximum distance for the sound object to propagate. When the distance from the attached listener of a sound object to that sound object itself is greater than the `max_distance` value, the gain is automatically set to zero.

## shape

`object` `required`

Each attenuation model is represented by a shape instructing the engine on how to affect the sound object's gain.

The value of the `shape` property is an object with the following properties:

### zone

`object` `required`

This value store the definition of the type of shape you want for the attenuation model. The final behavior will change according to that type of shape. Amplitude made available the following shapes for attenuation models:

- Box Shape
- Capsule Shape
- Cone Shape
- Sphere Shape

{{< alert context="info" >}}
To learn more about shapes and their properties, please refer to the [Shapes guide].
{{< /alert >}}

### max_attenuation_factor

`float` `required`

The `max_attenuation_factor` value defines the maximum amount of attenuation to apply to the sound object. This value is used when the listener is inside the outer zone of the shape but outside its inner zone.

## gain_curve

`object` `required`

This specify the curve used to change the sound object's gain. The values over the X-axis of the curve are the distance between the sound object and the listener, and over the Y-axis of the curve are the gain of the sound object For best results the curve must fit in the range `[0, max_distance]` over the X-axis, and in the range `[0, 1]` over the Y-axis.

## Example

An example of a simple attenuation model with a box shape may be:

```json
{
  "id": 2,
  "name": "room",
  "max_distance": 40,
  "shape": {
    "zone_type": "Box",
    "zone": {
      "inner": {
        "half_width": 15,
        "half_height": 5,
        "half_depth": 5
      },
      "outer": {
        "half_width": 20,
        "half_height": 35,
        "half_depth": 10
      }
    },
    "max_attenuation_factor": 0.125
  },
  "gain_curve": {
    "parts": [
      {
        "start": {
          "x": 0,
          "y": 1
        },
        "end": {
          "x": 40,
          "y": 0
        },
        "fader": "ExponentialSmooth"
      }
    ]
  }
}
```
