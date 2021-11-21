<p align="center">
  <a href="https://sparkystudios.github.io/AmplitudeAudioSDK">
    <img src="./docs/static/images/logo_trimmed.png" width="320">
  </a>
</p>

<div align="center">
  
# Amplitude Audio SDK

</div>

Amplitude is a full-featured and cross-platform audio engine designed with the needs of games in mind. Apart from his efficient audio mixer, it also adjusts gain based on user-defined attenuation models, plays spatialized sounds in 3D environments, manages prioritization, buses, and a lot more. With all those features, Amplitude is still easy to use, thanks to its data-driven way. All you need is to write configuration files and let Amplitude do the magic for you.

<div align="center">
  <br/>

  [Developer's Guide](https://sparkystudios.github.io/AmplitudeAudioSDK/docs/guide/) | [API Documentation](https://sparkystudios.github.io/AmplitudeAudioSDK/docs/api/) | [JSON Schemas](https://sparkystudios.github.io/AmplitudeAudioSDK/docs/schema/)

  <br/>
</div>

> Amplitude is still in beta and has not been fully battle-tested. So feel free to raise an [issue](https://github.com/SparkyStudios/AmplitudeAudioSDK/issues/new/choose "Open a Github Issue") or open a [PR](https://github.com/SparkyStudios/AmplitudeAudioSDK/pulls), by following our [Contributor Guidelines](https://github.com/SparkyStudios/AmplitudeAudioSDK/blob/main/CODE_OF_CONDUCT.md).

## Features

This is a list of some of the most important features of Amplitude:
- [Data-driven Development](https://sparkystudios.github.io/AmplitudeAudioSDK/features#data-driven-development)
- [Drivers and Codecs](https://sparkystudios.github.io/AmplitudeAudioSDK/features#drivers-and-codecs)
- [Soundbanks Management](https://sparkystudios.github.io/AmplitudeAudioSDK/features#soundbanks-management)
- [Sound Objects](https://sparkystudios.github.io/AmplitudeAudioSDK/features#sound-objects)
- [Game and Engine Synchronization](https://sparkystudios.github.io/AmplitudeAudioSDK/features#game-and-engine-synchronization)
- [Game Entities](https://sparkystudios.github.io/AmplitudeAudioSDK/features#game-entities)
- [Listeners](https://sparkystudios.github.io/AmplitudeAudioSDK/features#listeners)
- [Customizable Attenuation Models](https://sparkystudios.github.io/AmplitudeAudioSDK/features#customizable-attenuation-models)
- [Sound Effects](https://sparkystudios.github.io/AmplitudeAudioSDK/features#sound-effects)
- [Sound Events](https://sparkystudios.github.io/AmplitudeAudioSDK/features#sound-events)
- [Buses Management and Auto-ducking](https://sparkystudios.github.io/AmplitudeAudioSDK/features#buses-management-and-auto-ducking)
- [Channel Priority](https://sparkystudios.github.io/AmplitudeAudioSDK/features#channel-priority)

## Supported Platforms

Amplitude can support the following platforms:
- ❌ [Android](https://www.android.com/) devices
- ❌ [iOS](https://www.apple.com/ios) devices
- ✔️ [Linux](https://www.kernel.org/)
- ⚠️ [OS X](https://www.apple.com/osx/)
- ✔️ [Windows](https://www.microsoft.com/windows/)

Amplitude uses [MiniAudio](http://miniaud.io/) as the default audio device implementation for cross-platform support. You still can change that by implementing a [custom driver](docs/advanced/custom-driver).

## Download

Official Amplitude release files are available from the [GitHub releases page](https://github.com/SparkyStudios/AmplitudeAudioSDK/releases).

## License

Amplitude Audio SDK is licensed under the [Apache License 2.0](https://github.com/SparkyStudios/AmplitudeAudioSDK/blob/main/LICENSE).
