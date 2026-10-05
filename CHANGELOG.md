# Changelog

All notable changes to the Amplitude Audio SDK are recorded here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and the project uses
[Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Changed

- **`ResamplerInstance::SetRatioRamp` (new virtual, source-compatible but not binary-compatible).**
  `AM_API_PUBLIC` resampler implementations can now vary the conversion ratio across the output frames of a
  block instead of stepping it at the boundary. A plugin built against an earlier SDK still loads and runs;
  a plugin that does not override it keeps the mean-ratio behaviour, which advances the stream correctly but
  leaves the read position a staircase — audible as a click at every block boundary while a pitch glides.
  Recompile out-of-tree plugins against this SDK to pick up the smoother path.

### Fixed

- The mixer ramped the playback ratio across each block instead of stepping it once per block. A moving pitch
  — a glide, Doppler, a playback-rate change — no longer leaves a staircase in the resampler's read position,
  which was audible as a click at every block boundary.