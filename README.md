<div align="center">

![Logo](resources/logo.png)

[![FOSSA Status](https://app.fossa.io/api/projects/git%2Bgithub.com%2FTalonBraveInfo%2Fplatform.svg?type=shield)](https://app.fossa.io/projects/git%2Bgithub.com%2FTalonBraveInfo%2Fplatform?ref=badge_shield)
[![CodeFactor](https://www.codefactor.io/repository/github/quartermindgames/hei/badge)](https://www.codefactor.io/repository/github/quartermindgames/hei)

[Features](#features) | [Users](#users) | [License](#license)

</div>

----

A collection of relatively small utility libraries written in C11 that can be used as the foundation for various applications, though probably more oriented towards games.
Includes APIs for dealing with images, graphics, models, IO and much more.

Originally created around 2014/2015 as a support library for another game, it's since found
a new life supporting [ApeTech](https://www.hogsy.me/ape.htm) and other projects.
Do let me know if you find any use of it, and I'll give you a shoutout!

**Keep in mind this is likely far from the most efficient library in the world.
If performance is a big priority for you, then I would highly recommend looking elsewhere!**

# Features

## plcore
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

### Supported image formats

| Format            | Usages                      | Support Level | Mode |
|-------------------|-----------------------------|---------------|------|
| PNG               |                             | Core          | R/W  |
| BMP               |                             | Core          | R/W  |
| GIF               |                             | Core          | R    |
| TGA               |                             | Core          | R/W  |
| PSD               |                             | Core          | R    |
| JPG               |                             | Core          | R/W  |
| HDR               |                             | Core          | R    |
| DDS               |                             | Core          | R    |
| QOI               |                             | Core          | R/W  |
| 3dfx 3DF          |                             | Core          | R    |
| Monolith DTX      | LithTech (game engine)      | Core          | R    |
| Ritual FTX        |                             | Core          | R    |
| Ritual SWL        |                             | Core          | R    |
| Sony TIM          |                             | Core          | R    |
| 3D Realms TEX     |                             | Core          | R    |
| Angel Studios TEX | Angel Engine (game engine)  | Core          | R    |
| Red Storm RSB     |                             | Core          | R    |
| Valve VTX         | Source Engine (game engine) | Plugin        | R    |
| Core HGT          | Herdy Gerdy                 | Extra         | R    |

### Supported package formats

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
| WFear INU    | White Fear (Cancelled)                              | Core          | R    |
| OPK          | Outcast                                             | Core          | R    |
| Amuze AHF    | Headhunter                                          | Core          | R    |
| Amuze AFS    | Headhunter                                          | Core          | R    |
| Angel DAT    | Angel Studios                                       | Core          | R    |
| CLU          | Herdy Gerdy                                         | Extra         | R    |
| IBF          | Iron Storm                                          | Extra         | R    |
| HAL          | Mortyr                                              | Extra         | R    |
| RID/RIM      | Eradicator                                          | Extra         | R    |
| FTactics PAK | Future Tactics                                      | Extra         | R    |
| Kri WAD      | The Mark of Kri                                     | Extra         | R    |
| AITD PAK     | Alone in the Dark                                   | Extra         | R    |
| Ice3D DAT    | BioShock 3D                                         | Extra         | R    |
| FF           | Outwars                                             | Extra         | R    |
| Blitz DAT    | Titan A.E., Chicken Run                             | Extra         | R    |
| SWAT WAD     | Okre Engine (SWAT Global Strike Team)               | Extra         | R    |
| Haven DAT    | Haven Call of the King                              | Extra         | R    |
| VSR          | Sentient                                            | Extra         | R    |

## plgraphics
- Provides a relatively simple abstraction layer
- Camera implementation, supporting Isometric, orthographic and perspective views
- Easy-to-use rendering API
- Supported APIs via plugins, aka 'drivers'
  - OpenGL 3.3
  
## plmodel

- Model API supports static and animated per-vertex/skeletal formats
- Supported Model Formats
    - Cyclone MDL (*read*)
    - Triton HDV (*read*)
    - Unreal 3D (*read*)
    - Valve SMD (*write*)
    - Can easily be extended to support other model formats either via the API or via plugins

## Extras

Under the [extras](extras) directory, you'll find a number of source files for granting you support for various other formats. These are typically left out of the core library because they're either not widely used or support is considered experimental. In summary, if you're developing a larger project, I would highly recommend excluding these.

# Users

- [ApeTech](https://www.hogsy.me/ape.htm), a 3D game engine written in C
- [OpenHoW](https://github.com/TalonBraveInfo/OpenHoW), a reimplementation of Hogs of War

# License

The project is licensed under [MIT](LICENSE).

Additional licences for libraries used by `plcore` can be found [here](docs/plcore).
