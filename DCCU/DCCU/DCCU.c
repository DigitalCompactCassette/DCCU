/* 
  DCC Utility
  (C) 2020 Jac Goudsmit

  This program converts MPP files from DCC-Studio to MP1 (MPEG 1 Layer 1)
  and vice versa.

  It was written as a console application for Windows 98.

  Licensed under the MIT license. See the LICENSE file for licensing terms.
*/

#include <windows.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>


/////////////////////////////////////////////////////////////////////////////
// MACROS
/////////////////////////////////////////////////////////////////////////////


// The buffer sizes need to be at least the size of the largest possible
// frame size.
#define INPUT_BUFFER_SIZE (48000)       // One second of MP1 data
#define OUTPUT_BUFFER_SIZE (48000)      // Same as input buffer size


/////////////////////////////////////////////////////////////////////////////
// TYPES
/////////////////////////////////////////////////////////////////////////////


//---------------------------------------------------------------------------
// Types that evidently don't exist in early versions of the Windows SDK
#ifndef _LPCBYTE_DEFINED
#define _LPCBYTE_DEFINED
typedef const BYTE *LPCBYTE;
#endif


//---------------------------------------------------------------------------
// Enum type that corresponds to DCC-Studio file header value
//
// Do not change; these are used in MPP file headers and must correspond to
// what DCC-Studio does.
typedef enum RATEID_t
{
  RATEID_UNKNOWN = 0,                   // Used internally
  RATEID_32000 = 32,                    // 32 kHz
  RATEID_44100 = 44,                    // 44.1 kHz
  RATEID_48000 = 48,                    // 48 kHz

} RATEID;


//---------------------------------------------------------------------------
// Enum type for error codes
typedef enum ERR_t
{
  ERR_OK = 0,                           // No problem
  ERR_COMMAND,                          // Command line error
  ERR_PARAMETER,                        // Internal parameter error
  ERR_INTERNAL,                         // Internal error
  ERR_MALLOC,                           // Allocation error
  ERR_SYNC,                             // Input doesn't start with sync
  ERR_INSUFFICIENT_DATA,                // Not enough input data
  ERR_DATA_NOT_MPEG1,                   // Input is not MPEG1
  ERR_DATA_NOT_LAYER1,                  // Input is not layer 1
  ERR_DATA_NOT_384KBPS,                 // Input is not 384 kbps
  ERR_DATA_BAD_CHANMODE,                // Mono is not supported by DCC
  ERR_DATA_BAD_SAMPLERATE,              // Unsupported sample rate in input
  ERR_SAMPLERATE_MISMATCH,              // Input rate doesn't match output
  ERR_INPUT_FILE_NAME,                  // Input file name is invalid
  ERR_INPUT_FILE_OPEN,                  // Couldn't open input file
  ERR_INPUT_FILE_READ,                  // Error reading input file
  ERR_INPUT_FILE_EOF,                   // End of input file reached
  ERR_OUTPUT_FILE_OPEN,                 // Couldn't open output file
  ERR_OUTPUT_FILE_WRITE,                // Error writing output file

  ERR_NUM                               // (number of errors)
} ERR;


//---------------------------------------------------------------------------
// Struct type representing the input stream
typedef struct INPUTSTREAM_t
{
  // Handle for file operations
  FILE             *fin;                // Input file handle

  // The buffer is divided in three parts:
  // - The start of the buffer is discardable data
  // - This is followed by the current MPEG frame (or part of it)
  // - The end of the buffer may be unusable data (e.g. past end of file)
  UINT              startindex;         // Index to start of frame
  UINT              endindex;           // Index to end of usable data

  // When a valid frame of data is found in the input buffer,
  // the following are set to the attributes of the frame.
  RATEID            rateid;             // Sample rate
  size_t            framesize;          // Frame size

  // The actual buffer follows the struct once it's allocated.
  UINT              buffersize;         // Number of bytes in buffer
  BYTE              buffer[0];          // Input buffer follows struct
} INPUTSTREAM, *HINPUTSTREAM;


