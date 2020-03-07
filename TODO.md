# TO DO #

- Prevent accidental overwriting of existing output files
- Automatic generation of short file names
- Use the DCC-Studio INI file to determine path for MPP/LVL/TRK output files
- Read and interpret TRK files to determine which fragments of which MPP file need to be converted
- Support for splitting MPP files that have multiple sample rates (these are generated when recording files from tapes with mixed sample rates, but cannot be played due to a bug in DCC-Studio)
- Improved buffering for improved speed
- Better error messages
- Warnings for problems such as output files that already exist
- Support for wildcards in file names
- Compilation file generator based on markers in a .TRK file