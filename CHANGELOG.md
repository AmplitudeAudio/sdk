# Changelog

All notable changes to the Amplitude Audio SDK are recorded here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/). The build reports its own `AM_VERSION_*` values.

## [Unreleased]

### Changed

- **Centred sounds are 3 dB louder (re-level existing mixes).** Panning is constant power everywhere: a centred mono
  sound now reaches each side at -3.01 dB instead of -6.02 dB, and a stereo asset played without spatialization plays
  at unity on its own channels instead of being folded to mono and centred. Sounds keep their channels until the
  pipeline maps them: 3D multichannel sounds place each channel as an emitter around the sound's direction, its width
  set by the new `spread` property (sound definitions) and `spread_curve` (attenuations).

- **`Node::GetChannelPolicy` (new virtual; an ABI break for out-of-tree node plugins).** A node declares whether it
  takes a mono fold-down (`Mono`, the default, which matches the mono input nodes were written for), every channel
  (`PerChannel`) or handles layouts itself (`Custom`). Source stays compatible; binaries must be recompiled.
  `PipelineInstance::Configure` now returns `bool`: a pipeline whose mixer providers disagree on the channel count, or
  whose last node does not produce the output layout, is refused and the sound does not play.

- `StereoPanning` and `StereoMixer` are now `SpeakerPanning` and `Mixer`. The old names stay registered as aliases,
  so existing pipeline assets load unchanged.

- The miniaudio driver opens the device with an explicit WAV/SMPTE channel map.

- **`ResamplerInstance::SetRatioRamp` (new virtual; an ABI break for out-of-tree resampler plugins).**
  `AM_API_PUBLIC` resampler implementations can now vary the conversion ratio across the output frames of a
  block instead of stepping it at the boundary. The header change is source-compatible — a plugin that does not
  override it inherits the base class's mean-ratio implementation, which advances the stream correctly but
  leaves the read position a staircase, audible as a click at every block boundary while a pitch glides — but
  adding a virtual shifts the vtable layout, so a binary built against an earlier SDK must be recompiled.

- A seek crossfades over 8 ms instead of 5 ms. A seek restarts the resampler from an empty history, and the longer
  fade keeps the band-limited onset of the new position below the click floor.

### Added

- Surround output. The engine configuration's new `output.channels` renders Mono, Stereo, Quad, 5.1 or 7.1 (default
  Stereo); `Default` takes the device's native layout. `Engine::GetOutputLayout()` reports the active layout.
- `AmbisonicSpeakerDecoder` decodes the ambisonic branch to the output speakers; `sample_project/pipelines/surround.json`
  uses it. `AmbisonicBinauralDecoder` on a non-stereo output feeds L and R only, and warns once.
- `ChannelLayout` (public): named layouts, speaker azimuths and the ITU-R BS.775 fold-down matrices.

### Fixed

- The mixer ramped the playback ratio across each block instead of stepping it once per block. A moving pitch
  — a glide, Doppler, a playback-rate change — no longer leaves a staircase in the resampler's read position,
  which was audible as a click at every block boundary.

### Known issues

- A transport fade with the `Linear` fader still ticks faintly at its start and end, about -85 dBFS against the
  -90 dBFS target (fidelity scenarios P2 `default_fade` and P3 `pause_resume`). It does not depend on the resampler;
  it is the corner of the fade curve itself, and is left to a follow-up on fade shapes.
