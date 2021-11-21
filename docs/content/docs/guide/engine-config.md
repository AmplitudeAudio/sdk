---
title: Engine Configuration
weight: 6
---

{{< block >}}
{{< column "mb-1" >}}

You can change the behavior of Amplitude by tweaking the project's configuration file. This file will help you customize:

- The playback device ([output])
- The Amplitude Mixer, called **Amplimix** ([mixer])
- The number of Listener objects to pre-allocate ([listeners])
- The number of Entity objects to pre-allocate ([entities])
- The path to the buses file ([buses_file])
- The name of the driver implementation to use ([driver])

[output]: #output
[mixer]: #mixer
[listeners]: #listeners
[entities]: #entities
[buses_file]: #buses_file
[driver]: #driver

{{< /column >}}
{{< /block >}}

## output

The `output` property helps you to define how Amplitude should communicate with the physical audio device. It takes as value a map with the following properties.

#### frequency

The `frequency` property defines the audio frequency in Hertz (`Hz`) of the audio data sent to the audio device by Amplitude.

#### channels

The `channels` property sets the number of channels Amplitude will output. It can take as value the name of that channel or the number of channels. Possible values are:

| Channel Type | Description | Mapping | Value |
|-|-|-|-|
| Default | Takes the default value from the [Driver]. In most cases the default value is Stereo. | According to the default value. | Number:&nbsp;0<br/>String:&nbsp;Default |
| Mono | Compute and output audio data in one channel. | 0:&nbsp;FRONT&nbsp;CENTER | Number:&nbsp;1<br/>String:&nbsp;Mono |
| Stereo | Compute and output audio data in 2 channels. | 0:&nbsp;FRONT&nbsp;LEFT<br/>1:&nbsp;FRONT&nbsp;RIGHT | Number:&nbsp;2<br/>String:&nbsp;Stereo |
| Quad | Compute and output audio data in 4 channels. | 0:&nbsp;FRONT&nbsp;LEFT<br/>1:&nbsp;FRONT&nbsp;RIGHT<br/>2:&nbsp;FRONT&nbsp;CENTER<br/>3:&nbsp;BACK&nbsp;CENTER | Number:&nbsp;4<br/>String:&nbsp;Quad |
| 5.1 | Compute and output audio data in 6 channels. | 0:&nbsp;FRONT&nbsp;LEFT<br/>1:&nbsp;FRONT&nbsp;RIGHT<br/>2:&nbsp;FRONT&nbsp;CENTER<br/>3:&nbsp;LFE<br/>4:&nbsp;SIDE&nbsp;LEFT<br/>5:&nbsp;SIDE&nbsp;RIGHT | Number:&nbsp;6<br/>String:&nbsp;Surround_5_1 |
| 7.1 | Compute and output audio data in 8 channels. | 0:&nbsp;FRONT&nbsp;LEFT<br/>1:&nbsp;FRONT&nbsp;RIGHT<br/>2:&nbsp;FRONT&nbsp;CENTER<br/>3:&nbsp;LFE<br/>4:&nbsp;BACK&nbsp;LEFT<br/>5:&nbsp;BACK&nbsp;RIGHT<br/>6:&nbsp;SIDE&nbsp;LEFT<br/>7:&nbsp;SIDE&nbsp;RIGHT | Number:&nbsp;8<br/>String:&nbsp;Surround_7_1 |

{{< tip "warning" >}}
At the moment, Amplitude only supports **Stereo** channels. Setting the engine to use something other than Stereo will result in errors.
{{< /tip >}}

## mixer

The `mixer` property is used to configure the Amplitude Mixer (Amplimix). It takes as value an object with the following properties:

#### active_channels

Specifies the maximum number of channels to process during mixing. It is equal to the number of audio samples simultaneously playing in the game. If the maximum number of channels is reached, Amplitude will prioritize the most important channels and virtualize the others.

#### virtual_channels

Specifies the maximum number of virtual channels in addition to active channels. Amplimix doesn't process virtual channels, but all the information about them is still tracked.

## listeners

The `listeners` property specifies the maximum number of listeners to pre-allocate. You will not be able to create more [Listener] objects than that value during runtime.

## entities

The `entities` property specifies the maximum number of game entities to pre-allocate. This value does not represent all the entities of your game or scene but only the approximate number of entities managed by the Amplitude Engine. You will not be able to create more [Entity] objects than that value during runtime.

## buses_file

The `buses_file` property defines the path to the binary (`.ambus`) file that contains the buses definitions.

{{< tip "warning" >}}
This property is mandatory and should be defined.
{{< /tip >}}

## driver

The `driver` property indicates the name of the audio [Driver] implementation. It is used for communicating with the physical audio device. You can implement multiple audio drivers as needed and register them in the engine, read the [Writing Drivers](../../advanced/writing-drivers) page to learn more about them.

