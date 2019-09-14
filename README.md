## Pepper 
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
* Built with **/std:c++17** conformance

![](docs/img/Pepper_Dos.JPG)
![](docs/img/Pepper_Imp.JPG)
![](docs/img/Pepper_Resources.JPG)

## License
This software is available under the **"MIT License"**.

## Links
[Latest release page](https://github.com/jovibor/Pepper/releases/latest)  
[Microsoft Visual C++ Redistributable for Visual Studio 2019 might be needed.](https://aka.ms/vs/16/release/VC_redist.x86.exe)

## [](#)Help Point
If you would like to help the author in further project's development you can do it in form of donation:

[![paypal](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=M6CX4QH8FJJDL&currency_code=USD&source=url)