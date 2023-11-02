---
title: API
description: Browse the API of common types and enumerations shared between Amplitude assets.
menu:
  docs:
    parent: project-setup
weight: 299
toc: true
---

## CurveDefinition {#curve-definition}

Describes a function that is applied on a value to transform the result.

### parts {#curve-definition-parts}

`CurvePartDefinition[]` `required`

An array of objects representing each part of the curve. Each object provides a start point, an end point, and fading function to use to link them. Check the [CurvePartDefinition] section for more information.

The final range of the curve will be bounded within the lowest point and the highest point of all the curve parts.

### Example {#curve-definition-example}

```json
{
  "parts": [
    {
      "start": {
        "x": 0,
        "y": 0
      },
      "end": {
        "x": 1,
        "y": 1
      },
      "fader": "Linear"
    }
  ]
}
```

---

## CurvePartDefinition {#curve-part-definition}

An object describing a single part of a curve. Allowed properties are:

### start {#curve-part-definition-start}

`CurvePointDefinition` `required`

The start point of the curve. It stores the coordinates of the point in the graph. Check the [CurvePointDefinition] section for more information.

### end {#curve-part-definition-end}

`CurvePointDefinition` `required`

The end point of the curve. It stores the coordinates of the point in the graph. Check the [CurvePointDefinition] section for more information.

### fader {#curve-part-definition-fader}

`string` `required`

The fader transition to use while moving values from the `start` point to the `end` point. This stores as value a name to a registered fader transition.

### Example {#curve-part-definition-example}

```json
{
  "start": {
    "x": 0,
    "y": 0
  },
  "end": {
    "x": 1,
    "y": 1
  },
  "fader": "SCurveSmooth"
}
```

---

## CurvePointDefinition {#curve-point-definition}

Represents a point in a curve graph. It's an object defined by 2 coordinate values:

### x {#curve-point-definition-x}

`double` `required`

The position of the point over the X-axis. The X-axis represents the values passed to the curve for computation.

### y {#curve-point-definition-y}

`float` `required`

The position of the point over the Y-axis. The Y-axis represents the result of the computation for a given value.

### Example {#curve-point-definition-example}

```json
{
  "x": 1,
  "y": 343.33
}
```

---

## FadeTransitionSettings {#fade-transition-settings}

This object defines the settings for a fade transition. It is described by the following attributes:

### duration {#fade-transition-settings-duration}

`double` `required`

The duration of the fade transition. The value is expressed in seconds.

### fader {#fade-transition-settings-fader}

`string` `required`

The name of the [Fader] algorithm to be used. It can be one of those shipped with the engine or from plugins.

### Example {#fade-transition-settings-example}

```json
{
  "duration": 3.0,
  "fader": "ExponentialSmooth"
}
```

---

## RtpcCompatibleValue {#rtpc-compatible-value}

An RTPC-compatible value is an object that can hold a static value or a link to an RTPC value.

### kind {#rtpc-compatible-value-kind}

`ValueKind` `default: Static`

Specifies the kind of value that should be used. The possible values are:

| ID     | Description                                                   |
| ------ | ------------------------------------------------------------- |
| Static | The parameter is static value specified by the `value` field. |
| RTPC   | The parameter is a RTPC value specified by the `rtpc` field.  |

When the `kind` property is set to `Static`, it is required to defined a `value` field. When the `kind` property is set to `RTPC`, it is required to define a `rtpc` field.

The default value is `Static`.

### value {#rtpc-compatible-value-value}

`float`

A static value that will be used at runtime. This property is only available when the `kind` property is set to `Static`.

### rtpc {#rtpc-compatible-value-rtpc}

`RtpcParameter`

An object that describe how the value should be updated according to a RTPC object. This object takes as input:

- `id`: The ID of the RTPC object to use.
- `curve`: A [CurveDefinition] object that defines the function to apply on the RTPC value to compute this parameter value.

Check the [RtpcParameter] section for more information.

### Example {#rtpc-compatible-value-example}

```json
// An example of RTPC-compatible value with a static value
{
  "kind": "Static",
  "value": 5
}

// An example of RTPC-compatible value with a RTPC value
{
  "kind": "RTPC",
  "rtpc": {
    "id": 19,
    "curve": {
      "parts": [
        {
          "start": {
            "x": 0,
            "y": 1
          },
          "end": {
            "x": 100,
            "y": 0
          },
          "fader": "Linear"
        }
      ]
    }
  }
}
```

---

## RtpcParameter {#rtpc-parameter}

An object used to defines how to gather values from a [RtpcCompatibleValue] with the `kind` property set to `RTPC`. This allows you to use a curve to convert values from the RTPC object to other values.

### id {#rtpc-parameter-id}

`uint64` `required`

The id of the RTPC object where to get the value. This value should reference a valid RTPC object.

### curve {#rtpc-parameter-curve}

`CurveDefinition` `required`

The curve to use when converting values from the RTPC object. It stores as value an object matching the specification of a [CurveDefinition]. Check the [CurveDefinition] section for more information.

### Example {#rtpc-parameter-example}

```json
{
  "id": 24,
  "curve": {
    "parts": [
      {
        "start": {
          "x": 0,
          "y": 1
        },
        "end": {
          "x": 1,
          "y": 0
        },
        "fader": "SCurveSharp"
      }
    ]
  }
}
```

[CurveDefinition]: #curve-definition
[CurvePartDefinition]: #curve-part-definition
[CurvePointDefinition]: #curve-point-definition
[RtpcCompatibleValue]: #rtpc-compatible-value
[RtpcParameter]: #rtpc-parameter
