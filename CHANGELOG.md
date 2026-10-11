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
  Plugin API:
  - `NodeInstance::SetPipelineOutputLayout` / `GetPipelineOutputLayout`: `Custom` nodes read the output layout here.
    A new protected member shifts the `NodeInstance` layout.
  - `ProcessorNodeInstance::EnableFoldDown` / `DisableFoldDown`.
  - `OutputNodeInstance::GetProvider`.
  - New pure virtuals `Sound::GetSpread()` and `Attenuation::GetSpreadFactor()` (source-breaking for implementers).

- Assets with 3, 5, 7 or more than 8 channels no longer play (they were averaged to mono before); the engine logs the
  reason. Supported: mono, stereo, quad, 5.1, 7.1.

- With no `spread` set, a 3D multichannel sound now plays at its asset's native width (stereo +-30 degrees) instead of
  as a mono point; uncorrelated stereo content is up to +3 dB louder than before. Set `spread` to 0 to keep the old
  point-source behaviour.

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

- Binaural decoding filters are built once at engine init and shared by every voice. Each decoder sums its ambisonic
  channels in the frequency domain (16 forward and 2 inverse FFTs per block at order 3, from 32 and 32), and FFT plans
  are shared per size. Starting a voice with the default pipeline dropped from 10745 to 910 profile samples over the
  `core_engine` tests in a debug build (`AmbisonicBinauralizer::Configure`: 9871 to 143), an estimated 1.3 ms per voice from
  about 15 ms; binaural decoding fell from 8.1% to 3.8% of the mix time.

- `AmbisonicSpeakerDecoder` decodes with AllRAD (ITU-R BS.2127 style): the sound field is decoded onto a 1202-point
  Lebedev grid and each direction is panned with the speaker pan law, extended to the sphere with imaginary poles, then
  split in two bands by the ambisonic shelf filter with energy-matched max-rE above the crossover. Decoders are built
  once per output layout and order and shared by every voice. On 5.1 at third order, the high-band loudness spread over
  a full orbit falls from 10.7 to 1.8 dB, and a front-left source now reaches Ls about 6.8 dB more than Rs (it reached
  Rs 1.5 dB more). The fidelity scenario A1 measures it through the engine: 1.85 dB spread and -6.5 dB rear leak.

- Second-order binaural (`BinauralMediumQuality`) decodes on a 20-point dodecahedron instead of a 12-speaker
  horizontal ring, so it keeps elevation.

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
- `Convolver` convolves calls that start mid-block correctly, starts its history silent, and no longer overlap-adds
  with aligned SIMD loads at unaligned offsets (a debug assert, and a fault on SSE).

### Known issues

- A transport fade with the `Linear` fader still ticks faintly at its start and end, about -85 dBFS against the
  -90 dBFS target (fidelity scenarios P2 `default_fade` and P3 `pause_resume`). It does not depend on the resampler;
  it is the corner of the fade curve itself, and is left to a follow-up on fade shapes.
- On a 5.1 output, a source orbiting across the narrow C-R pair ticks at about -89.9 dBFS at block starts against the
  -90 dBFS target (fidelity scenario S1), from per-block pan gain targets.
- The direct `SpeakerPanning` path ignores elevation (an overhead source pans by azimuth), while the same source on
  the ambisonic branch spreads over every speaker.
- `AmbisonicRotator` reverses rotations about the listener's right axis (pitch): a pitched listener, or one in a Y-up
  world (forward -Z, up +Y), hears the ambisonic branch front-to-back swapped. Yaw and roll are correct. The fidelity
  scenario A1 places its listener in the engine's Z-up frame until this is fixed.
