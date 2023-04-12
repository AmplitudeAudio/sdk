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
|--📄 config.pc.json
|--📄 config.mobile.json
|--📄 buses.json
```

## Engine configuration files

The Amplitude engine settings are provided through a json file at the root of the project. In our previous example, we had two engine configuration files according to the runtime platform: `config.pc.json` and `config.mobile.json`. The config file to use should be given when initializing Amplitude.

The config files stores all the needed settings for the audio device setup, memory allocation, and mixer configuration. Learn more about those settings on the [Engine Configuration](../engine-config) page.

{{< tip >}}
At least one engine configuration file should exist in an Amplitude project directory, and should match this [flatbuffer schema](https://github.com/SparkyStudios/AmplitudeAudioSDK/blob/main/schemas/engine_config_definition.fbs).
{{< /tip >}}

## Buses files

An Amplitude project should have a file (or files) where the different buses used at runtime are defined. In our previous example, the `buses.json` file plays this role.

You can only use one bus file per engine instance, and you should give his path in the engine configuration file selected at runtime. Learn more about those settings on the [Creating Buses](../creating-buses) page.

{{< tip >}}
At least one buses file is expected in an Amplitude project. If no file is specified in the engine settings, the default expected path is `./buses.json`. If the buses file is not found during the engine initialization, the library will throw an exception. The `.json` file of the buses definition should math this [flatbuffer schema](https://github.com/SparkyStudios/AmplitudeAudioSDK/blob/main/schemas/buses_definition.fbs).
{{< /tip >}}

## attenuators/

The `attenuators` directory stores all the configuration files for custom [Attenuation] models. Each `.json` file of this directory should match this [flatbuffer schema](https://github.com/SparkyStudios/AmplitudeAudioSDK/blob/main/schemas/attenuation_definition.fbs). Learn more about attenuators in the [Attenuation Model](../attenuation-model) guide.

## collections/

The `collections` directory contains the configuration files for [Collection] sound objects. Each `.json` file of this directory should match this [flatbuffer schema](https://github.com/SparkyStudios/AmplitudeAudioSDK/blob/main/schemas/collection_definition.fbs). Learn more about collections in the [Sound Collections](../sound-collections) guide.

## effects/

The `effects` directory contains the configuration files for sound [Effect]s. Each `.json` file should match this [flatbuffer schema](https://github.com/SparkyStudios/AmplitudeAudioSDK/blob/main/schemas/effect_definition.fbs). Learn more about effects in the [Sound Effects & Filters](../sound-effects) guide.

## events/

The `events` directory contains the configuration files for [Event]s. Each `.json` file of this directory should match this [flatbuffer schema](https://github.com/SparkyStudios/AmplitudeAudioSDK/blob/main/schemas/event_definition.fbs). Learn more about events on the [Sound Events](../sound-events) page.

## rtpc/

The `rtpc` directory stores the configuration files to create [RTPC] values. Each `.json` file of this directory should match this [flatbuffer schema](https://github.com/SparkyStudios/AmplitudeAudioSDK/blob/main/schemas/rtpc_definition.fbs). Learn more about rtpc values on the [Real Time Parameter Control](../rtpc) guide.

## soundbanks/

The `soundbanks` directory is the place where you define all of your game sound banks. Each `.json` file of this directory should match this [flatbuffer schema](https://github.com/SparkyStudios/AmplitudeAudioSDK/blob/main/schemas/sound_bank_definition.fbs). Learn more about sound banks on the [Building Sound Banks](../build-sound-bank) guide.

## sounds/

The `sounds` directory contains the definition files for [Sound] objects. Those files are used to describe raw audio sample assets to make them usable as standalone objects or in Collections and Switch Containers. Each `.json` file of this directory should match this [flatbuffer schema](https://github.com/SparkyStudios/AmplitudeAudioSDK/blob/main/schemas/sound_definition.fbs). Learn more about sounds on the [Sounds](../sounds) guide.

## switch_containers/

The `switch_containers` directory contains configuration files for [SwitchContainer] sound objects. Each `.json` file of this directory should match this [flatbuffer schema](https://github.com/SparkyStudios/AmplitudeAudioSDK/blob/main/schemas/switch_container_definition.fbs). Learn more about switches and switch containers on the [Switch States & Switch Containers](../switch-states-and-switch-containers) guide.

## switches/

The `switches` directory contains configuration files for [Switch] objects, that will be used in [SwitchContainer]s. Each `.json` file of this directory should match this [flatbuffer schema](https://github.com/SparkyStudios/AmplitudeAudioSDK/blob/main/schemas/switch_definition.fbs). Learn more about switches and switch containers on the [Switch States & Switch Containers](../switch-states-and-switch-containers) guide.

[Attenuation]: ../../api/Sound/Attenuation
[Collection]: ../../api/Sound/Collection
[Effect]: ../../api/Sound/Effect
[Event]: ../../api/Sound/Event
[RTPC]: ../../api/Sound/RTPC
[Sound]: ../../api/Sound/Sound
[SwitchContainer]: ../../api/Sound/SwitchContainer
[Switch]: ../../api/Sound/Switch
