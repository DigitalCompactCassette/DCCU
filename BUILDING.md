# Building #

The program was written in C, using Microsoft Visual Studio 10. Newer versions of Visual Studio can probably also be used but that's what I had available. The program is linked against the static C runtime libraries so that it's not necessary to install the Visual Studio Runtime on the system where you want to use this.

DCCU is a 32 bit console application which was designed to run under Windows 98 and later. It will not run in Windows 3.1 or earlier (though DCC-Studio was designed for Windows 3.1) though maybe it can be made to run with the Win32s extensions installed. I didn't test this because I don't have a system that runs Windows 3.x and I expect that most users of DCC-Studio run Windows 98 anyway. If necessary, it should be possible to compile the program with Visual C++ 1.5 as a DOS 16-bit application (maybe with a few minor modifications to change the safe C string library function calls to their unsafe equivalents). I haven't tried building with VC++ 1.5 but I may do so in the future.

To build the program, open the SLN file in Visual Studio 2010 or later, and build. There is only one project and one source file, so there's nothing much else to say about it.

There is no installer and I don't have plans to build one, unless I change DCCU to have a Windows user interface one day.
