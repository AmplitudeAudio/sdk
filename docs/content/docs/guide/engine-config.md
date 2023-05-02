---
title: Engine Configuration
weight: 5
---

{{< block >}}
{{< column "mb-1" >}}

Amplitude has been built in a way to let you have complete freedom on the behavior of the engine at runtime. You can create several configuration file per device (pc, mobile, console), per platform (Windows, Android, XBox, PlayStation), or any other criteria your project have to suit with, then pick and load the right settings file at runtime.

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

{{< /column >}}
{{< /block >}}

## output

`type: object`<br/>`required`

The `output` property helps you define how Amplitude should communicate with the physical audio device. It takes as value a map with the following properties.

#### frequency

`type: uint`<br/>`default: 48000`

The `frequency` property defines the audio frequency in Hertz (`Hz`) of the audio data sent to the audio device by Amplitude.

#### channels

`type: enum`<br/>`default: Stereo`

The `channels` property sets the number of channels Amplitude will output. It can take as value the name of that channel or the number of channels. Possible values are:

| Channel Type | Description                                                                           | Mapping                                                                                                                                                                                                          | Value                                        |
| ------------ | ------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------- |
| Default      | Takes the default value from the [Driver]. In most cases the default value is Stereo. | According to the default value.                                                                                                                                                                                  | Number:&nbsp;0<br/>String:&nbsp;Default      |
| Mono         | Compute and output audio data in one channel.                                         | 0:&nbsp;FRONT&nbsp;CENTER                                                                                                                                                                                        | Number:&nbsp;1<br/>String:&nbsp;Mono         |
| Stereo       | Compute and output audio data in 2 channels.                                          | 0:&nbsp;FRONT&nbsp;LEFT<br/>1:&nbsp;FRONT&nbsp;RIGHT                                                                                                                                                             | Number:&nbsp;2<br/>String:&nbsp;Stereo       |
| Quad         | Compute and output audio data in 4 channels.                                          | 0:&nbsp;FRONT&nbsp;LEFT<br/>1:&nbsp;FRONT&nbsp;RIGHT<br/>2:&nbsp;FRONT&nbsp;CENTER<br/>3:&nbsp;BACK&nbsp;CENTER                                                                                                  | Number:&nbsp;4<br/>String:&nbsp;Quad         |
| 5.1          | Compute and output audio data in 6 channels.                                          | 0:&nbsp;FRONT&nbsp;LEFT<br/>1:&nbsp;FRONT&nbsp;RIGHT<br/>2:&nbsp;FRONT&nbsp;CENTER<br/>3:&nbsp;LFE<br/>4:&nbsp;SIDE&nbsp;LEFT<br/>5:&nbsp;SIDE&nbsp;RIGHT                                                        | Number:&nbsp;6<br/>String:&nbsp;Surround_5_1 |
| 7.1          | Compute and output audio data in 8 channels.                                          | 0:&nbsp;FRONT&nbsp;LEFT<br/>1:&nbsp;FRONT&nbsp;RIGHT<br/>2:&nbsp;FRONT&nbsp;CENTER<br/>3:&nbsp;LFE<br/>4:&nbsp;BACK&nbsp;LEFT<br/>5:&nbsp;BACK&nbsp;RIGHT<br/>6:&nbsp;SIDE&nbsp;LEFT<br/>7:&nbsp;SIDE&nbsp;RIGHT | Number:&nbsp;8<br/>String:&nbsp;Surround_7_1 |

{{< tip "warning" >}}
At the moment, Amplitude only supports **Stereo** and **Mono** channels. Setting the engine to use something other than these values  will result in errors, or undefined behaviors.
{{< /tip >}}

#### format

The `format` property defines the audio format in which Amplitude should process audio data. It can take as value the name of the audio format or the audio format ID:

| ID  | Name    | Description                                                                            |
| --- | ------- | -------------------------------------------------------------------------------------- |
| 0   | Default | Uses the default format available on the audio device.                                 |
| 1   | UInt8   | Process and send data as `unsigned 8 bytes fixed-point numbers` to the audio device.   |
| 2   | Int16   | Process and send data as `signed 16 bytes fixed-point numbers` to the audio device.    |
| 3   | Int24   | Process and send data as `signed 24 bytes fixed-point numbers` to the audio device.    |
| 4   | Int32   | Process and send data as `signed 32 bytes fixed-point numbers` to the audio device.    |
| 5   | Float32 | Process and send data as `signed 32 bytes floating-point numbers` to the audio device. |

{{< tip "warning" >}}
At the moment, Amplitude only supports **Int16** format.
{{< /tip >}}

## mixer

`type: object`<br/>`required`

The `mixer` property configures the Amplitude Mixer (Amplimix). It takes as value an object with the following properties:

#### active_channels

`type: uint`<br/>`required`

Specifies the maximum number of channels to render by Amplimix. It equals the number of audio samples simultaneously playing in the game. If the maximum number of channels is reached, Amplitude will prioritize the most important channels and virtualize the others.

#### virtual_channels

`type: uint`<br/>`required`

