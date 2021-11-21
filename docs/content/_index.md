+++
title = "Amplitude Audio SDK"
+++

{{< block >}}
{{< column >}}

# Amplitude Audio SDK

Amplitude is a full-featured and cross-platform audio engine designed with the needs of games in mind. Apart from his efficient audio mixer, it also adjusts gain based on user-defined attenuation models, plays spatialized sounds in 3D environments, manages prioritization, buses, and a lot more. With all those features, Amplitude is still easy to use, thanks to its data-driven way. All you need is to write configuration files and let Amplitude do the magic for you.

{{< button "docs/" "Get Started" >}}

{{< tip "warning" >}}
Amplitude is still in beta and has not been fully battle-tested. So feel free to raise an [issue](https://github.com/SparkyStudios/AmplitudeAudioSDK/issues/new/choose "Open a Github Issue") or open a [PR](https://github.com/SparkyStudios/AmplitudeAudioSDK/pulls), by following our [Contributor Guidelines](https://github.com/SparkyStudios/AmplitudeAudioSDK/blob/main/CODE_OF_CONDUCT.md).
{{< /tip >}}

{{< /column >}}
{{< /block >}}

{{< block "mt-1" >}}
{{< column >}}

## Features

This is a list of some of the most important features of Amplitude:
- [Data-driven Development](features#data-driven-development)
- [Drivers and Codecs](features#drivers-and-codecs)
- [Soundbanks Management](features#soundbanks-management)
- [Sound Objects](features#sound-objects)
- [Game and Engine Synchronization](features#game-and-engine-synchronization)
- [Game Entities](features#game-entities)
- [Listeners](features#listeners)
- [Customizable Attenuation Models](features#customizable-attenuation-models)
- [Sound Effects](features#sound-effects)
- [Sound Events](features#sound-events)
- [Buses Management and Auto-ducking](features#buses-management-and-auto-ducking)
- [Channel Priority](features#channel-priority)

{{< /column >}}
{{< /block >}}

{{< block "mt-1" >}}
{{< column >}}

## Supported Platforms

Amplitude can support the following platforms:
- ❌ [Android](https://www.android.com/) devices
- ❌ [iOS](https://www.apple.com/ios) devices
- ✔️ [Linux](https://www.kernel.org/)
- ⚠️ [OS X](https://www.apple.com/osx/)
- ✔️ [Windows](https://www.microsoft.com/windows/)

Amplitude uses [MiniAudio](http://miniaud.io/) as the default audio device implementation for cross-platform support. You still can change that by implementing a [custom driver](docs/advanced/custom-driver).

{{< /column >}}
{{< /block >}}

{{< block "mt-1" >}}
{{< column >}}

## Download

Official Amplitude release files are available from the [GitHub releases page](https://github.com/SparkyStudios/AmplitudeAudioSDK/releases).

{{< /column >}}
{{< /block >}}

{{< block "mt-1" >}}
{{< column >}}

## License

Amplitude Audio SDK is licensed under the [Apache License 2.0](https://github.com/SparkyStudios/AmplitudeAudioSDK/blob/main/LICENSE).

{{< /column >}}
{{< /block >}}