typedef struct OUTPUTSTREAM_t
{
  CHAR              filename[MAX_PATH]; // File name for output
  FILE             *fout;               // Output file handle
  FILE             *flvl;               // Level file handle
  FILE             *ftrk;               // Track file handle
  BOOL              is_mpp;             // TRUE=MPP FALSE=MP1
  RATEID            rateid;             // Rate ID for MPP file
  UINT32            numframes;          // Number of frames generated

} OUTPUTSTREAM, *HOUTPUTSTREAM;


/////////////////////////////////////////////////////////////////////////////
// CODE
/////////////////////////////////////////////////////////////////////////////


//---------------------------------------------------------------------------
// Get the extension of a file name
//
// Returns the location in the input string of the period in front of the
// filename extension.
//
// If the name doesn't end in an extension, the function returns the location
// of the end of the string.
//
// If the name includes a directory with a path with an extension, the
// function handles this correctly.
//
// If the file name is NULL, the function returns NULL.
LPCSTR                                  // Returns extension; NULL on error
GetFileExtension(
  LPCSTR filename,                      // Input file name
  LPCSTR *pbasename)                    // Optional output base file name
{
  LPCSTR result = NULL;
  LPCSTR basename = filename;

  if (filename)
  {
    LPCSTR s;

    for (s = filename; *s; s++)
    {
      if (*s == '.')
      {
        result = s;
      }
      else if ((*s == '\\') || (*s == '/') || (*s == ':'))
      {
        basename = s + 1;
        result = NULL;
      }
    }

    if (!result)
    {
      // If no period was found (or the last period appeared in front of
      // a slash or backslash), return the end of the string
      result = s;
    }
  }

  if (pbasename)
  {
    *pbasename = basename;
  }

  return result;
}


//---------------------------------------------------------------------------
// Replace the extension of a file if it matches
//
// Extension parameters to this function don't include the period.
//
// If the extension doesn't match, the input string is not copied to the
// output buffer.
BOOL                                    // Returns TRUE if replacement done
ReplaceFileExtension(
  LPCSTR filename,                      // Input filename
  LPSTR outfilename,                    // Output filename (ok to use input)
  LPCSTR matchext,                      // Input ext must be this (NULL=any)
  LPCSTR replaceext)                    // Replace with this (NULL=no change)
{
  BOOL result = TRUE;
  LPCSTR extension = NULL;
  UINT32 baselen = 0;
  BOOL haveperiod = FALSE;

  if (result)
  {
    // Make sure we have an input filename
    if ((!filename) || (!*filename) || (!outfilename))
    {
      result = FALSE;
    }
  }

  if (result)
  {
    extension = GetFileExtension(filename, NULL);

    if (!extension)
    {
      // Something went wrong
      result = FALSE;
    }
  }

  if (result)
  {
    // Move the extension pointer beyond the period if there is one
    if (*extension == '.')
    {
      haveperiod = TRUE;
      extension++;
    }

    baselen = extension - filename;

    if (replaceext)
    {
      // If the replacement extension would make the output file too long,
      // bail out
      if (baselen + !haveperiod + strlen(replaceext) >= MAX_PATH)
      {
        result = FALSE;
      }
    }
  }

  if (result)
  {
    // If a match string was given, check if the extension matches
    if (matchext)
    {
      result = !stricmp(extension, matchext);
    }
  }

  if (result)
  {
    if (replaceext)
    {
      LPSTR dstex;

      // Check if we need to copy the string to the output
      if (filename != outfilename)
      {
        memcpy(outfilename, filename, baselen);
      }

      // Write a period if needed
      dstex = outfilename + baselen;
      if (!haveperiod)
      {
        *dstex++ = '.';
      }

      // Copy the replacement extension
      // We already checked that there is enough space.
      strcpy(dstex, replaceext); // Safe
    }
  }

  return result;
}


