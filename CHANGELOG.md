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

### Fixed

- The mixer ramped the playback ratio across each block instead of stepping it once per block. A moving pitch
  — a glide, Doppler, a playback-rate change — no longer leaves a staircase in the resampler's read position,
  which was audible as a click at every block boundary.
