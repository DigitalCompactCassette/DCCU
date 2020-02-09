# History #

When the DCC format was still alive, it was revealed that the PASC encoding system was not some sort of proprietary compression system but was in fact "similar to" MPEG 1 Layer 1. Also, the DCC-175 was released in the Netherlands and made it possible to store PASC encoded music on the hard disk of a Windows PC. Someone on the DCC-L discussion list made a program that converted the MPP files of DCC-Studio to MP1 by removing the extra bytes. The program was put online on a website including its source code, but the website eventually disappeared.

In 2003, I (Jac Goudsmt) had a chance to look at the ISO-11172.3 document which revealed that the padding bit in an MP1 stream is necessary for a stream that's recorded at 44.1 kHz because the PCM bit rate cannot be evenly divided into the MPEG bit rate. However I was confused about how this applied to the MPP files created by DCC-Studio.

In 2019 I finally realized that the extra bytes in the MPP files are there for the benefit of easy seeking: By making all the frames the same size, the DCC-Studio could simply seek to a particular frame in an MPP file by multiplying the frame number by the (now fixed) size. I had thought previously that the extra bytes are also on the tape but this isn't the case.

I also realized that the byte that's at the start of each MPP files not just an arbitrary byte that indicates that the file is an MPP file (and it's not there to prevent MPEG players from decoding the file). The byte indicates the sample rate of the frames in the MPP file, because the frame size is different for each sample frequency. And the entire MPP file must contain frames that are all the same size, so that seeking to a particular frame number in the file would be easy.

DCC supports the 44.1 kHz sample frequency to record audio CD's, of course, but it also supports 32 kHz and 48 kHz. I never had any 48 kHz audio sources to connect to a DCC recorder, but I extrapolated my recent epiphany about the format of MPP files and created a 48 kHz MP1 stream file from a sample file that I found online, and then I used some simple tools to put what I speculated would be the correct header for a 48 kHz MPP file in front of it. Importing into DCC-Studio was successful but when I opened the file, DCC-Studio crashed because there was no .LVL file. But I copied another .LVL file in the hope that it would work, and it did!

So now I knew enough to make a new tool that was similar to the old DCCConv (and DCCConv32) tools, but would also support the other sample frequencies (and frame sizes), and would be able to do the reverse, too: convert from MP1 to MPP. That had never been done before as far as I know (I have no idea why this functionality was not added to DCC-Studio by Philips). You are looking at the result.

The initial release of DCCU can't do very much: it only converts between MPP and MP1 and vice versa, but I made sure that the architecture of the tool is prepared to do some more things in the future to make things easy. For that, I will need to do some more work.

=== Jac Goudsmit