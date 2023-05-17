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

#include "FshIo.h"
#include "FileIo.h"
#include "QFS.h"
#include "resource.h"
#include <new>
#include <memory>

OSErr CountEntryMipMaps(HANDLE hFile, const FshHeader& header, const FshDirEntry& dir, const FshBmpEntry& entry, int* count, bool* packed)
{
    OSErr e = noErr;

    *count = 0;
    *packed = false;
    const bool compressed = (entry.code & 0x80) != 0;

    if (!compressed && (entry.misc[3] & 0x0FFF) == 0) // multiscale bitmaps
    {
        int numScales = (entry.misc[3] >> 12) & 0x0F;
        if ((entry.width % (1 << numScales)) != 0 || (entry.height % (1 << numScales)) != 0)
        {
            numScales = 0;
        }

        if (numScales > 0)
        {
            try
            {
                FshDirEntry* dirEntries = new FshDirEntry[header.numBmps];
                for (int i = 0; i < header.numBmps; i++)
                {
                    e = ReadFshDir(hFile, i, &dirEntries[i]);
                    if (e != noErr) break;
                }

                if (e == noErr)
                {
                    UINT32 nextOffset = static_cast<UINT32>(header.size);

                    for (int i = 0; i < header.numBmps; i++)
                    {
                        if (dirEntries[i].offset > dir.offset && static_cast<UINT32>(dirEntries[i].offset) < nextOffset)
                        {
                            nextOffset = dirEntries[i].offset;
                        }
                    }

                    const FshBmpType code = static_cast<FshBmpType>(entry.code & 0x7F);

                    UINT32 mbpLen = 0;
                    UINT32 mbpPadLen = 0;

                    for (int i = 0; i <= numScales; i++)
                    {
                        const int width = entry.width >> i;
                        const int height = entry.height >> i;

                        UINT32 dataLength = 0;

                        switch (code)
                        {
                        case DXT1:
                            // DXT1 images must be padded to a multiple of four.
                            dataLength = ((((width + 3) & ~3) * ((height + 3) & ~3)) / 2);
                            break;
                        case DXT3:
                            dataLength = (width * height);
                            break;
                        case ThirtyTwoBit:
                            dataLength = (width * height * 4);
                            break;
                        case TwentyFourBit:
                            dataLength = (width * height * 3);
                            break;
                        case SixteenBit:
                        case SixteenBitAlpha:
                        case SixteenBit4x4:
                            dataLength = (width * height * 2);
                            break;
                        }

                        mbpLen += dataLength;
                        mbpPadLen += dataLength;

                        // DXT1 mipmaps smaller than 4x4 are also padded
                        int padLength = ((16 - static_cast<INT64>(mbpLen)) & 15);
                        if (padLength > 0)
                        {
                            mbpLen += padLength;
                            if (i == numScales)
                            {
                                mbpPadLen += padLength;
                            }
                        }
                    }

                    const UINT32 entryLength = static_cast<UINT32>(entry.code >> 8);

                    if (entryLength != 0 && entryLength != (mbpLen + sizeof(FshBmpEntry)) ||
                        entryLength == 0 && (mbpLen + dir.offset + sizeof(FshBmpEntry)) != nextOffset)
                    {
                        *packed = true;
                        if (entryLength != 0 && entryLength != (mbpPadLen + sizeof(FshBmpEntry)) ||
                            entryLength == 0 && (mbpPadLen + dir.offset + sizeof(FshBmpEntry)) != nextOffset)
                        {
                            numScales = 0;
                        }
                    }

                    *count = numScales;
                }

                delete[] dirEntries;
            }
            catch(std::bad_alloc)
            {
                e = memFullErr;
            }
        }
    }

    return e;
}