//---------------------------------------------------------------------------
// Determine MPEG-1 Layer 1 frame size based on header
//
// The frame must start with:
// 12 bits sync word 0xFFF                                \
// 1  bit  ID (1=MPEG)                                    | FF FF
// 2  bits layer (binary 11 for layer 1)                  |
// 1  bit  protection (=CRC added) (1=no)(ignored)        /
//
// 4  bits bit rate (1100 = 384kbps for layer 1)          \
// 2  bits sample freq (00=44kHz 01=48kHz 10=32kHz 11=res)| C0/C2 (44.1)
// 1  bit  indicates 44.1kHz frame is padded              | or C4 (48)
// 1  bit  private bit (ignored)                          / or C6 (32)
//
// 2  bits mode (00=stereo 01=joint 10=2ch 11=res./mono)  \
// 2  bits mode extension for joint stereo (ignored)      | Usually 0C
// 1  bit  copyright protected (1=yes, 0=no) (ignored)    | or 00
// 1  bit  original (1=original 0=copy) (ignored)         |
// 2  bits emphasis (00=none 01=50/15us 10=res. 11=CCITT) /
ERR                                     // Returns error code
GetFrameSize(
  PBYTE frame,                          // Pointer to start of frame
  UINT insize,                          // Number of available bytes
  PUINT pframesize,                     // Output required frame size
  RATEID *prateid)                      // Output sample rate enum
{
  ERR result = ERR_OK;
  UINT framesize = 0;
  RATEID rateid = RATEID_UNKNOWN;

  if (!result)
  {
    // The header must be at least 4 bytes
    if (insize < 4)
    {
      result = ERR_INSUFFICIENT_DATA;
    }
  }

  if (!result)
  {
    // The header must start with a sync word
    if ((frame[0] != 0xFF) || ((frame[1] & 0xF0) != 0xF0))
    {
      result = ERR_SYNC;
    }
  }

  if (!result)
  {
    // We can only do MPEG 1
    if ((frame[1] & 0x8) != 0x8)
    {
      result = ERR_DATA_NOT_MPEG1;
    }
  }

  if (!result)
  {
    // We can only do layer 1
    if ((frame[1] & 0x6) != 0x6)
    {
      result = ERR_DATA_NOT_LAYER1;
    }
  }

  if (!result)
  {
    // (frame[1] & 0x1) ignored (private bit)

    // The bit rate must be 384kbps
    if ((frame[2] & 0xF0) != 0xC0)
    {
      result = ERR_DATA_NOT_384KBPS;
    }
  }

  if (!result)
  {
    // The channel mode cannot be 11 binary (mono).
    // DCC doesn't support mono, only dual mono.
    if ((frame[3] & 0xC0) == 0xC0)
    {
      result = ERR_DATA_BAD_CHANMODE;
    }
  }

  if (!result)
  {
    // The frame size in bytes is 48 * 384000 / samplerate according to the
    // standard.
    //
    // For 44.1kHz frames, that calculation is not an integer so
    // some frames are shorter and some are longer.
    switch (frame[2] & 0x0C)
    {
    case 0x00:
      rateid = RATEID_44100;
      framesize = 416 + 4 * ((frame[2] & 0x2) != 0); // padding bit used only here
      break;

    case 0x04:
      rateid = RATEID_48000;
      framesize = 384;
      break;

    case 0x08:
      rateid = RATEID_32000;
      framesize = 576;
      break;

    default:
      result = ERR_DATA_BAD_SAMPLERATE;
      break;
    }
  }

  if (prateid)
  {
    *prateid = rateid;
  }

  if (pframesize)
  {
    *pframesize = framesize;
  }

  return result;
}


//---------------------------------------------------------------------------
// Destroy an output stream
void outputstream_Destroy(
  HOUTPUTSTREAM hso)                    // Output stream handle
{
  if (hso)
  {
    if (hso->fout)
    {
      fclose(hso->fout);
    }

    if (hso->flvl)
    {
      static CHAR lvldata[MAX_PATH];
      UINT32 u = hso->numframes;

      // Use the file name as dummy data
      strcpy(lvldata, hso->filename); // safe

      for (;;)
      {
        UINT32 x = sizeof(lvldata) / 2; // number of frames in dummy pattern

        if (u < x)
        {
          x = u;
        }

        if (!fwrite(lvldata, 2 * x, 1, hso->flvl)) // 2 bytes per frame
        {
          // TODO: Error
          break;
        }

        if (!(u -= x))
        {
          break;
        }
      }

      fclose(hso->flvl);
    }

    if (hso->ftrk)
    {
      LPCSTR basename;
      LPCSTR extension = GetFileExtension(hso->filename, &basename);

      if (extension)
      {
        UINT namelen = extension - basename;

        if (namelen > 8)
        {
          // TODO: Warning, name is too long
          //namelen = 8;
        }

        fprintf(hso->ftrk,
          "A-IO"
          "=5 Track{"
            "%u \"%s\""                 // Artist name (length first)
            "=5 Stack{"
              "1 4 ["
                "=8 Fragment{"
                  "%u \"%.*s\""         // Filename (length first)
                  "%u "                 // Number of frames
                  "0 "                  // Skip this many frames at start?
                "}"
              "]"
            "}"
            "c1 {0 0 []}"
            "%u "                       // sample rate ID
            "%u \"%.*s\""               // Title (length first)
            "%u "                       // Number of frames
          "}",

          4, "DCCU",                    // Artist name (TODO allow specifying from command line)
          namelen, namelen, basename,   // File name
          hso->numframes,               // Number of frames
          hso->rateid,                  // Sample rate ID
          namelen, namelen, basename,   // Title (TODO allow specifying from command line)
          hso->numframes                // Number of frames
        );
      }
      else
      {
        // TODO: Something went wrong parsing the file name. This shouldn't happen
      }

      fclose(hso->ftrk);
    }

    free(hso);
  }
}


//---------------------------------------------------------------------------
// Create an output stream
ERR
outputstream_Create(
  HOUTPUTSTREAM *phso,                  // Ptr to handle; must point to NULL
  LPCSTR filename,                      // File to open
  BOOL is_mpp)                          // TRUE=MPP, FALSE=MP1
{
  ERR result = ERR_OK;
  HOUTPUTSTREAM hs = NULL;

  if (!result)
  {
    if ((!phso) || (*phso) || (!filename) || (!*filename))
    {
      result = ERR_PARAMETER;
    }
  }

  if (!result)
  {
    if (!(hs = (HOUTPUTSTREAM)calloc(1, sizeof(OUTPUTSTREAM))))
    {
      result = ERR_MALLOC;
    }
  }

  if (!result)
  {
    strncpy(hs->filename, filename, sizeof(hs->filename) - 1); // safe; buffer is filled with \0
    hs->is_mpp = is_mpp;
    hs->rateid = RATEID_UNKNOWN;
    hs->numframes = 0;
  }

  if (phso)
  {
    *phso = hs;
  }

  return result;
}


//---------------------------------------------------------------------------
// Process a frame to an output stream
ERR                                     // Returns error code
outputstream_ProcessFrame(
  HOUTPUTSTREAM hso,                    // Output stream handle
  LPCBYTE buffer,                       // Frame to write
  size_t framesize,                     // Size of frame in bytes
  RATEID rateid)                        // Rate ID of frame
{
  ERR result = ERR_OK;

  if (!result)
  {
    if ((!hso) || (!buffer) || (rateid == RATEID_UNKNOWN))
    {
      result = ERR_PARAMETER;
    }
  }

  if (!result)
  {
    // If the file isn't open yet, open it now
    if (!hso->fout)
    {
      if (!(hso->fout = fopen(hso->filename, "wb")))
      {
        result = ERR_OUTPUT_FILE_OPEN;
      }
      else
      {
        // The file was opened successfully and we're at the start of it.
        if (hso->is_mpp)
        {
          // MPP files need a two byte header representing the sample
          // frequency so that DCC-Studio can seek in the file
          BYTE header[2];

          header[0] = (BYTE)rateid;
          header[1] = 0;

          if (!fwrite(header, sizeof(header), 1, hso->fout))
          {
            result = ERR_OUTPUT_FILE_WRITE;
          }
          else
          {
            hso->rateid = rateid;
          }

          // Generate a .TRK file too
          if (ReplaceFileExtension(hso->filename, hso->filename, NULL, "TRK"))
          {
            hso->ftrk = fopen(hso->filename, "w");
          }

          if (ReplaceFileExtension(hso->filename, hso->filename, NULL, "LVL"))
          {
            hso->flvl = fopen(hso->filename, "wb");
          }
        }
      }
    }
  }

  if (!result)
  {
    // Make sure the rate ID hasn't changed
    if (rateid != hso->rateid)
    {
      // MPP files can only have a single sample rate. For MP1 files,
      // changing the sample rate is okay.
      if (hso->is_mpp)
      {
        // TODO: Instead of returning an error, start a new MPP output file
        result = ERR_SAMPLERATE_MISMATCH;
      }
    }
  }

  if (!result)
  {
    // Write the buffer
    if (!fwrite(buffer, framesize, 1, hso->fout))
    {
      result = ERR_OUTPUT_FILE_WRITE;
    }
    else
    {
      // Write extra padding if necessary
      // This is only necessary for 44.1kHz in MPP files:
      // At 44.1kHz, the frame size can be 416 or 420 bytes because the
      // bit rate is not evenly divisible by the sample rate. DCC-Studio
      // pretends that all frames are 420 bytes to make it easier to seek
      // in the MPP file. We need to mimic that behavior.
      if ((hso->is_mpp) && (rateid == RATEID_44100) && (framesize == 416))
      {
        const static BYTE padding[4]; // static, so these are set to 0

        if (!fwrite(padding, sizeof(padding), 1, hso->fout))
        {
          result = ERR_OUTPUT_FILE_WRITE;
        }
      }
    }
  }

  if (!result)
  {
    // TODO: This could overflow but by the time that happens, DCC-Studio
    // TODO:   probably won't be able to read the file anyway because of
    // TODO:   other limitations such as a maximum file size of 2^32.
    hso->numframes++;
  }

  return result;
}


