/*
*  This file is part of FshFormat, a file format plug-in for Adobe Photoshop(R)
*  that loads and saves FSH images.
*
*  Copyright (C) 2011, 2012, 2013, 2014, 2015, 2022, 2023 Nicholas Hayes
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/

#include "Common.h"
#include "FileIo.h"
#include "QFS.h"
#include "QFSHeader.h"

BufferID qfsBufferID;
BYTE* qfsBuffer = nullptr;

OSErr QFSDecompress(const BYTE* inData, const DWORD inLength, BYTE* outData, const DWORD outLength)
{
    if (inData == nullptr || outData == nullptr)
    {
        return paramErr;
    }

    try
    {
        QFSHeader header(inData, inLength);

        const DWORD uncompressedSize = static_cast<DWORD>(header.GetUncompressedSize());

        if (outLength < uncompressedSize)
        {
            return paramErr;
        }

        DWORD index = static_cast<DWORD>(header.GetDataStartOffset());

        BYTE ccbyte1 = 0; // control char 1
        BYTE ccbyte2 = 0; // control char 2
        BYTE ccbyte3 = 0; // control char 3
        BYTE ccbyte4 = 0; // control char 4
        int plainCount = 0;
        int copyCount = 0;
        int copyOffset = 0;

        DWORD outIndex = 0;

        while (index < inLength && inData[index] < 0xFC)
        {
            ccbyte1 = inData[index++];

            if (ccbyte1 >= 0xE0) // 1 byte op code 0xE0 - 0xFB
            {
                plainCount = ((ccbyte1 & 0x1F) << 2) + 4;
                copyCount = 0;
                copyOffset = 0;
            }
            else if (ccbyte1 >= 0xC0) // 4 byte op code 0xC0 - 0xDF
            {
                ccbyte2 = inData[index++];
                ccbyte3 = inData[index++];
                ccbyte4 = inData[index++];

                plainCount = (ccbyte1 & 3);
                copyCount = ((ccbyte1 & 0x0C) << 6) + ccbyte4 + 5;
                copyOffset = (((ccbyte1 & 0x10) << 12) + (ccbyte2 << 8)) + ccbyte3 + 1;
            }
            else if (ccbyte1 >= 0x80) // 3 byte op code 0x80 - 0xBF
            {
                ccbyte2 = inData[index++];
                ccbyte3 = inData[index++];

                plainCount = (ccbyte2 & 0xC0) >> 6;
                copyCount = (ccbyte1 & 0x3F) + 4;
                copyOffset = ((ccbyte2 & 0x3F) << 8) + ccbyte3 + 1;
            }
            else // 2 byte op code 0x00 - 0x7F
            {
                ccbyte2 = inData[index++];

                plainCount = (ccbyte1 & 3);
                copyCount = ((ccbyte1 & 0x1C) >> 2) + 3;
                copyOffset = ((ccbyte1 >> 5) << 8) + ccbyte2 + 1;
            }

            for (int i = 0; i < plainCount; i++)
            {
                outData[outIndex] = inData[index];
                index++;
                outIndex++;
            }

            if (copyCount > 0)
            {
                int srcIndex = outIndex - copyOffset;

                for (int i = 0; i < copyCount; i++)
                {
                    outData[outIndex] = outData[srcIndex];
                    srcIndex++;
                    outIndex++;
                }
            }
        }

        // Write the trailing bytes.
        if (index < inLength && outIndex < uncompressedSize)
        {
            // 1 byte EOF op code 0xFC - 0xFF.
            plainCount = (inData[index++] & 3);

            for (int i = 0; i < plainCount; i++)
            {
                outData[outIndex] = inData[index];
                index++;
                outIndex++;
            }
        }
    }
    catch (const OSErr e)
    {
        return e;
    }

    return noErr;
}

OSErr GetUncompressedSize(const BYTE* inData, const DWORD inLength, int* uncompressedSize)
{
    *uncompressedSize = 0;

    if (inData == nullptr)
    {
        return paramErr;
    }

    OSErr err = noErr;

    try
    {
        QFSHeader header(inData, inLength);

        *uncompressedSize = header.GetUncompressedSize();
    }
    catch (const OSErr headerError)
    {
        err = headerError;
    }

    return err;
}

OSErr GetUncompressedSize(const HANDLE hFile, const LONG offset, int* uncompressedSize)
{
    OSErr err = noErr;

    try
    {
        QFSHeader header(hFile, offset);

        *uncompressedSize = header.GetUncompressedSize();
    }
    catch (const OSErr headerError)
    {
        err = headerError;
    }

    return err;
}

OSErr IsQFSCompressed(HANDLE hFile, LONG offset, bool* isCompressed)
{
    *isCompressed = false;

    OSErr e = SetFilePosition(hFile, FILE_BEGIN, offset);

    if (e == noErr)
    {
        BYTE signature[2];
        e = ReadBytes(hFile, signature, 2);

        if (e == noErr)
        {
            if (QFSHeader::CheckSignature(signature))
            {
                *isCompressed = true;
            }
            else
            {
                e = SetFilePosition(hFile, FILE_BEGIN, 4);

                if (e == noErr)
                {
                    e = ReadBytes(hFile, signature, 2);

                    if (e == noErr && QFSHeader::CheckSignature(signature))
                    {
                        *isCompressed = true;
                    }
                }
            }
        }
    }

    return e;
}