OSErr CheckFshBmpTypes(HANDLE file, const FshHeader& header)
{
    OSErr e = noErr;

    if (header.numBmps >= 1)
    {
        for (int i = 0; i < header.numBmps; i++)
        {
            FshDirEntry dir;
            FshBmpEntry entry;

            e = ReadFshDir(file, i, &dir);

            if (e != noErr) break;

            e = ReadFshEntryDir(file, dir, &entry);

            if (e != noErr) break;

            const FshBmpType code = static_cast<FshBmpType>(entry.code & 0x7F);

            if (code != DXT1 &&
                code != DXT3 &&
                code != ThirtyTwoBit &&
                code != TwentyFourBit &&
                code != SixteenBit &&
                code != SixteenBitAlpha &&
                code != SixteenBit4x4)
            {
                e = formatCannotRead;
                break;
            }
        }
    }
    else
    {
        e = formatCannotRead;
    }

    return e;
}

OSErr GetFshBmpInfo(HANDLE file, const int index, FshBmpEntry* entry)
{
    OSErr e = noErr;

    FshDirEntry dir;
    e = ReadFshDir(file, index, &dir);

    if (e == noErr)
    {
        e = ReadFshEntryDir(file, dir, entry);
    }

    return e;
}

int GetImageDataSize(const int width, const int height, const FshBmpType format)
{
    switch (format)
    {
        case DXT1:
            return ((width * height) / 2);
        case DXT3:
            return (width * height);
        case ThirtyTwoBit:
            return (width * height * 4);
        case TwentyFourBit:
            return (width * height * 3);
        case SixteenBit:
        case SixteenBitAlpha:
        case SixteenBit4x4:
            return (width * height * 2);
    }

    return 0; // unsupported format
}

OSErr DecompressFsh(FormatRecordPtr pb, HANDLE file)
{
    bool compressed = false;

    OSErr e = IsQFSCompressed(file, 0, &compressed);

    if (e == noErr && compressed)
    {
        INT64 size;
        e = GetFileSize(file, &size);

        if (e == noErr)
        {
            if (size > INT_MAX)
            {
                // Sanity check as the Photoshop Buffer suite uses a signed 32-bit integer for the memory amount.
                e = memFullErr;
            }

            if (e == noErr)
            {
                e = SetFilePosition(file, FILE_BEGIN, 0);

                if (e == noErr)
                {
                    BufferID tempBuffer;

                    e = pb->bufferProcs->allocateProc(static_cast<int32>(size), &tempBuffer);
                    if (e == noErr)
                    {
                        BYTE* fshBytes = reinterpret_cast<BYTE*>(pb->bufferProcs->lockProc(tempBuffer, FALSE));
                        e = ReadBytes(file, fshBytes, static_cast<DWORD>(size));

                        if (e == noErr)
                        {
                            int uncompressedSize;

                            e = GetUncompressedSize(fshBytes, static_cast<DWORD>(size), &uncompressedSize);

                            if (e == noErr)
                            {
                                e = pb->bufferProcs->allocateProc(static_cast<int32>(uncompressedSize), &qfsBufferID);

                                if (e == noErr)
                                {
                                    qfsBuffer = reinterpret_cast<BYTE*>(pb->bufferProcs->lockProc(qfsBufferID, FALSE));

                                    e = QFSDecompress(fshBytes, static_cast<DWORD>(size), qfsBuffer, uncompressedSize);
                                }
                            }
                        }

                        pb->bufferProcs->unlockProc(tempBuffer);
                        pb->bufferProcs->freeProc(tempBuffer);
                    }
                }
            }
        }
    }

    return e;
}

static bool CheckIdentifier (const char (&identifier)[4])
{
    return identifier[0] == 'S' &&
           identifier[1] == 'H' &&
           identifier[2] == 'P' &&
           identifier[3] == 'I';
}

OSErr ReadFshHeader(HANDLE file, FshHeader* head)
{
    ZeroMemory(head, sizeof(FshHeader));
    OSErr e = noErr;

    if (qfsBuffer != nullptr)
    {
        *head = *reinterpret_cast<FshHeader*>(qfsBuffer);
    }
    else
    {
        e = SetFilePosition(file, FILE_BEGIN, 0);

        if (e == noErr)
        {
            e = ReadBytes(file, head->SHPI, 4);

            if (e == noErr)
            {
                e = ReadInt32(file, &head->size);

                if (e == noErr)
                {
                    e = ReadInt32(file, &head->numBmps);

                    if (e == noErr)
                    {
                        e = ReadBytes(file, head->dirID, 4);
                    }
                }
            }
        }
    }

    if (e == noErr && !CheckIdentifier(head->SHPI))
    {
        e = formatCannotRead;
    }

    return e;
}

