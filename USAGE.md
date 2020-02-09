# Usage #

DCCU is a command line tool. You will probably want to copy the tool to the audio directory of the DCC-Studio program.

The command syntax is as follows:

> DCCU sourcefile [sourcefile...]

Every file name on the command line is interpreted as a source file name. The name must end in ".MPP" or ".MP1". The program opens each file and creates an output file with the same base file name but a different extension: MPP is converted to MP1 and MP1 is converted to MPP.

#### IMPORTANT: Please read the following instructions on how to successfully convert a track to MP1 and how to import a converted MPP file without crashing DCC-Studio. ####

# Importing an MP1 file into DCC-Studio #

If you want to use an MP1 file with DCC-Studio, you will need to go through a few steps to convert it to a "trinity" of a .TRK file, an MPP file and a .LVL file. These extra steps won't be necessary in a future version of DCCU.

1. Open DCC-Studio and click the Extra>Options menu. Make sure the "Unused Audio Fragments" option is set to "Show at startup", and "Every 1 sessions" [sic]. Click OK. Then close DCC-Studio.
1. Use the DCCU program to convert the MP1 file that you want to record to tape, to MPP. Keep in mind that DCC-Studio is a 16 bit program that doesn't understand long filenames. It uses "AF000000.MPP" (where 000000 is replaced by a number) for its own file names but you don't have to follow this pattern; however, 8-character file names (or less) are recommended so you don't have to figure out what the short filename is for your long file name.
1. Copy the MPP file to the DCC Studio audio directory (usually C:\STUDIO\AUDIO).
1. Without a .LVL file, DCC-Studio crashes if you want to open your file. To prevent this, you can reuse an existing .LVL file but it must have enough data in it. A .LVL file that belongs to an entire (side of) a tape will do fine. In the DCC-Studio audio directory, search for the largest file with an LVL extension and make a copy so that the copy has a base file name that matches your MPP file.
1. Open DCC-Studio. It will tell you that it found unused audio fragments. Select your file and click "Create track".
1. Then it will ask for a file name for the .TRK file which it will generate. You can also enter the track title and artist name here.

Now you can use the newly imported track the same way as you would use other DCC-Studio tracks. However, the wave form that's shown in the editor won't match the actual sound. You can record the file to tape and then re-record it to hard disk to get a copy (and the copy will be exact: the DCC recorder doesn't make a change to it). Once DCC-Studio reads the wave from tape, the waveform on the screen will match what you hear.

# Creating an MP1 File from a DCC-Studio Audio Track #

If you want to convert a DCC-Studio to an MP1 file, do the following. The number of steps for this procedure will also be reduced in a future version.

1. Use DCC-Studio to create an audio track using the Wave Editor. Edit the track as much as you like. NOTE: Placing markers will have no effect.
1. Close and save the track you edited, and open the File Manager from the Extra menu.
1. In the File Manager, click the file you want to convert and select "Save Audio". This saves the audio from the track as a single continuous MPP file.
1. In the DCC-Studio audio directory, you can use dir /od to find out what the last-written MPP file name is. Or you can open the .TRK file with Notepad and see if you can make out the file name from the gibberish, Or you can open the track file in DCC-Studio and select Editor>Show Audio Files. You should see a single file name.
1. Use the DCCU command with the .MPP file as command line argument. It will generate an MP1 file with the same base name but with an MP1 extension (e.g. AF000001.MPP will be converted to AF000001.MP1). The MP1 file can be played in most media players such as VLC or Windows Media Player (including the version of Media Player that's included in Windows 98).
