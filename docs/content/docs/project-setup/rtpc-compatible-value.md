---
title: RTPC Compatible Value
description: ""
menu:
  docs:
    parent: project-setup
weight: 299
toc: true
---

That value should be RTPC-compatible, hence, its defined by an object with the following properties:

- **kind**: This property can store two different values.
- `Static`: When `kind` is set to `Static`, it means that the defined value is not provided by an RTPC and should be used as is every time needed. In this case the `value` property of this object should be set.
- `RTPC`: When `kind` is set to `RTPC`, it means that the defined value is provided by an RTPC. In this case the `rtpc` property of this object should be set.
- **value**: This property stores the value as float only in cases where `kind` is set to `Static`.
- **rtpc**: This property should be set only when `kind` is set to `RTPC`. It stores an object with a reference to the RTPC to use, and a curve describing how to affect the gain:
- `id`:
