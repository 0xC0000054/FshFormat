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

#include "FshFormatPS.h"
#include "FileIo.h"
#include "QFS.h"
#include "ui.h"
#include "squish.h"

BufferID outBufferID;
void* outData = nullptr;

static OSErr GetDataSize(HANDLE hFile, const FshDirEntry& dir, const FshBmpEntry& entry, int* dataSize)
{
    OSErr e = noErr;

    if ((entry.code & 0x80) != 0)
    {
        const LONG offset = static_cast<LONG>(dir.offset + sizeof(FshBmpEntry));

        bool compressed = false;

        e = IsQFSCompressed(hFile, offset, &compressed);

        if (e == noErr)
        {
            if (compressed)
            {
                e = GetUncompressedSize(hFile, offset, dataSize);
            }
            else
            {
                e = formatCannotRead; // Unknown compression format
            }
        }
    }
    else
    {
        *dataSize = GetImageDataSize(entry.width, entry.height, static_cast<FshBmpType>(entry.code & 0x7F));

        if (*dataSize == 0)
        {
            e = formatCannotRead; // Unsupported image format
        }
    }

    return e;
}

static OSErr ReadFsh(FormatRecordPtr pb, const FshHeader& header, const FshDirEntry& dir, const FshBmpEntry& entry)
{
    int dataSize;

    OSErr e = GetDataSize(reinterpret_cast<HANDLE>(pb->dataFork), dir, entry, &dataSize);

    if (e == noErr)
    {
        BufferID temp = nullptr;
        void* tempData = nullptr;

        const FshBmpType code = static_cast<FshBmpType>(entry.code & 0x7f);

        if (code == DXT1 || code == DXT3)
        {
            e = pb->bufferProcs->allocateProc(dataSize, &temp);

            if (e == noErr)
            {
                tempData = pb->bufferProcs->lockProc(temp, FALSE);
                e = ReadFshImageData(pb, header, dir, entry, tempData, dataSize);

                if (e == noErr)
                {
                    e = pb->bufferProcs->allocateProc((entry.width * entry.height * 4), &outBufferID);

                    if (e == noErr)
                    {
                        pb->data = outData = reinterpret_cast<BYTE*>(pb->bufferProcs->lockProc(outBufferID, FALSE));

                        int flags = 0;
                        switch (code)
                        {
                        case DXT1:
                                flags |= squish::kDxt1;
                            break;
                        case DXT3:
                                flags |= squish::kDxt3;
                            break;
                        }

                        squish::DecompressImage(
                            reinterpret_cast<squish::u8*>(outData),
                            static_cast<int>(entry.width),
                            static_cast<int>(entry.height),
                            tempData,
                            flags);
                    }
                }
            }
        }
        else if (code == TwentyFourBit || code == ThirtyTwoBit)
        {
            // Set the plane map to BGRA
            pb->planeMap[0] = 2; // B
            pb->planeMap[1] = 1; // G
            pb->planeMap[2] = 0; // R
            pb->planeMap[3] = 3; // A

            e = pb->bufferProcs->allocateProc(dataSize, &outBufferID);

            if (e == noErr)
            {
                pb->data = outData = reinterpret_cast<BYTE*>(pb->bufferProcs->lockProc(outBufferID, FALSE));
                e = ReadFshImageData(pb, header, dir, entry, outData, dataSize);
            }
        }
        else // the packed 16-bit formats
        {
            e = pb->bufferProcs->allocateProc(dataSize, &temp);

            if (e == noErr)
            {
                tempData = pb->bufferProcs->lockProc(temp, FALSE);

                e = ReadFshImageData(pb, header, dir, entry, tempData, dataSize);

                if (e == noErr)
                {
                    e = pb->bufferProcs->allocateProc((pb->rowBytes * entry.height), &outBufferID);

                    if (e == noErr)
                    {
                        pb->data = outData = pb->bufferProcs->lockProc(outBufferID, FALSE);
                        BYTE* destPtr = reinterpret_cast<BYTE*>(outData);

                        if (code == SixteenBit) // 16-bit RGB (0:5:6:5)
                        {
                            const UINT16* sPtr = static_cast<UINT16*>(tempData);

                            for (int y = 0; y < entry.height; y++)
                            {
                                const UINT16* src = sPtr + (y * entry.width);
                                BYTE* p = destPtr + (y * pb->rowBytes);

                                for (int x = 0; x < entry.width; x++)
                                {
                                    p[0] = (((src[0] >> 11) & 0x1f) << 3);
                                    p[1] = (((src[0] >> 5) & 0x3f) << 2);
                                    p[2] = ((src[0] & 0x1f) << 3);

                                    src++;
                                    p += 3;
                                }

                            }
                        }
                        else if (code == SixteenBitAlpha) // 16-bit ARGB (1:5:5:5)
                        {
                            const UINT16* sPtr = static_cast<UINT16*>(tempData);

                            for (int y = 0; y < entry.height; y++)
                            {
                                const UINT16* src = sPtr + (y * entry.width);
                                BYTE* p = destPtr + (y * pb->rowBytes);

                                for (int x = 0; x < entry.width; x++)
                                {
                                    p[0] = (((src[0] >> 10) & 0x1f) << 3);
                                    p[1] = (((src[0] >> 5) & 0x1f) << 3);
                                    p[2] = ((src[0] & 0x1f) << 3);
                                    p[3] = ((src[0] & 0x8000) != 0) ? 255 : 0;

                                    src++;
                                    p += 4;
                                }
                            }
                        }
                        else if (code == SixteenBit4x4) // 16-bit ARGB (4:4:4:4)
                        {
                            const BYTE* ptr = static_cast<BYTE*>(tempData);
                            const int srcStride = entry.width * 2;

                            for (int y = 0; y < entry.height; y++)
                            {
                                const BYTE* src = ptr + (y * srcStride);
                                BYTE* p = destPtr + (y * pb->rowBytes);

                                for (int x = 0; x < entry.width; x++)
                                {
                                    p[0] = ((src[1] & 15) * 0x11);
                                    p[1] = ((src[0] >> 4) * 0x11);
                                    p[2] = ((src[0] & 15) * 0x11);
                                    p[3] = ((src[1] >> 4) * 0x11);

                                    src += 2;
                                    p += 4;
                                }
                            }
                        }
                    }
                }
            }
        }

        if (tempData != nullptr)
        {
            pb->bufferProcs->unlockProc(temp);
            pb->bufferProcs->freeProc(temp);
        }
    }

    return e;
}

