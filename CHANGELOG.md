# Changelog

All notable changes to the Amplitude Audio SDK are recorded here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/). The build reports its own `AM_VERSION_*` values.

## [Unreleased]

### Changed

- **`ResamplerInstance::SetRatioRamp` (new virtual; an ABI break for out-of-tree resampler plugins).**
  `AM_API_PUBLIC` resampler implementations can now vary the conversion ratio across the output frames of a
  block instead of stepping it at the boundary. The header change is source-compatible — a plugin that does not
  override it inherits the base class's mean-ratio implementation, which advances the stream correctly but
  leaves the read position a staircase, audible as a click at every block boundary while a pitch glides — but
  adding a virtual shifts the vtable layout, so a binary built against an earlier SDK must be recompiled.

- A seek crossfades over 8 ms instead of 5 ms. A seek restarts the resampler from an empty history, and the longer
  fade keeps the band-limited onset of the new position below the click floor.

### Fixed

- The mixer ramped the playback ratio across each block instead of stepping it once per block. A moving pitch
  — a glide, Doppler, a playback-rate change — no longer leaves a staircase in the resampler's read position,
  which was audible as a click at every block boundary.

### Known issues

- A transport fade with the `Linear` fader still ticks faintly at its start and end, about -85 dBFS against the
  -90 dBFS target (fidelity scenarios P2 `default_fade` and P3 `pause_resume`). It does not depend on the resampler;
  it is the corner of the fade curve itself, and is left to a follow-up on fade shapes.
