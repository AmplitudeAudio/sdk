---
title : "Engine Configuration"
description: "Amplitude has been built in a way to let you have complete freedom on the behavior of the engine at runtime. You can create several configuration file per device (pc, mobile, console), per platform (Windows, Android, XBox, PlayStation), or any other criteria your project have to suit with, then pick and load the right settings file at runtime."
lead: "Amplitude has been built in a way to let you have complete freedom on the behavior of the engine at runtime. You can create several configuration file per device (pc, mobile, console), per platform (Windows, Android, XBox, PlayStation), or any other criteria your project have to suit with, then pick and load the right settings file at runtime."
date: 2023-06-01T00:39:09+01:00
lastmod: 2023-06-01T00:39:09+01:00
draft: false
images: []
menu:
  docs:
    parent: "project-setup"
weight: 220
toc: true
---

{{< alert >}}
The flatbuffers schema of this file can be found [here](https://github.com/SparkyStudios/AmplitudeAudioSDK/blob/main/schemas/engine_config_definition.fbs).
{{< /alert >}}

The config file will let you customize:

- The playback device ([output])
- The Amplitude Mixer, called **Amplimix** ([mixer])
- The game synchronization ([game])
- The path to the buses file ([buses_file])
- The name of the driver implementation to use ([driver])

[output]: #output
[mixer]: #mixer
[game]: #game
[buses_file]: #buses_file
[driver]: #driver

## output

`object` `required`

The `output` property helps you define how Amplitude should communicate with the physical audio device. It takes as value a map with the following properties:

### frequency

`uint` `default: 48000`

The `frequency` property defines the audio frequency in Hertz (`Hz`) of the audio data sent to the audio device by Amplitude. This value may differ to the frequency of the audio device, in such scenario, the audio data will be resampled from this value to the device's frequency.

### channels

`enum` `default: Stereo`

The `channels` property sets the number of channels Amplitude will output. It can take as value the name of that channel or the number of channels. Possible values are:

| Channel Type | Description                                                                           | Mapping                                                                                                                                                                                                          | Value                                        |
| ------------ | ------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------- |
| Default      | Takes the default value from the [Driver]. In most cases the default value is Stereo. | According to the default value.                                                                                                                                                                                  | Number:&nbsp;0<br/>String:&nbsp;Default      |
| Mono         | Compute and output audio data in one channel.                                         | 0:&nbsp;FRONT&nbsp;CENTER                                                                                                                                                                                        | Number:&nbsp;1<br/>String:&nbsp;Mono         |
| Stereo       | Compute and output audio data in 2 channels.                                          | 0:&nbsp;FRONT&nbsp;LEFT<br/>1:&nbsp;FRONT&nbsp;RIGHT                                                                                                                                                             | Number:&nbsp;2<br/>String:&nbsp;Stereo       |
| Quad         | Compute and output audio data in 4 channels.                                          | 0:&nbsp;FRONT&nbsp;LEFT<br/>1:&nbsp;FRONT&nbsp;RIGHT<br/>2:&nbsp;FRONT&nbsp;CENTER<br/>3:&nbsp;BACK&nbsp;CENTER                                                                                                  | Number:&nbsp;4<br/>String:&nbsp;Quad         |
| 5.1          | Compute and output audio data in 6 channels.                                          | 0:&nbsp;FRONT&nbsp;LEFT<br/>1:&nbsp;FRONT&nbsp;RIGHT<br/>2:&nbsp;FRONT&nbsp;CENTER<br/>3:&nbsp;LFE<br/>4:&nbsp;SIDE&nbsp;LEFT<br/>5:&nbsp;SIDE&nbsp;RIGHT                                                        | Number:&nbsp;6<br/>String:&nbsp;Surround_5_1 |
| 7.1          | Compute and output audio data in 8 channels.                                          | 0:&nbsp;FRONT&nbsp;LEFT<br/>1:&nbsp;FRONT&nbsp;RIGHT<br/>2:&nbsp;FRONT&nbsp;CENTER<br/>3:&nbsp;LFE<br/>4:&nbsp;BACK&nbsp;LEFT<br/>5:&nbsp;BACK&nbsp;RIGHT<br/>6:&nbsp;SIDE&nbsp;LEFT<br/>7:&nbsp;SIDE&nbsp;RIGHT | Number:&nbsp;8<br/>String:&nbsp;Surround_7_1 |

{{< alert context="warning" >}}
At the moment, Amplitude only supports **Stereo** and **Mono** channels. Setting the engine to use something else than these values will result in errors, or undefined behaviors.
{{< /alert >}}

### buffer_size

`int` `default: 1024`

This value defines the number of audio bytes used per output. The number of samples to produce for each output will be calculated automatically by dividing this value with the number of channels. It is highly recommended to use a multiple of 2 for the buffer size.

### format

`enum` `default: Float32`

The `format` property specifies the audio format in which Amplitude will send the audio data to the output device. It can take as value the name of the audio format or the audio format ID:

| ID  | Name    | Description                                                                          |
| --- | ------- | ------------------------------------------------------------------------------------ |
| 0   | Default | Uses the default format available on the audio device.                               |
| 1   | UInt8   | Process and send data as `unsigned 8-bit fixed-point numbers` to the audio device.   |
| 2   | Int16   | Process and send data as `signed 16-bit fixed-point numbers` to the audio device.    |
| 3   | Int24   | Process and send data as `signed 24-bit fixed-point numbers` to the audio device.    |
| 4   | Int32   | Process and send data as `signed 32-bit fixed-point numbers` to the audio device.    |
| 5   | Float32 | Process and send data as `signed 32-bit floating-point numbers` to the audio device. |

{{< alert context="info" >}}
Amplitude internally process audio data as 32-bit floating-point numbers. The `format` setting is used only when sending audio data to the audio device. If the audio device is also set to receive float32 audio data, no conversion will be performed.
{{< /alert >}}

## mixer

`object` `required`

The `mixer` property configures the Amplitude Mixer (Amplimix). It takes as value an object with the following properties:

### active_channels

`uint` `required`

Specifies the maximum number of sound channels to render by Amplimix. It equals the number of sound simultaneously playing in the game. If the maximum number of channels is reached, Amplitude will prioritize the most important channels and virtualize the others.

### virtual_channels

`uint` `required`

Specifies the maximum number of virtual channels to use in addition to active channels. Amplimix doesn't render virtual channels, but all the information about them are still tracked.

### pipeline

`array` `required`

It's inside the `pipeline` setting you describe the graph in which audio data will be processed before to be sent to the audio device for rendering. This setting take as value an array of sound processor definitions that will be applied to the audio data. For further explanation about how to setup a pipeline, see the [Pipeline & Sound Processors](../pipeline-and-sound-processors) guide.

## game

`object` `required`

This setting is used by Amplitude to understand how it should synchronize with the game, how to handle game and sound objects, and how to render sounds in the game environment. This is achieved by specifying a set of inner values in this setting.

### listener_fetch_mode

`enum` `default: Nearest`

Since Amplitude allows you to define many listeners at the same time, but only one can actually render data for a single sound object, this setting is used to define how the engine will pick the right listener for each rendered sound objects. Available values are:

| ID  | Name     | Description                                                                      |
| --- | -------- | -------------------------------------------------------------------------------- |
| 0   | None     | Do not fetch for listeners. This mute all audio, but keeps processing data.      |
| 1   | Nearest  | Fetches for the listener nearest to the currently processed sound.               |
| 2   | Farthest | Fetches for the listener farthest to the currently processed sound.              |
| 2   | Default  | Always use the default listener set in the engine at runtime for every playback. |
| 2   | First    | Always use the first available listener of the list for every playback.          |
| 2   | Last     | Always use the last available listener of the list for every playback.           |

By using `Nearest` or `Farthest`, different listeners may be used at the same time for each playback. Using `Default`, `First`, or `Last` ensure that only one listener is used for every playback.

### listeners

`uint` `default: 1`

The `listeners` property specifies the maximum number of listeners to pre-allocate. You will not be able to create more [Listener] objects than that value at runtime.

### entities

`uint` `default: 4096`

The `entities` property specifies the maximum number of game entities to pre-allocate. This value does not represent all the entities of your game or scene but only the approximate number of entities managed by the Amplitude Engine. You will not be able to create more [Entity] objects than that value at runtime.

### environments

`uint` `default: 64`

The `environments` property specifies the maximum number of sound environments to pre-allocate. Sound environments are spaces in the 3D environment of the game where Amplitude may process specific effects and attenuation models. You will not be able to create more [Environment] objects than that value at runtime.

### sound_speed

`float` `default: 343.0`

This property sets the speed of sound (in meters per seconds) in the game. This value will be used by the engine to process some effects like [the Doppler effect](https://en.wikipedia.org/wiki/Doppler_effect). If the value is not defined, it will default to `343.0`, which is the approximated value of the real speed of sound.

### doppler_factor

`float` `default: 1.0`

This property takes a float value greater or equal to `0.0`. It will affect how much power is given to the Doppler effect. A value of `0.0` will disable the Doppler effect, while a value of `1.0` will render it as it should. Any other value will affect the sound pitch.

### up_axis

`enum` `default: Y`

This property may help you to not perform additional math while integrating Amplitude in your game or your game engine. It specifies the up axis you use in your game. Possible values are:

| ID  | Name |
| --- | ---- |
| 0   | Y    |
| 1   | Z    |

This setting will only affect how math operations on vectors and matrices are performed internally in the engine.

### obstruction

`object` `required`

The `obstruction` property let you setup the way Amplitude will compute sound obstruction in the game. It takes as value an object with the following properties:

- **lpf_curve**: Set the Low-Pass Filter curve for the obstruction sound processor.
- **gain_curve**: Set the gain curve for the obstruction sound processor.

### occlusion

`object` `required`

The `occlusion` property works exactly the same as the `obstruction` property, but it's used instead to instruct Amplitude how to process sound occlusion in the game.

{{< alert >}}
To learn more about how to create curves, you can read the [Building Curves](../building-curves) guide.
{{< /alert >}}

### track_environments

`boolean` `default: true`

Defines whether the game is tracking environments. This means that the game will compute and send the environment amounts to the engine. This implies that the shapes defined in environments (if any) will not be used.

Setting this value to `false` will instruct Amplitude to track environment amounts by himself. This way, Amplitude will use the environment shapes and the positions of the entities playing the sounds to compute the environment amounts.

## buses_file

`string` `required`

The `buses_file` property defines the path to the binary (`.ambus`) file that contains the buses definitions. Only one bus file can be loaded per engine configuration.

## driver

`string` `required`

The `driver` property indicates the name of the audio [Driver] implementation communicating with the physical audio device. You can implement multiple audio drivers as needed and register them in the engine with the plugin API. Read the [Writing Drivers](../../advanced/writing-drivers) guide to learn how to do.

## Example

The following example describes an engine configuration file:

```json
{
  "output": {
    "frequency": 44100,
    "channels": 2,
    "buffer_size": 1024,
    "format": "Float32"
  },
  "mixer": {
    "active_channels": 50,
    "virtual_channels": 100,
    "pipeline_type": [
      "AudioSoundProcessor",
      "AudioSoundProcessor",
      "AudioProcessorMixer"
    ],
    "pipeline": [
      {
        "processor": "EffectProcessor"
      },
      {
        "processor": "OcclusionProcessor"
      },
      {
        "dry_processor": "ObstructionProcessor",
        "wet_processor": "EnvironmentProcessor",
        "dry": 0.5,
        "wet": 0.5
      }
    ]
  },
  "game": {
    "listener_fetch_mode": "Nearest",
    "track_environments": true,
    "up_axis": "Y",
    "listeners": 100,
    "entities": 4096,
    "environments": 512,
    "sound_speed": 333,
    "doppler_factor": 1.0,
    "obstruction": {
      "lpf_curve": {
        "parts": [
          {
            "start": {
              "x": 0,
              "y": 0
            },
            "end": {
              "x": 1,
              "y": 1
            },
            "fader": "Linear"
          }
        ]
      },
      "gain_curve": {
        "parts": [
          {
            "start": {
              "x": 0,
              "y": 1
            },
            "end": {
              "x": 1,
              "y": 0
            },
            "fader": "Linear"
          }
        ]
      }
    },
    "occlusion": {
      "lpf_curve": {
        "parts": [
          {
            "start": {
              "x": 0,
              "y": 0
            },
            "end": {
              "x": 1,
              "y": 1
            },
            "fader": "Linear"
          }
        ]
      },
      "gain_curve": {
        "parts": [
          {
            "start": {
              "x": 0,
              "y": 1
            },
            "end": {
              "x": 1,
              "y": 0
            },
            "fader": "Linear"
          }
        ]
      }
    }
  },
  "buses_file": "buses.ambus",
  "driver": "miniaudio"
}
```

{{< alert >}}
The file is written in a flatbuffers compatible format, that is why properties like **pipeline_type** are present. You may want to read the flatbuffers documentation to know more.
{{< /alert >}}
