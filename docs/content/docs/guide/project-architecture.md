---
title: Project Architecture
weight: 4
---

An Amplitude project is resumed to a set of `.json` files organized in dedicated directories. The complete architecture of an Amplitude project can be described by:

```text
📁 amplitude_project_name/
|--📁 attenuators/
|--📁 collections/
|--📁 effects/
|--📁 environments/
|--📁 events/
|--📁 rtpc/
|--📁 soundbanks/
|--📁 sounds/
|--📁 switch_containers/
|--📁 switches/
|--📄 audio_config.json
|--📄 buses.json
```

## audio_config.json

The `audio_config.json` file is used to configure the Amplitude engine behavior at initialization. It stores all the needed settings for the audio device setup, memory allocation, and mixer configuration. Learn more about those settings on the [Engine Configuration](../engine-config) page.

{{< tip >}}
The `audio_config.json` file should exist in an Amplitude project directory and match this [flatbuffer schema](https://github.com/SparkyStudios/AmplitudeAudioSDK/blob/main/schemas/engine_config_definition.fbs).
{{< /tip >}}

## buses.json

The `buses.json` file contains the list of configured buses for the project and their settings. In an Amplitude project, there should be only one file that describes project buses. You can set a custom file path to that file in `audio_config.json`. Learn more about those settings on the [Creating Buses](../creating-buses) page.

{{< tip >}}
Only one buses file is expected in an Amplitude project. If no file is specified in `audio_config.json`, the default expected path is `./buses.json`. If the buses file is not found during the engine initialization, the library will throw an exception. The `.json` file of the buses definition should math this [flatbuffer schema](https://github.com/SparkyStudios/AmplitudeAudioSDK/blob/main/schemas/buses_definition.fbs).
{{< /tip >}}

## attenuators/

The `attenuators` directory stores all the configuration files for custom [Attenuation] models. Each `.json` file of this directory should match this [flatbuffer schema](https://github.com/SparkyStudios/AmplitudeAudioSDK/blob/main/schemas/attenuation_definition.fbs). Learn more about attenuators in the [Attenuation Model](../attenuation-model) page.

## collections/

The `collections` directory contains the configuration files for [Collection] sound objects. Each `.json` file of this directory should match this [flatbuffer schema](https://github.com/SparkyStudios/AmplitudeAudioSDK/blob/main/schemas/collection_definition.fbs). Learn more about collections in the [Sounds Collection](../sounds-collection) page.

## effects/

The `effects` directory contains the configuration files for sound [Effect]s. Each `.json` file should match this [flatbuffer schema](https://github.com/SparkyStudios/AmplitudeAudioSDK/blob/main/schemas/effect_definition.fbs). Learn more about effects in the [Sound Effects & Filters](../sound-effects) page.

## events/

The `events` directory contains the configuration files for [Event]s. Each `.json` file of this directory should match this [flatbuffer schema](https://github.com/SparkyStudios/AmplitudeAudioSDK/blob/main/schemas/event_definition.fbs). Learn more about events on the [Sound Events](../sound-events) page.

## rtpc/

The `rtpc` directory stores the configuration files to create [RTPC] values. Each `.json` file of this directory should match this [flatbuffer schema](https://github.com/SparkyStudios/AmplitudeAudioSDK/blob/main/schemas/rtpc_definition.fbs). Learn more about rtpc values on the [Real Time Parameter Control](../rtpc) page.

## soundbanks/

The `soundbanks` directory is the place where you define all the game sound banks. Each `.json` file of this directory should match this [flatbuffer schema](https://github.com/SparkyStudios/AmplitudeAudioSDK/blob/main/schemas/sound_bank_definition.fbs). Learn more about sound banks on the [Building Sound Banks](../build-sound-bank) page.

## sounds/

The `sounds` directory contains the definition files for [Sound] objects. Those files are used to describe raw audio sample assets to make them usable as standalone objects or in Collections and Switch Containers. Each `.json` file of this directory should match this [flatbuffer schema](https://github.com/SparkyStudios/AmplitudeAudioSDK/blob/main/schemas/sound_definition.fbs). Learn more about sounds on the [Sounds](../sounds) page.

## switch_containers/

The `switch_containers` directory contains configuration files for [SwitchContainer] sound objects. Each `.json` file of this directory should match this [flatbuffer schema](https://github.com/SparkyStudios/AmplitudeAudioSDK/blob/main/schemas/switch_container_definition.fbs). Learn more about switches and switch containers on the [Switches & Switch Containers](../switches-and-switch-containers) page.

## switches/

The `switches` directory contains configuration files for [Switch] objects, that will be used in [SwitchContainer]s. Each `.json` file of this directory should match this [flatbuffer schema](https://github.com/SparkyStudios/AmplitudeAudioSDK/blob/main/schemas/switch_definition.fbs). Learn more about switches and switch containers on the [Switches & Switch Containers](../switches-and-switch-containers) page.

[Attenuation]: ../../api/Sound/Attenuation
[Collection]: ../../api/Sound/Collection
[Effect]: ../../api/Sound/Effect
[Event]: ../../api/Sound/Event
[RTPC]: ../../api/Sound/RTPC
[Sound]: ../../api/Sound/Sound
[SwitchContainer]: ../../api/Sound/SwitchContainer
[Switch]: ../../api/Sound/Switch
