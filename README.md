<div align="center">

![Logo](resources/logo.png)

[![FOSSA Status](https://app.fossa.io/api/projects/git%2Bgithub.com%2FTalonBraveInfo%2Fplatform.svg?type=shield)](https://app.fossa.io/projects/git%2Bgithub.com%2FTalonBraveInfo%2Fplatform?ref=badge_shield) [![CodeFactor](https://www.codefactor.io/repository/github/oldtimes-software/hei/badge)](https://www.codefactor.io/repository/github/oldtimes-software/hei)

[Features](#features) | [Users](#users) | [License](#license)

</div>

----

A collection of relatively small utility libraries written in C11 that can be used as the foundation for various applications, though probably more oriented towards games.
Includes APIs for dealing with images, graphics, models, IO and much more.

Originally created around 2014/2015 as a support library for another game, it's since found
a new life supporting [APE Tech](https://www.hogsy.me/p/yin.html) and other projects.
Do let me know if you find any use of it, and I'll give you a shoutout!

**Keep in mind this is likely far from the most efficient library in the world.
If performance is a big priority for you, then I would highly recommend looking elsewhere!**

## Features

### plcore
- Support for multiple compression algorithms
- PCMD command-line interface, that exposes features the library has to offer
- Plugin interface for supporting new image and package formats
- C/C++ Math Library (Vector, Matrix, Quaternion)
    - Also provides optional OpenGL-style matrix functions
- Console Interface, with logging, variables and commands
- File I/O is endianness agnostic
- Support for mounting packages; I/O requests will automatically be mapped to any 
mounted packages before falling back to the local FS
- Image API provides manipulation functions
- Supported Image Formats
  - PNG (*read, write*)
  - BMP (*read, write*)
  - GIF (*read*)
  - TGA (*read, write*)
  - PSD (*read*)
  - JPG (*read, write*)
  - HDR (*read*)
  - PIC (*read*)
  - PNM (*read*)
  - 3dfx 3DF (*read*)
  - Monolith DTX (*read*)
  - Ritual FTX (*read*)
  - Ritual SWL (*read*)
  - Sony TIM (*read*)
  - Valve VTX (via plugin) (*read*)
  - Herdy Gerdy HGT (*read*)
  - Angel Engine TEX (*read*)
  - 3D Realms' TEX (*read*)
  - Red Storm Entertainment RSB Format (*read*)
  - Can easily be extended to support other image formats either via the API or via plugins

Below is a table of all supported package formats.

| Format       | Usages                                              | Support Level | Mode |
|--------------|-----------------------------------------------------|---------------|------|
| ZIP          | General package container format                    | Core          | R    |
| MAD/MTD      | Hogs of War, Actua Soccer                           | Core          | R    |
| VPP          | Red Faction, Red Faction II, Summoner, The Punisher | Core          | R    |
| WAD          | Doom, Quake, Half-Life                              | Core          | R    |
| PAK          | Quake and Half-Life                                 | Core          | R    |
| Fresh BIN    | FreshEngine                                         | Core          | R    |
| GRP          | Duke Nukem 3D                                       | Core          | R    |
| VPK          | Vampire The Masquerade Bloodlines / Troika          | Core          | R    |
| CLU          | Herdy Gerdy                                         | Extra         | R    |
| IBF          | Iron Storm                                          | Extra         | R    |
| HAL          | Mortyr                                              | Extra         | R    |
| RID/RIM      | Eradicator                                          | Extra         | R    |
| FTactics PAK | Future Tactics                                      | Extra         | R    |
| Kri WAD      | The Mark of Kri                                     | Extra         | R    |
| WFear INU    | White Fear (Cancelled)                              | Core          | R    |
| Haven DAT    | Haven Call of the King                              | Extra         | R    |
| VSR          | Sentient                                            | Extra         | R    |
| OPK          | Outcast                                             | Core          | R    |
| AITD PAK     | Alone in the Dark                                   | Extra         | R    |
| Ice3D DAT    | BioShock 3D                                         | Extra         | R    |
| Angel DAT    | Angel Studios                                       | Extra         | R    |
| FF           | Outwars                                             | Extra         | R    |
| Blitz DAT    | Titan A.E., Chicken Run                             | Extra         | R    |
| SWAT WAD     | Okre Engine (SWAT Global Strike Team)               | Extra         | R    |

### plgraphics
- Provides a relatively simple abstraction layer
- Camera implementation, supporting Isometric, orthographic and perspective views
- Easy-to-use rendering API
- Supported APIs via plugins, aka 'drivers'
  - OpenGL 3.3
  
### plmodel

- Model API supports static and animated per-vertex/skeletal formats
- Supported Model Formats
    - Cyclone MDL (*read*)
    - Triton HDV (*read*)
    - Unreal 3D (*read*)
    - Valve SMD (*write*)
    - Can easily be extended to support other model formats either via the API or via plugins

### Extras

Under the [extras](extras) directory, you'll find a number of source files for granting you support for various other formats. These are typically left out of the core library because they're either not widely used or support is considered experimental. In summary, if you're developing a larger project, I would highly recommend excluding these.

## Users

- [APE Tech](https://hogsy.itch.io/ape-tech), a 3D game engine written in C
- [OpenHoW](https://github.com/TalonBraveInfo/OpenHoW), a reimplementation of Hogs of War

## License

Project is licensed under [MIT](LICENSE).

Additional licences for libraries used by `plcore` can be found [here](docs/plcore).