OSErr DoReadPrepare(FormatRecordPtr pb)
{
    // All buffers are allocated using the Photoshop API suites.
    pb->maxData = 0;

    return noErr;
}

OSErr DoReadStart(FormatRecordPtr pb)
{
    OSErr e = noErr;

    HANDLE hFile = reinterpret_cast<HANDLE>(pb->dataFork);
    qfsBuffer = nullptr;
    outData = nullptr;

    e = DecompressFsh(pb, hFile);

    if (e == noErr)
    {
        FshHeader header;
        e = ReadFshHeader(hFile, &header);

        if (e == noErr)
        {
            e = CheckFshBmpTypes(hFile, header);

            if (e == noErr)
            {
                int fshIndex = 0;

                if (pb->revertInfo != nullptr || header.numBmps == 1 || LoadFshDlg(pb, header, &fshIndex))
                {
                    RevertInfo* rev = nullptr;

                    if (pb->revertInfo != nullptr)
                    {
                        if (pb->handleProcs->getSizeProc(pb->revertInfo) == sizeof(RevertInfo))
                        {
                            rev = reinterpret_cast<RevertInfo*>(pb->handleProcs->lockProc(pb->revertInfo, FALSE));
                            fshIndex = rev->loadIndex;
                        }
                    }

                    pb->imageMode = plugInModeRGBColor;
                    pb->depth = 8;

                    FshDirEntry dir;
                    e = ReadFshDir(hFile, fshIndex, &dir);

                    if (e == noErr)
                    {
                        FshBmpEntry entry;
                        e = ReadFshEntryDir(hFile, dir, &entry);

                        if (e == noErr)
                        {
                            int mipCount;
                            bool mipPacked;

                            e = CountEntryMipMaps(hFile, header, dir, entry, &mipCount, &mipPacked);
                            if (e == noErr)
                            {
                                const FshBmpType code = static_cast<FshBmpType>(entry.code & 0x7f);
                                const bool hasAlpha = (code == DXT1 || code == DXT3 || code == ThirtyTwoBit || code == SixteenBitAlpha || code == SixteenBit4x4);

                                pb->imageSize.h = entry.width;
                                pb->imageSize.v = entry.height;

                                pb->planes = 3;

                                if (hasAlpha)
                                {
                                    pb->planes += 1;

                                    if (pb->transparencyPlane != 0)
                                    {
                                        pb->transparencyPlane = 3;
                                        pb->transparencyMatting = 0;
                                    }
                                }

                                pb->loPlane = 0;
                                pb->hiPlane = pb->planes - 1;
                                pb->colBytes = pb->planes;
                                pb->rowBytes = pb->imageSize.h * pb->colBytes;
                                pb->planeBytes = 1;

                                SETRECT(pb->theRect, 0, 0, entry.width, entry.height);

                                e = ReadFsh(pb, header, dir, entry);

                                if (e == noErr && pb->revertInfo == nullptr)
                                {
                                    pb->revertInfo = pb->handleProcs->newProc(sizeof(RevertInfo));
                                    if (pb->revertInfo != nullptr)
                                    {
                                        rev = reinterpret_cast<RevertInfo*>(pb->handleProcs->lockProc(pb->revertInfo, FALSE));
                                        rev->fshCode = code;
                                        memcpy(rev->headerDir, header.dirID, 4);
                                        memcpy(rev->entryDir, dir.name, 4);
                                        rev->mipCount = mipCount;
                                        rev->mipPacked = mipPacked;
                                        rev->loadIndex = fshIndex;
                                    }
                                }
                            }
                        }

                        if (pb->revertInfo != nullptr)
                        {
                            pb->handleProcs->unlockProc(pb->revertInfo);
                        }
                    }
                }
                else
                {
                    e = userCanceledErr;
                }
            }
        }

        if (e != noErr)
        {
            // If we encounter an error call DoReadFinish to free any allocated buffers.
            DoReadFinish(pb);
        }
    }

    return e;
}

OSErr DoReadContinue(FormatRecordPtr pb)
{
    pb->data = nullptr;

    return noErr;
}

OSErr DoReadFinish(FormatRecordPtr pb)
{
    if (outData != nullptr)
    {
        pb->bufferProcs->unlockProc(outBufferID);
        pb->bufferProcs->freeProc(outBufferID);
        outData = nullptr;
    }

    if (qfsBuffer != nullptr)
    {
        pb->bufferProcs->unlockProc(qfsBufferID);
        pb->bufferProcs->freeProc(qfsBufferID);
        qfsBuffer = nullptr;
    }

    return noErr;
}