//---------------------------------------------------------------------------
// Destroy input stream
void
inputstream_Destroy(
  HINPUTSTREAM hsi)                     // Input stream handle
{
  if (hsi)
  {
    if (hsi->fin)
    {
      fclose(hsi->fin);
    }

    free(hsi);
  }
}


//---------------------------------------------------------------------------
// Create input stream
ERR                                     // Returns error code
inputstream_Create(
  HINPUTSTREAM *phsi,                   // Ptr to handle; must point to NULL
  LPCSTR filename,                      // File to open
  size_t buffersize)                    // Buffer size in bytes
{
  ERR result = ERR_OK;
  HINPUTSTREAM hs = NULL;

  if (!result)
  {
    if ((!phsi) || (!filename) || (!buffersize) || (*phsi))
    {
      result = ERR_PARAMETER;
    }
  }

  if (!result)
  {
    if (!(hs = (HINPUTSTREAM)calloc(1, sizeof(INPUTSTREAM) + buffersize)))
    {
      result = ERR_MALLOC;
    }
  }

  if (!result)
  {
    // Initialize file handle
    if (!(hs->fin = fopen(filename, "rb")))
    {
      result = ERR_INPUT_FILE_OPEN;
    }
  }

  if (!result)
  {
    // Initialize other members. Yes this is redundant after calloc but these
    // explicit initializations will show future programmers that we know
    // what we're doing.
    hs->startindex = 0;
    hs->endindex   = 0;
    hs->rateid     = RATEID_UNKNOWN;
    hs->framesize  = 0;

    hs->buffersize = buffersize;
  }

  if (result)
  {
    // If something went wrong, destroy the instance
    inputstream_Destroy(hs);
    hs = NULL;
  }

  // Feedback
  if (phsi)
  {
    *phsi = hs;
  }

  return result;
}


