## Pepper 
## Installation
`git clone https://github.com/jovibor/Pepper.git --recursive`

## Features

#### [PE/PE+](https://docs.microsoft.com/en-us/windows/win32/debug/pe-format) binaries viewer, built on top of [libpe](https://github.com/jovibor/libpe).
* Supports PE(x86) and PE+(x64) binaries of any size
* All inner PE/PE+ data structures, headers and layouts
    * MSDOS Header
    * «Rich» Header
    * NT/File/Optional Headers
    * Data Directories
    * Sections
    * Export Table
    * Import Table
    * Resource Table
    * Exceptions Table
    * Security Table
    * Relocations Table
    * Debug Table
    * TLS Table
    * Load Config Directory
    * Bound Import Table
    * Delay Import Table
    * COM Table
* Resources viewer
* Multiple binaries can be browsed simultaneously
* Ability to edit inner structures through hex editor

![](docs/img/Pepper_Dos.JPG)
![](docs/img/Pepper_Imp.JPG)
![](docs/img/Pepper_Resources.JPG)

## License
This software is available under the **"MIT License"**.
