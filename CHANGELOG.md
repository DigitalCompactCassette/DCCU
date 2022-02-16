# Change Log #

2020-01-23 Created.<br>
2020-02-08 Initial release.<br>
2020-03-06 Added code to generate .TRK and (dummy) .LVL files from MP1 files.<br>
2020-03-12 Added support for compiling with Visual Studio 6.0.<br>
2022-01-08 Fixed problem when handling MP1 files with CRC's<br>
2022-01-08 When converting 44.1 kHz MP1 to MPP, the program now clears the padding slots. The DCC spec says these are "dummy" slots but they may contain data in MP1 files.<br>
2022-02-15 Modified to handle long file names correctly<br>