//---------------------------------------------------------------------------
// Read data into the input buffer if possible
ERR                                     // Returns error code
inputstream_ReadFile(
  HINPUTSTREAM hsi)                     // Input stream handle
{
  ERR result = ERR_OK;

  if (!result)
  {
    if (!hsi)
    {
      result = ERR_PARAMETER;
    }
  }

  if (!result)
  {
    // If there is any data in the buffer that hasn't been processed yet,
    // move it to the start of the buffer, overwriting any data that was
    // already processed before.
    if (hsi->startindex)
    {
      // The data doesn't start at the top of the buffer so it may need to be
      // moved.
      if (hsi->endindex != hsi->startindex)
      {
        // There is unprocessed data in the buffer. Move it to the top.
        memmove(hsi->buffer, hsi->buffer + hsi->startindex, hsi->endindex - hsi->startindex);
        hsi->endindex -= hsi->startindex;
      }
      else
      {
        // The unprocessed data has zero length.
        // So to move it to the start of the buffer, all we need to do is reset
        // the index.
        hsi->endindex = 0;
      }

      // The start of the unused data is now at the top of the buffer so reset
      // the index.
      hsi->startindex = 0;
    }
  }

  // At this point, the buffer starts with unprocessed data and ends with
  // space for new data.

  if (!result)
  {
    // Make sure we have buffer space available to read data, otherwise
    // we'll end up in an infinite loop.
    if (hsi->endindex == hsi->buffersize)
    {
      result = ERR_INTERNAL;
    }
  }

  if (!result)
  {
    size_t read_length;

    // Read data starting at the end of the unhandled bytes
    read_length = fread(hsi->buffer + hsi->endindex, 1, hsi->buffersize - hsi->endindex, hsi->fin);
    hsi->endindex += read_length;

    if (!read_length)
    {
      result = (ferror(hsi->fin) ? ERR_INPUT_FILE_READ : ERR_INPUT_FILE_EOF);
    }
  }

  // TODO: At this point, if we have just read the first two bytes of an
  // TODO:   .MPP file, we could check if the first byte is a valid rate
  // TODO:   ID, and compare it to the following frames. DCC-Studio will
  // TODO:   happily read tapes with mixed sample rates and store them in
  // TODO:   a single .MPP file but it stores the last encountered sample
  // TODO:   rate in the header of the file, which makes it almost always
  // TODO:   impossible to play the track from the hard disk or record it
  // TODO:   back to tape, since DCC-Studio will incorrectly calculate the
  // TODO:   offset of each frame based on the ID at the start of the file.
  // TODO:   We will eventually do the right thing (i.e. track the sample
  // TODO:   rate from the frames, not the header, and split files between
  // TODO:   sample rates), and this is a rare occurrence anyway, so it's
  // TODO:   not really that important.

  return result;
}


//---------------------------------------------------------------------------
// Copy a frame from the input stream to the output stream if possible
ERR                                     // Returns error code
inputstream_CopyFrame(
  HINPUTSTREAM hsi,                     // Input stream handle
  HOUTPUTSTREAM hso)                    // Output stream handle
{
  ERR result = ERR_OK;

  if (!result)
  {
    if (!hsi)
    {
      result = ERR_PARAMETER;
    }
  }

  if (!result)
  {
    // If we don't have a framesize yet, see if we can calculate it now.
    if (!hsi->framesize)
    {
      for(;;)
      {
        result = GetFrameSize(hsi->buffer + hsi->startindex, hsi->endindex - hsi->startindex, &hsi->framesize, &hsi->rateid);

        if (result == ERR_SYNC)
        {
          // There's extra data between frames, just get the next byte
          // if possible
          if (hsi->startindex < hsi->endindex)
          {
            hsi->startindex++;
            continue;
          }
          else
          {
            // Need more input
            result = ERR_INSUFFICIENT_DATA;
          }
        }

        break;
      }
    }
  }

  if (!result)
  {
    // Check if we finally have a frame size
    if (!hsi->framesize)
    {
      result = ERR_INSUFFICIENT_DATA;
    }
  }

  if (!result)
  {
    // Check if the entire frame is in the buffer
    if (hsi->endindex - hsi->startindex < hsi->framesize)
    {
      result = ERR_INSUFFICIENT_DATA;
    }
  }

  if (!result)
  {
    // Call the output callback function to process the frame.
    result = outputstream_ProcessFrame(hso, hsi->buffer + hsi->startindex, hsi->framesize, hsi->rateid);
  }

  if (!result)
  {
    // Remove the frame from the input buffer
    hsi->startindex += hsi->framesize;
    hsi->framesize = 0;
    hsi->rateid = RATEID_UNKNOWN;
  }

  return result;
}


