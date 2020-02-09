# DCCU #

The DCC Utility (DCCU) can be used to convert DCC-Studio .MPP files to .MP1 (MPEG 1 Layer 1), or convert .MP1 files to MPP. Unlike its predecessor "DCCConv", it can convert in both directions, and it correctly supports files encoded at 32, 44.1 and 48 kHz (NOTE: DCC is limited to a bit rate of 384 kbps only).

The program is a command line tool intended to be used on Windows 98 or later. It doesn't work on Windows 3.x.

Please refer to the other .md files in this directory for more documentation.

# Quick Start
Syntax:

> DCCU inputfilename [inputfilename...]

Every name on the command line is interpreted as an input file name. The program automatically generates the output file name by replacing the file name extension of the input file: MPP is converted to MP1 and MP1 is converted to MPP.

#### IMPORTANT: Read the USAGE.md file for important instructions on how to successfully convert a DCC-Studio track to MP1, and import a converted MP1 into DCC-Studio without crashing. ####
