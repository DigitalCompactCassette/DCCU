# Usage #

DCCU is a command line tool. You will probably want to copy the tool to the audio directory of the DCC-Studio program.

The command syntax is as follows:

> DCCU sourcefile [sourcefile...]

Every file name on the command line is interpreted as a source file name. The name must end in ".MPP" or ".MP1". The program opens each file and creates an output file with the same base file name but a different extension: MPP is converted to MP1 and MP1 is converted to MPP/LVL/TRK.

# Importing an MP1 file into DCC-Studio #

1. Make sure the file you want to convert has only 8 characters in the base file name, and preferably no spaces or special characters. DCCU currently uses the input file name to generate the output file name but only changes the file extension; this can give problems because DCC-Studio is not capable of handling long file names. Also make sure the file name doesn't correspond to any existing .TRK or .LVL or .MPP files in the DCC-Studio audio directory.
2. Use the DCCU program to convert the MP1 file that you want to record to tape, to MPP/LVL/TRK, using e.g. `DCCU FILENAME.MP1`.
1. Either copy the MPP, LVL and TRK files to the DCC Studio audio directory (usually C:\STUDIO\AUDIO), or use the Import option from the Extra menu. The latter is slower but it will generate new non-conflicting file names and it will ask you for the new track name, artist name and track title.

#### Please note: If you open the track file in an edit window, the wave form will not reflect the actual audio. This is normal and cannot be fixed unless MP1 decoding is added to the DCCU application.

The audio is correct and can be played, edited and recorded to tape as usual. If you want, you can record the audio to tape and then copy it back to hard disk to get an accurate waveform representation. Copying to tape and then back to hard disk will not result in loss of quality.

# Creating an MP1 File from a DCC-Studio Audio Track #

If you want to convert a DCC-Studio to an MP1 file, do the following. The number of steps for this procedure will also be reduced in a future version.

1. Use DCC-Studio to create an audio track using the Wave Editor. Edit the track as much as you like. NOTE: Placing markers will have no effect.
1. Close and save the track you edited, and open the File Manager from the Extra menu.
1. In the File Manager, click the file you want to convert and select "Save Audio". This saves the audio from the track as a single continuous MPP file.
1. In the DCC-Studio audio directory, you can use dir /od to find out what the last-written MPP file name is. Or you can open the .TRK file with Notepad and see if you can make out the file name from the gibberish, Or you can open the track file in DCC-Studio and select Editor>Show Audio Files. You should see a single file name.
1. Use the DCCU command with the .MPP file as command line argument. It will generate an MP1 file with the same base name but with an MP1 extension (e.g. AF000001.MPP will be converted to AF000001.MP1). The MP1 file can be played in most media players such as VLC or Windows Media Player (including the version of Media Player that's included in Windows 98).