OSErr ReadFshDir(HANDLE file, const int index, FshDirEntry* dir)
{
    OSErr e = noErr;

    ZeroMemory(dir, sizeof(FshDirEntry));
    int offset = sizeof(FshHeader) + index * sizeof(FshDirEntry);

    if (qfsBuffer != nullptr)
    {
        *dir = *reinterpret_cast<FshDirEntry*>(qfsBuffer + offset);
    }
    else
    {
        e = SetFilePosition(file, FILE_BEGIN, offset);

        if (e == noErr)
        {
            e = ReadBytes(file, dir->name, 4);

            if (e == noErr)
            {
                e = ReadInt32(file, &dir->offset);
            }
        }
    }

    return e;
}

OSErr ReadFshEntryDir(HANDLE file, const FshDirEntry& dir, FshBmpEntry* entry)
{
    ZeroMemory(entry, sizeof(FshBmpEntry));
    OSErr e = noErr;

    if (qfsBuffer != nullptr)
    {
        *entry = *reinterpret_cast<FshBmpEntry*>(qfsBuffer + dir.offset);
    }
    else
    {
        e = SetFilePosition(file, FILE_BEGIN, dir.offset);

        if (e == noErr)
        {
            e = ReadInt32(file, &entry->code);

            if (e == noErr)
            {
                e = ReadUInt16(file, &entry->width);

                if (e == noErr)
                {
                    e = ReadUInt16(file, &entry->height);

                    if (e == noErr)
                    {
                        for (int i = 0; i < 4; i++)
                        {
                            e = ReadUInt16(file, &entry->misc[i]);

                            if (e != noErr) break;
                        }
                    }
                }
            }
        }
    }

    return e;
}

static OSErr DecompressEntry(FormatRecordPtr pb,
                             const FshHeader& header,
                             const FshDirEntry& dir,
                             const FshBmpEntry& entry,
                             void* outData,
                             const DWORD outLength)
{
    OSErr e = noErr;
    const int imageStartOffset = dir.offset + sizeof(FshBmpEntry);

    HANDLE hFile = reinterpret_cast<HANDLE>(pb->dataFork);

    int size = 0;

    int entrySize = entry.code >> 8;
    if (entrySize > 0)
    {
        size = entrySize;
    }
    else
    {
        if (header.numBmps == 1)
        {
            size = header.size - imageStartOffset;
        }
        else
        {
            try
            {
                // Calculate the next offset to get the size.
                FshDirEntry* dirEntries = new FshDirEntry[header.numBmps];
                for (int j = 0; j < header.numBmps; j++)
                {
                    e = ReadFshDir(hFile, j, &dirEntries[j]);
                    if (e != noErr)	break;
                }

                if (e == noErr)
                {
                    int nextOffset = header.size;

                    for (int j = 0; j < header.numBmps; j++)
                    {
                        if ((dirEntries[j].offset > dir.offset) && (dirEntries[j].offset < nextOffset))
                        {
                            nextOffset = dirEntries[j].offset;
                        }
                    }

                    size = nextOffset - imageStartOffset;
                }

                delete[] dirEntries;
            }
            catch (std::bad_alloc)
            {
                e = memFullErr;
            }
        }
    }

    if (e == noErr)
    {
        if (qfsBuffer != nullptr)
        {
            e = QFSDecompress(qfsBuffer + imageStartOffset, static_cast<DWORD>(size), reinterpret_cast<BYTE*>(outData), outLength);
        }
        else
        {
            BufferID temp;
            e = pb->bufferProcs->allocateProc(static_cast<int32>(size), &temp);

            if (e == noErr)
            {
                Ptr compressedData = pb->bufferProcs->lockProc(temp, FALSE);

                e = SetFilePosition(hFile, FILE_BEGIN, imageStartOffset);

                if (e == noErr)
                {
                    e = ReadBytes(hFile, compressedData, size);

                    if (e == noErr)
                    {
                        e = QFSDecompress(reinterpret_cast<BYTE*>(compressedData), size, reinterpret_cast<BYTE*>(outData), outLength);
                    }
                }

                pb->bufferProcs->unlockProc(temp);
                pb->bufferProcs->freeProc(temp);
            }
        }
    }

    return e;
}

