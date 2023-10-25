---
title: API
description: ""
menu:
  docs:
    parent: project-setup
weight: 299
toc: true
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

[CurveDefinition]: #curve-definition