//---------------------------------------------------------------------------
// Process input file
ERR                                     // Returns error code
ProcessFile(
  LPCSTR infilename,                    // Input file name
  LPCSTR outfilename,                   // Output file name
  BOOL output_is_mpp)                   // TRUE=Generate MPP file
{
  ERR result = ERR_OK;
  HINPUTSTREAM hsi = NULL;
  HOUTPUTSTREAM hso = NULL;

  if (!result)
  {
    if ((!infilename) || (!*infilename) || (!outfilename) || (!*outfilename))
    {
      result = ERR_PARAMETER;
    }
  }

  if (!result)
  {
    result = inputstream_Create(&hsi, infilename, INPUT_BUFFER_SIZE);
  }

  if (!result)
  {
    result = outputstream_Create(&hso, outfilename, output_is_mpp);
  }

  if (!result)
  {
    ERR inresult  = ERR_OK;
    ERR outresult = ERR_OK;

    fprintf(stderr, "%s -> %s\n", infilename, outfilename);

    while ((!inresult) && (!outresult))
    {
      BOOL eof = FALSE;

      inresult = inputstream_ReadFile(hsi);
      if (inresult == ERR_INPUT_FILE_EOF)
      {
        eof = TRUE;
        inresult = ERR_OK;
        break;
      }

      for (;;)
      {
        static UINT32 timestamp;

        outresult = inputstream_CopyFrame(hsi, hso);
        if (outresult == ERR_INSUFFICIENT_DATA)
        {
          outresult = ERR_OK;
          break;
        }

        if (GetTickCount() - timestamp > 1000)
        {
          timestamp = GetTickCount();
          fprintf(stderr, "%u frame%s\r", hso->numframes, (hso->numframes == 1 ? "" : "s"));
        }
      }

      if (eof)
      {
        break;
      }
    }

    fprintf(stderr, "%u frame%s DONE\n", hso->numframes, (hso->numframes == 1 ? "" : "s"));

    if (inresult)
    {
      result = inresult;
    }
    else if (outresult)
    {
      result = outresult;
    }
  }

  outputstream_Destroy(hso);
  inputstream_Destroy(hsi);

  return result;
}


//---------------------------------------------------------------------------
// Main program
int main(int argc, char *argv[])
{
  int result = 0;

  fprintf(stderr, 
    "DCCU File Conversion Utility for DCC-Studio\n"
    "Version 3.2\n"
    "(C) 2020 Jac Goudsmit\n"
    "Licensed under the MIT license.\n"
    "\n");

  if (argc < 2)
  {
    fprintf(stderr, 
      "This program converts MPP files (used by DCC-Studio) to MP1 (MPEG 1 Layer 1)\n"
      "and vice versa.\n"
      "\n"
      "Syntax: DCCU inputfile [inputfile...]\n"
      "\n"
      "You can convert multiple files at a time by putting multiple file names on\n"
      "the command line. The output file name(s) is/are generated from the input\n"
      "file names by changing the file extension from \".MPP\" to \".MP1\" or\n"
      "from \".MP1\" to \".MPP\".\n"
      "\n"
      "When converting to .MPP, the program also generates a .LVL and a .TRK file.\n"
      "Those are necessary to import the audio into the DCC-Studio. However,\n"
      "because DCCU is not an MP1 decoder, it has to put dummy information into\n"
      "the .LVL file. As a result is, you won't see the actual audio levels\n"
      "in the DCC Studio wave editor unless you record the track to tape first,\n"
      "and then copy it back to hard disk again.\n"
      "\n");

    result = ERR_COMMAND;
  }

  if (!result)
  {
    int i;

    for (i = 1; i < argc; i++)
    {
      ERR loopresult = ERR_OK;
      LPCSTR inputfilename = argv[i];
      char outputfilename[MAX_PATH] = "";
      BOOL output_is_mpp = FALSE;

      if (!loopresult)
      {
        if (!*inputfilename)
        {
          loopresult = ERR_INPUT_FILE_NAME;
        }
      }

      if (!loopresult)
      {
        if (ReplaceFileExtension(inputfilename, outputfilename, "MP1", "MPP"))
        {
          output_is_mpp = TRUE;
        }
        else if (ReplaceFileExtension(inputfilename, outputfilename, "MPP", "MP1"))
        {
          // Nothing to do
        }
        else
        {
          // Filename extension not recognized
          loopresult = ERR_INPUT_FILE_NAME;
        }
      }

      if (!loopresult)
      {
        // We have an input file name and an output file name and we know
        // if we are creating an MPP file. Let's go!
        loopresult = ProcessFile(inputfilename, outputfilename, output_is_mpp);
      }

      if (loopresult)
      {
        // TODO: print error message
        fprintf(stderr, "Error %u processing file %s\n", loopresult, inputfilename);

        // Set result to first error that appeared
        if (!result)
        {
          result = loopresult;
        }
      }
    } // for
  }

  return result;
}


/////////////////////////////////////////////////////////////////////////////
// END
/////////////////////////////////////////////////////////////////////////////