OSErr ReadFshImageData(FormatRecordPtr pb,
                       const FshHeader& header,
                       const FshDirEntry& dir,
                       const FshBmpEntry& entry,
                       void* outData,
                       const DWORD outLength)
{
    OSErr e = noErr;

    if ((entry.code & 0x80) != 0)
    {
        e = DecompressEntry(pb, header, dir, entry, outData, outLength);
    }
    else
    {
        if (qfsBuffer != nullptr)
        {
            const void* src = qfsBuffer + (dir.offset + sizeof(FshBmpEntry));
            memcpy(outData, src, outLength);
        }
        else
        {
            HANDLE file = reinterpret_cast<HANDLE>(pb->dataFork);
            e = SetFilePosition(file, FILE_BEGIN, dir.offset + sizeof(FshBmpEntry));

            if (e == noErr)
            {
                e = ReadBytes(file, outData, outLength);
            }
        }
    }

    return e;
}

OSErr WriteFshHeader(HANDLE file, const FshHeader& header)
{
    OSErr e = noErr;

    e = SetFilePosition(file, FILE_BEGIN, 0);

    if (e == noErr)
    {
        e = WriteBytes(file, header.SHPI, 4);
        if (e == noErr)
        {
            e = WriteInt32(file, header.size);
            if (e == noErr)
            {
                e = WriteInt32(file, header.numBmps);
                if (e == noErr)
                {
                    e = WriteBytes(file, header.dirID, 4);
                }
            }
        }
    }

    return e;
}

OSErr WriteFshDir(HANDLE file, const FshDirEntry& dir)
{
    OSErr e = noErr;

    e = WriteBytes(file, dir.name, 4);
    if (e == noErr)
    {
        e = WriteInt32(file, dir.offset);
    }

    return e;
}

OSErr WriteFshEntryDir(HANDLE file, const FshDirEntry& dir, const FshBmpEntry& entry)
{
    OSErr e = noErr;

    e = SetFilePosition(file, FILE_BEGIN, dir.offset);
    if (e == noErr)
    {
        e = WriteInt32(file, entry.code);
        if (e == noErr)
        {
            e = WriteUInt16(file, entry.width);
            if (e == noErr)
            {
                e = WriteUInt16(file, entry.height);
                if (e == noErr)
                {
                    for (int i = 0; i < 4; i++)
                    {
                        e = WriteUInt16(file, entry.misc[i]);

                        if (e != noErr) break;
                    }
                }
            }
        }
    }

    return e;
}

OSErr WriteFshImageData(HANDLE file, const void* inData, const int length)
{
    return WriteBytes(file, inData, length);
}

OSErr IsValidFshFile(FormatRecordPtr pb)
{
    OSErr e = noErr;
    HANDLE hFile = reinterpret_cast<HANDLE>(pb->dataFork);

    e = DecompressFsh(pb, hFile);
    if (e == noErr)
    {
        // ReadFshHeader will return formatCannotRead if the header signature is not valid.

        FshHeader header;
        e = ReadFshHeader(hFile, &header);

        if (e == noErr && header.numBmps < 1)
        {
            e = formatCannotRead;
        }

        if (qfsBuffer != nullptr)
        {
            pb->bufferProcs->unlockProc(qfsBufferID);
            pb->bufferProcs->freeProc(qfsBufferID);
        }
    }

    return e;
}