Specifies the maximum number of virtual channels to use in addition to active channels. Amplimix doesn't render virtual channels, but all the information about them are still tracked.

#### pipeline

`type: array`<br/>`required`

It's inside the `pipeline` setting you describe the graph in which audio data will be processed before to be sent to the audio device for rendering. This setting take as value an array of sound processor defnitions that will be applied to the audio data. For further explanation about how to setup a pipeline, see the [Pipeline & Sound Processors](../pipeline-and-sound-processors) guide.

## game

`type: object`<br/>`required`

This setting is used by Amplitude to understand how it should synchronize with the game, how to handle game and sound objects, and how to render sounds in the game environment. This is achieved by specifying a set of inner values in this setting.

### listener_fetch_mode 

`type: enum`<br/>`default: Nearest`

Since Amplitude allows you to define many listeners at the same time, but only one can actually render data for a single sound object, this setting allows you to define how the engine will pick the right listener for each rendered sound object. Available values are:

| ID | Name     | Description                                                                      |
|----|----------|----------------------------------------------------------------------------------|
| 0  | None     | Do not fetch for listeners. This mute all audio, but keeps processing data.      |
| 1  | Nearest  | Fetches for the listener nearest to the currently processed sound.               |
| 2  | Farthest | Fetches for the listener farest to the currently processed sound.                |
| 2  | Default  | Always use the default listener set in the engine at runtine for every playback. |
| 2  | First    | Always use the first available listener of the list for every playback.          |
| 2  | Last     | Always use the last available listener of the list for every playback.           |

By using `Nearest` or `Farthest`, different listeners may be used at the same time for each playback. Using `Default`, `First`, or `Last` ensure that only one listener is used for every playback.

#### listeners

`type: uint`<br/>`default: 1`

The `listeners` property specifies the maximum number of listeners to pre-allocate. You will not be able to create more [Listener] objects than that value at runtime.

#### entities

`type: uint`<br/>`default: 4096`

The `entities` property specifies the maximum number of game entities to pre-allocate. This value does not represent all the entities of your game or scene but only the approximate number of entities managed by the Amplitude Engine. You will not be able to create more [Entity] objects than that value at runtime.

#### environments

`type: uint`<br/>`default: 64`

The `environments` property specifies the maximum number of sound environment to pre-allocate. Sound environments are spaces in the game 3D environment where Amplitude may process specific effects and attenuation models. You will not be able to create more [Environment] objects than that value at runtime.

#### sound_speed

`type: float`<br/>`default: 333.0`

This property sets the speed of sound (in meters per seconds) in the game. This value will be used by the engine to process some effects like [the Doppler effect](https://en.wikipedia.org/wiki/Doppler_effect). If the value is not defined, it will default to `333.0`, which is the approximated value of the real speed of sound.

#### doppler_factor

`type: float`<br/>`default: 1.0`

This property takes a float value greater or equal to `0.0`. It will affect how much power is given to the Doppler effect. A value of `0.0` will disable the Doppler effect, while a value of `1.0` will render it as it should. Any other value will affect the sound pitch.

#### up_axis

`type: enum`<br/>`default: Y`

This property may help you not perform additional math while integrating Amplitude in your game or your game engine. It specifies the up axis you use in your game. Possible values are:

| ID  | Name |
| --- | ---- |
| 0   | Y    |
| 1   | Z    |

This setting will only affect how math operations on vectors and matrices are performed internally.

#### obstruction

`type: object`<br/>`required`

The `obstruction` property let you setup the way Amplitude will compute sound obstruction in the game. It takes as value an object with the following properties:

- **lpf_curve**: Set the Low-Pass Filter curve for the obstruction sound processor.
- **gain_curve**: Set the gain curve for the obstruction sound processor.

To learn more about how to create curves, you can read the [Building Curves](../building-curves) guide.

#### occlusion

`type: object`<br/>`required`

The `occlusion` property works exactly the same as the `obstruction` property, but it's used instead to instruct Amplitude how to process sound occlusion in the game.

#### track_environments

`type: boolean`<br/>`default: true`

Defines whether the game is tracking environments. This means that the game will compute and send the environment amounts to the engine. This implies that the shapes defined in environments (if any) will not be used.

Setting this value to `false` will instruct Amplitude to track environment amounts by himself.

## buses_file

`type: string`<br/>`required`

The `buses_file` property defines the path to the binary (`.ambus`) file that contains the buses definitions.

## driver

`type: string`<br/>`required`

The `driver` property indicates the name of the audio [Driver] implementation communicating with the physical audio device. You can implement multiple audio drivers as needed and register them in the engine. Read the [Writing Drivers](../../advanced/writing-drivers) page to learn more about them.

## Example

The following example describes a simple engine configuration file:

```json
{
  "output": {
    "frequency": 48000,
    "channels": "Stereo",
    "buffer_size": 2048,
    "format": "Int16"
  },
  "mixer": {
    "active_channels": 50,
    "virtual_channels": 100
  },
  "listeners": 1,
  "entities": 10,
  "buses_file": "assets/buses.ambus"
}
```
