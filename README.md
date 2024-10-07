![build](https://github.com/AmplitudeAudio/sdk/actions/workflows/build.yml/badge.svg)
[![Codecov](https://img.shields.io/codecov/c/gh/AmplitudeAudio/sdk?style=flat&logo=codecov&label=Coverage)](https://codecov.io/gh/AmplitudeAudio/sdk?branch=develop)
[![discord server](https://dcbadge.vercel.app/api/server/q8eT8Mq6du?style=flat)](https://discord.gg/q8eT8Mq6du)
[![X (formerly Twitter) Follow](https://img.shields.io/twitter/follow/amaudiosdk?style=social&logo=x&label=%40amaudiosdk)](https://x.com/@amaudiosdk)

<p align="center">
  <a href="https://amplitudeaudiosdk.com">
    <img src="https://amplitudeaudiosdk.com/images/logo_trimmed.png" width="320">
  </a>
</p>

<div align="center">

# Amplitude Audio SDK

Amplitude is a full-featured and cross-platform audio engine designed with the needs of games in mind. Apart from its
efficient audio mixer, it also adjusts gain based on user-defined attenuation models, plays spatialized sounds in 3D
environments, manages prioritization, buses, and a lot more. With all those features, Amplitude is still easy to use,
thanks to its data-driven way. All you need is to write configuration files and let Amplitude do the magic for you.

  <br/>

[Project Setup](https://docs.amplitudeaudiosdk.com/nightly/project/) | [Integration](https://docs.amplitudeaudiosdk.com/nightly/integration/) | API Documentation

  <br/>
</div>

> Amplitude is still in active development, and no official release is made yet. However, the API is stable and no breaking changes will be introduced until the first release. To get a copy of the SDK, you should [build it from sources](https://docs.amplitudeaudiosdk.com/nightly/getting-started/installation/#build-from-sources). Feel free to raise an [issue](https://github.com/AmplitudeAudio/sdk/issues/new/choose "Open a Github Issue") or open a [PR](https://github.com/AmplitudeAudio/sdk/pulls), by following our [Contributor Guidelines](https://github.com/AmplitudeAudio/sdk/blob/main/CODE_OF_CONDUCT.md).

## Sample Projects

The quickest way to get started with the SDK is to have a look on the [sample projects](https://github.com/AmplitudeAudio/sdk/blob/develop/samples).

## Features

This is a list of some of the most important features of Amplitude:

- [Spatial Audio, HRTF, and Ambisonic Rendering](https://docs.amplitudeaudiosdk.com/nightly/getting-started/introduction/#spatial-audio-hrtf-and-ambisonic-rendering)
- [Data-driven Development](https://docs.amplitudeaudiosdk.com/nightly/getting-started/introduction/#data-driven-development)
- [Plugin Architecture](https://docs.amplitudeaudiosdk.com/nightly/getting-started/introduction/#plugin-architecture)
- [Drivers and Codecs](https://docs.amplitudeaudiosdk.com/nightly/getting-started/introduction/#drivers-and-codecs)
- [Customizable Pipeline](https://docs.amplitudeaudiosdk.com/nightly/getting-started/introduction/#customizable-pipeline)
- [Sound banks Management](https://docs.amplitudeaudiosdk.com/nightly/getting-started/introduction/#sound-banks-management)
- [Sound Objects](https://docs.amplitudeaudiosdk.com/nightly/getting-started/introduction/#sound-objects)
- [Game and Engine Synchronization](https://docs.amplitudeaudiosdk.com/nightly/getting-started/introduction/#game-and-engine-synchronization)
- [Game Entities](https://docs.amplitudeaudiosdk.com/nightly/getting-started/introduction/#game-entities)
- [Listeners](https://docs.amplitudeaudiosdk.com/nightly/getting-started/introduction/#listeners)
- [Customizable Attenuation Models](https://docs.amplitudeaudiosdk.com/nightly/getting-started/introduction/#customizable-attenuation-models)
- [Sound Effects](https://docs.amplitudeaudiosdk.com/nightly/getting-started/introduction/#sound-effects)
- [Sound Events](https://docs.amplitudeaudiosdk.com/nightly/getting-started/introduction/#sound-events)
- [Buses Management and Auto-ducking](https://docs.amplitudeaudiosdk.com/nightly/getting-started/introduction/#buses-management-and-auto-ducking)
- [Channel Priority](https://docs.amplitudeaudiosdk.com/nightly/getting-started/introduction/#channel-priority)
- [CLI Tools](https://docs.amplitudeaudiosdk.com/nightly/getting-started/introduction/#cli-tools)

## Supported Platforms

Amplitude can support the following platforms:
- ❌ [Android](https://www.android.com/) devices
- ❌ [iOS](https://www.apple.com/ios/) devices
- ✔️ [Linux](https://www.kernel.org/)
- ✔️ [OS X](https://www.apple.com/osx/)
- ✔️ [Windows](https://www.microsoft.com/windows/)

Amplitude uses [MiniAudio](http://miniaud.io/) as the default audio device implementation for cross-platform support. You still can change that by implementing a [custom driver](https://docs.amplitudeaudiosdk.com/nightly/tutorials/custom-driver/).

## Download

Official Amplitude release files are available from the [GitHub releases page](https://github.com/AmplitudeAudio/sdk/releases).

## License

Amplitude Audio SDK is licensed under the [Apache License 2.0](https://github.com/AmplitudeAudio/sdk/blob/main/LICENSE).
