# Platform Library
[![FOSSA Status](https://app.fossa.io/api/projects/git%2Bgithub.com%2FTalonBraveInfo%2Fplatform.svg?type=shield)](https://app.fossa.io/projects/git%2Bgithub.com%2FTalonBraveInfo%2Fplatform?ref=badge_shield) [![CodeFactor Status](https://www.codefactor.io/Content/badges/D.svg)](https://www.codefactor.io/repository/github/talonbraveinfo/platform)

A relatively small utility library that can be used as the foundation for various applications. 
Includes APIs for dealing with images, graphics, models, IO and much more.

Usually best used alongside SDL2 or other similar solutions.

## Features

- PCMD command-line interface, that exposes features the library has to offer
- Plugin interface for supporting new image, model and package formats
- C/C++ Math Library (Vector, Matrix, Quaternion)
    - Also provides optional OpenGL-style matrix functions
- Console Interface, with logging, variables and commands
- File I/O is endianness agnostic
- Support for mounting packages; I/O requests will automatically be mapped to any 
mounted packages before falling back to the local FS
- Graphics Abstraction Layer
    - Camera implementation, supporting Isometric, orthographic and perspective views
    - Easy-to-use rendering API
    - Supported APIs
        - OpenGL 3.3
        - OpenGL 1.0
        - Vulkan (*planned*)
        - Software (*planned*)
        - 3dfx Glide (*planned :)* )
- Model API supports static and animated per-vertex/skeletal formats
- Supported Model Formats
    - Cyclone MDL (*read*)
    - Triton HDV (*read*)
    - Unreal 3D (*read*)
    - Valve SMD (*write*)
    - Can easily be extended to support other model formats either via the API or via plugins
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
    - Can easily be extended to support other image formats either via the API or via plugins
- Supported Package Formats
    - IBF (Iron Storm)
    - RID/RIM (Eradicator)
    - WAD (Doom)
    - FF (Outwars)
    - MAD/MTD (Hogs of War, Actua Soccer)
    - HAL (Mortyr)
    - SFA (SF Adventures)
    - VSR (Sentient)
    - Can easily be extended to support other package formats either via the API or via plugins
    
## Roadmap

- Rename library to Hei (conventions will remain pl/PL)
- Windowing API (?)
