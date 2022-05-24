#include "FshFormatPS.h"
#include "DxtComp.h"
#include <PIChannelPortsSuite.h>
#include <new>
#include "resource.h"
#include "squish.h"

BufferID inDataBuf;

void* inDataPtr;

static OSErr WriteImageDataImpl(FormatRecordPtr pb,
                                const void* data,
                                const int width,
                                const int height,
                                const int rowBytes,
                                const int colBytes,
                                const int outPlanes,
                                const Globals* globals)
{
    OSErr e = noErr;

    const FshBmpType fshType = globals->fshCode;
    int dxtWidth = width;
    int dxtHeight = height;

    BufferID outDataBuf = nullptr;
    void* outBuf = nullptr;

    int dataLength = 0;

    if (fshType != SixteenBit && fshType != SixteenBitAlpha && fshType != SixteenBit4x4)
    {
        int outRowBytes;
        int outColBytes;

        if (fshType == DXT1 || fshType == DXT3)
        {
            // DXTn images must be padded to a multiple of four.
            dxtWidth = (width + 3) & ~3;
            dxtHeight = (height + 3) & ~3;

            // DXTn images require an alpha channel, so we set an opaque one if we do not have any transparency.
            if (outPlanes == 4)
            {
                outRowBytes = width * outPlanes;
                outColBytes = outPlanes;
                dataLength = outRowBytes * height;
            }
            else
            {
                outRowBytes = dxtWidth * 4;
                outColBytes = 4;
                dataLength = outRowBytes * dxtHeight;
            }
        }
        else
        {
            outRowBytes = width * outPlanes;
            outColBytes = outPlanes;
            dataLength = outRowBytes * height;
        }

        if (fshType == TwentyFourBit || fshType == ThirtyTwoBit)
        {
            if (globals->mipCount > 0 && !globals->mipPacked)
            {
                // Pad the length to a multiple of 16 bytes.
                dataLength = (dataLength + 15) & ~15;
            }
        }

        e = pb->bufferProcs->allocateProc(dataLength, &outDataBuf);
        if (e == noErr)
        {
            outBuf = pb->bufferProcs->lockProc(outDataBuf, FALSE);
            ZeroMemory(outBuf, dataLength);
            const BYTE* dataPtr = reinterpret_cast<const BYTE*>(data);
            BYTE* outPtr = reinterpret_cast<BYTE*>(outBuf);

            for (int y = 0; y < height; y++)
            {
                const BYTE* in = dataPtr + (y * rowBytes);
                BYTE* out = outPtr + (y * outRowBytes);
                for (int x = 0; x < width; x++)
                {
                    if (outPlanes == 4)
                    {
                        if (in[3] == 0)
                        {
                            // Set the color of any transparent pixels to black.
                            out[0] = 0;
                            out[1] = 0;
                            out[2] = 0;
                        }
                        else
                        {
                            out[0] = in[0];
                            out[1] = in[1];
                            out[2] = in[2];
                        }

                        out[3] = in[3];
                    }
                    else
                    {
                        out[0] = in[0];
                        out[1] = in[1];
                        out[2] = in[2];

                        if (outColBytes == 4)
                        {
                            out[3] = 255;
                        }
                    }

                    in += colBytes;
                    out += outColBytes;
                }
            }
        }
    }

    if (e == noErr)
    {
        if (fshType == DXT1 || fshType == DXT3)
        {
            BufferID temp;

            const int blockCount = (dxtWidth / 4) * (dxtHeight / 4);
            const int blockSize = fshType == DXT1 ? 8 : 16;

            const int compressedSize = blockCount * blockSize;

            e = pb->bufferProcs->allocateProc(compressedSize + 2048, &temp);

            if (e == noErr)
            {
                BYTE* compressedData = reinterpret_cast<BYTE*>(pb->bufferProcs->lockProc(temp, FALSE));

                if (globals->fshWriteCompression)
                {
                    int flags = 0;

                    switch (fshType)
                    {
                    case DXT1:
                            flags |= squish::kDxt1;
                        break;
                    case DXT3:
                            flags |= squish::kDxt3;
                        break;
                    }

                    flags |= squish::kColourIterativeClusterFit;
                    flags |= squish::kColourMetricUniform;

                    squish::CompressImage(reinterpret_cast<squish::u8*>(outBuf), dxtWidth, dxtWidth, compressedData, flags);
                }
                else
                {
                    switch (fshType)
                    {
                    case DXT1:
                            CompressFSHToolDXT1(reinterpret_cast<BYTE*>(outBuf), compressedData, dxtWidth, dxtWidth);
                        break;
                    case DXT3:
                            CompressFSHToolDXT3(reinterpret_cast<BYTE*>(outBuf), compressedData, dxtWidth, dxtWidth);
                        break;
                    }
                }

                if (globals->mipCount > 0)
                {
                    if (!globals->mipPacked && fshType == DXT3 || fshType == DXT1 && (width == 1 || height == 1))
                    {
                        // Pad the data to a multiple of 16 bytes.
                        int padLength = ((16 - compressedSize) & 15);

                        if (padLength > 0)
                        {
                            memset(compressedData + compressedSize, 0, padLength);
                        }
                    }
                }
                e = WriteFshImageData(reinterpret_cast<HANDLE>(pb->dataFork), compressedData, compressedSize);
                pb->bufferProcs->unlockProc(temp);
                pb->bufferProcs->freeProc(temp);
            }
        }
        else if (fshType == TwentyFourBit || fshType == ThirtyTwoBit)
        {
            e = WriteFshImageData(reinterpret_cast<HANDLE>(pb->dataFork), outBuf, dataLength);
        }
        else
        {
            dataLength = (width * height) * 2;

            e = pb->bufferProcs->allocateProc(dataLength + 2048, &outDataBuf);
            if (e == noErr)
            {
                outBuf = pb->bufferProcs->lockProc(outDataBuf, FALSE);

                const BYTE* ptr = reinterpret_cast<const BYTE*>(data);
                UINT16* sPtr = reinterpret_cast<UINT16*>(outBuf);

                if (fshType == SixteenBit)// 16-bit RGB (0:5:6:5)
                {
                    for (int y = 0; y < height; y++)
                    {
                        const BYTE* src = ptr + (y * rowBytes);
                        UINT16* dst = sPtr + (y * width);
                        for (int x = 0; x < width; x++)
                        {
                            *dst = (((src[0] >> 3) << 11) + ((src[1] >> 2) << 5) + (src[2] >> 3));

                            src += colBytes;
                            dst++;
                        }
                    }
                }
                else if (fshType == SixteenBitAlpha) // 16-bit ARGB (1:5:5:5)
                {
                    for (int y = 0; y < height; y++)
                    {
                        const BYTE* src = ptr + (y * rowBytes);
                        UINT16* dst = sPtr + (y * width);
                        for (int x = 0; x < width; x++)
                        {
                            if (src[3] >= 128)
                            {
                                *dst = ((((src[0] >> 3) << 10) + ((src[1] >> 3) << 5) + (src[2] >> 3)) | 0x8000);
                            }
                            else
                            {
                                *dst = 0;
                            }

                            src += colBytes;
                            dst++;
                        }
                    }
                }
                else if (fshType == SixteenBit4x4)// 16-bit ARGB (4:4:4:4)
                {
                    for (int y = 0; y < height; y++)
                    {
                        const BYTE* src = ptr + (y * rowBytes);
                        UINT16* dst = sPtr + (y * width);
                        for (int x = 0; x < width; x++)
                        {
                            if (src[3] > 0)
                            {
                                *dst = (((src[0] >> 4) << 8) + ((src[1] >> 4) << 4) + (src[2] >> 4) + ((src[3] >> 4) << 12));
                            }
                            else
                            {
                                *dst = 0;
                            }

                            src += colBytes;
                            dst++;
                        }
                    }
                }

                if (globals->mipCount > 0 && !globals->mipPacked)
                {
                    // Pad the data to a multiple of 16 bytes.
                    int padLength = dataLength & 15;

                    if (padLength > 0)
                    {
                        BYTE* dest = reinterpret_cast<BYTE*>(outBuf) + dataLength;
                        memset(dest, 0, padLength);
                    }
                }

                e = WriteFshImageData(reinterpret_cast<HANDLE>(pb->dataFork), outBuf, dataLength);
            }
        }
    }

    if (outBuf != nullptr)
    {
        pb->bufferProcs->unlockProc(outDataBuf);
        pb->bufferProcs->freeProc(outDataBuf);
    }

    return e;
}

static OSErr CreateImageChannelPorts(FormatRecordPtr pb, const PSChannelPortsSuite1* suite, ReadChannelDesc** src, const int nPlanes)
{
    if (suite == nullptr)
    {
        return errPlugInHostInsufficient;
    }

    // Scale the image by creating new ports for the image data, for compatibility with Photoshop 6 and earlier.
    PixelMemoryDesc desc;
    desc.data = inDataPtr;
    desc.rowBits = pb->rowBytes * 8;
    desc.colBits = pb->colBytes * 8;
    desc.depth = 8;

    VRect portRect = {0, 0, pb->imageSize.v, pb->imageSize.h};

    OSErr e = noErr;
    SPErr spErr = kSPNoError;
    try
    {
        for (int i = 0; i < nPlanes; i++)
        {
            src[i] = new ReadChannelDesc();
            spErr = suite->New(&src[i]->port, &portRect, 8, FALSE);
            if (spErr != kSPNoError)
            {
                throw spErr;
            }

            desc.bitOffset = i * 8;
            spErr = suite->WritePixelsToBaseLevel(src[i]->port, &portRect, &desc);
            if (spErr != kSPNoError)
            {
                throw spErr;
            }
        }
    }
    catch (std::bad_alloc)
    {
        e = memFullErr;
    }
    catch (SPErr)
    {
        e = spErr == kSPOutOfMemoryError ? memFullErr : errPlugInHostInsufficient;
    }

    return e;
}

static OSErr WriteMipMaps(FormatRecordPtr pb, const Globals* globals)
{
    OSErr e = noErr;

    const int nPlanes = (pb->hiPlane - pb->loPlane) + 1;

    ReadChannelDesc* src[4] = { nullptr, nullptr, nullptr, nullptr };

    PSChannelPortsSuite1* suite = nullptr;
    SPErr spErr;
    BufferID scaleTemp;
    void* dataPtr = nullptr;

    if (pb->channelPortProcs != nullptr && pb->documentInfo != nullptr)
    {
        src[0] = pb->documentInfo->targetCompositeChannels;
        src[1] = src[0]->next;
        src[2] = src[1]->next;
        src[3] = pb->documentInfo->targetTransparency;
    }
    else
    {
        spErr = pb->sSPBasic->AcquireSuite(kPSChannelPortsSuite,
                                           kPSChannelPortsSuiteVersion2,
                                           const_cast<const void**>(reinterpret_cast<void**>(&suite)));
        if (spErr != kSPNoError)
        {
            return errPlugInHostInsufficient;
        }

        e = CreateImageChannelPorts(pb, suite, src, nPlanes);
    }

    if (e == noErr)
    {
        int nRows = pb->imageSize.v / 2;
        int nCols = pb->imageSize.h / 2;

        int size = (nRows * nCols) * nPlanes;
        e = pb->bufferProcs->allocateProc(size, &scaleTemp);

        if (e == noErr)
        {
            dataPtr = pb->bufferProcs->lockProc(scaleTemp, FALSE);

            PSScaling scale;
            scale.sourceRect.top = 0;
            scale.sourceRect.left = 0;
            scale.sourceRect.bottom = pb->imageSize.v;
            scale.sourceRect.right = pb->imageSize.h;

            PixelMemoryDesc dest;
            dest.data = dataPtr;
            dest.bitOffset = 0;
            dest.colBits = pb->colBytes * 8;
            dest.depth = pb->depth;

            VRect readRect;
            readRect.top = 0;
            readRect.left = 0;
            readRect.bottom = pb->imageSize.v;
            readRect.right = pb->imageSize.h;

            for (int i = 1; i <= globals->mipCount; i++)
            {
                const int width = pb->imageSize.h >> i;
                const int height = pb->imageSize.v >> i;

                scale.destinationRect.top = 0;
                scale.destinationRect.left = 0;
                scale.destinationRect.right = width;
                scale.destinationRect.bottom = height;
                dest.rowBits = width * dest.colBits;

                for (int j = 0; j < nPlanes; j++)
                {
                    if (src[j] != nullptr)
                    {
                        dest.bitOffset = j * 8;

                        if (pb->channelPortProcs != nullptr)
                        {
                            e = pb->channelPortProcs->readPixelsProc(src[j]->port, &scale, &scale.destinationRect, &dest, &readRect);

                            if (e != noErr)
                            {
                                goto cleanup;
                            }
                        }
                        else if (suite != nullptr)
                        {
                            spErr = suite->ReadScaledPixels(src[j]->port, &readRect, &scale, &dest);
                            if (spErr != kSPNoError)
                            {
                                e = spErr == kSPOutOfMemoryError ? memFullErr : errPlugInHostInsufficient;
                                goto cleanup;
                            }
                        }
                    }
                }

                e = WriteImageDataImpl(pb, dest.data, width, height, width * nPlanes, nPlanes, nPlanes, globals);

                if (e != noErr)
                {
                    break;
                }
            }
        }
    }

cleanup:
    if (suite != nullptr)
    {
        for (int i = 0; i < 4; i++)
        {
            if (src[i] != nullptr)
            {
                suite->Dispose(&src[i]->port);

                delete src[i];
                src[i] = nullptr;
            }
        }

        pb->sSPBasic->ReleaseSuite(kPSChannelPortsSuite, kPSChannelPortsSuiteVersion2);
    }

    if (dataPtr != nullptr)
    {
        pb->bufferProcs->unlockProc(scaleTemp);
        pb->bufferProcs->freeProc(scaleTemp);
    }

    return e;
}

static OSErr UpdateMipMapHeaders(HANDLE hFile, const FshDirEntry& dir, const FshBmpEntry& entry, const Globals* globals)
{
    DWORD newOffset;
    OSErr e = GetFilePosition(hFile, &newOffset);

    if (e == noErr)
    {
        e = SetFilePosition(hFile, FILE_BEGIN, dir.offset);
        if (e == noErr)
        {
            DWORD entrySize = newOffset - dir.offset;

            FshBmpEntry newEntry;
            ZeroMemory(&newEntry, sizeof(FshBmpEntry));
            newEntry.code = (entrySize << 8) | globals->fshCode;
            newEntry.width = entry.width;
            newEntry.height = entry.height;
            for (int i = 0; i < 4; i++)
            {
                newEntry.misc[i] = entry.misc[i];
            }

            newEntry.misc[3] = static_cast<UINT16>(globals->mipCount << 12);

            e = WriteFshEntryDir(hFile, dir, newEntry);
        }
    }

    return e;
}

static OSErr WriteFileLength(HANDLE hFile)
{
    OSErr e = noErr;
    INT64 fileSize;

    // Update the header with the new file size.
    e = GetFileSize(hFile, &fileSize);
    if (e == noErr)
    {
        e = SetFilePosition(hFile, FILE_BEGIN, static_cast<LONG>(offsetof(FshHeader, size)));
        if (e == noErr)
        {
            e = WriteInt32(hFile, static_cast<INT32>(fileSize));
        }
    }

    return e;
}

static OSErr WriteImageData(FormatRecordPtr pb, const FshDirEntry& dir, const FshBmpEntry& entry, const Globals* globals)
{
    OSErr e = noErr;

    const int nPlanes = (pb->hiPlane - pb->loPlane) + 1;

    HANDLE hFile = reinterpret_cast<HANDLE>(pb->dataFork);

    e = SetFilePosition(hFile, FILE_BEGIN, dir.offset + sizeof(FshBmpEntry));

    if (e == noErr)
    {
        e = WriteImageDataImpl(pb, inDataPtr, entry.width, entry.height, pb->rowBytes, pb->colBytes, nPlanes, globals);

        if (globals->mipCount > 0 && e == noErr && ChannelPortsSuiteAvailable(pb))
        {
            e = WriteMipMaps(pb, globals);

            if (e == noErr)
            {
                e = UpdateMipMapHeaders(hFile, dir, entry, globals);
            }
        }
    }

    return e;
}

OSErr DoWritePrepare(FormatRecordPtr pb)
{
    // All buffers are allocated using the Photoshop API suites.
    pb->maxData = 0;

    return noErr;
}

OSErr DoWriteStart(FormatRecordPtr pb, const Globals* globals)
{
    // Check that the current document is 8-bit RGB.
    if (pb->imageMode != plugInModeRGBColor || pb->depth != 8)
    {
        return formatBadParameters;
    }

    const FshBmpType fshCode = globals->fshCode;

    if (pb->planes == 3)
    {
        // If the current document does not have an alpha channel, return an error if the format requires one.
        if (fshCode == DXT3 || fshCode == ThirtyTwoBit || fshCode == SixteenBitAlpha || fshCode == SixteenBit4x4)
        {
            return ShowErrorMessage(pb, REQUIRESALPHACHANNEL);
        }
    }

    if (fshCode == DXT1 || fshCode == DXT3)
    {
        // DXT compressed images must be a multiple of 4.
        if ((pb->imageSize.h & 3) != 0 || (pb->imageSize.v & 3) != 0)
        {
            return ShowErrorMessage(pb, DXTIMAGESIZE);
        }
    }

    if (globals->mipCount > 0)
    {
        if (globals->mipCount > 15)
        {
            return ShowErrorMessage(pb, TOOMANYMIPMAPS);
        }

        // The image dimensions must be divisible by the total number of mipmaps.
        if ((pb->imageSize.h % (1 << globals->mipCount)) != 0 || (pb->imageSize.v % (1 << globals->mipCount)) != 0)
        {
            return ShowErrorMessageFormat(pb, INVALIDMIPCOUNTFORMAT, globals->mipCount);
        }
    }

    OSErr e = noErr;

    FshHeader head;
    ZeroMemory(&head, sizeof(FshHeader));
    head.SHPI[0] = 'S';
    head.SHPI[1] = 'H';
    head.SHPI[2] = 'P';
    head.SHPI[3] = 'I';
    head.size = 0; // Placeholder for the real length which will be written after the image data.
    head.numBmps = 1;

    if (globals->headerDir[0] != 0)
    {
        memcpy(head.dirID, globals->headerDir, 4);
    }
    else
    {
        head.dirID[0] = 'G';
        head.dirID[1] = '2';
        head.dirID[2] = '6';
        head.dirID[3] = '4';
    }

    e = WriteFshHeader(reinterpret_cast<HANDLE>(pb->dataFork), head);
    if (e == noErr)
    {
        SETRECT(pb->theRect, 0, 0, pb->imageSize.h,pb->imageSize.v);
        pb->loPlane = 0;

        switch (fshCode)
        {
            case DXT1:
                if (pb->planes == 4)
                {
                    pb->hiPlane = 3;
                }
                else
                {
                    pb->hiPlane = 2;
                }
                break;
            case DXT3:
            case ThirtyTwoBit:
            case SixteenBitAlpha:
            case SixteenBit4x4:
                pb->hiPlane = 3;
            break;
            case TwentyFourBit:
            case SixteenBit:
                pb->hiPlane = 2;
            break;
        }

        if (fshCode == ThirtyTwoBit || fshCode == TwentyFourBit)
        {
            // Set the plane map to BGR
            pb->planeMap[0] = 2; // B
            pb->planeMap[1] = 1; // G
            pb->planeMap[2] = 0; // R
        }
        else
        {
            pb->planeMap[0] = 0; // R
            pb->planeMap[1] = 1; // G
            pb->planeMap[2] = 2; // B
        }

        // Set the plane map to the index of the transparency plane or the first alpha channel (if any).
        if (pb->transparencyPlane != 0)
        {
            pb->planeMap[3] = static_cast<int16>(pb->transparencyPlane);
        }
        else
        {
            pb->planeMap[3] = 3;
        }

        pb->colBytes = pb->hiPlane + 1;
        pb->rowBytes = pb->imageSize.h * pb->colBytes;
        pb->planeBytes = 1;

        inDataPtr = nullptr;
        e = pb->bufferProcs->allocateProc((pb->rowBytes * pb->imageSize.v), &inDataBuf);
        if (e == noErr)
        {
            pb->data = inDataPtr = pb->bufferProcs->lockProc(inDataBuf, FALSE);
        }
    }

    return e;
}

OSErr DoWriteContinue(FormatRecordPtr pb, const Globals* globals)
{
    if (pb->abortProc())
    {
        return userCanceledErr;
    }

    OSErr e = noErr;

    FshDirEntry dir;
    ZeroMemory(&dir, sizeof(FshDirEntry));

    if (globals->entryDir[0] != 0)
    {
        memcpy(dir.name, globals->entryDir, 4);
    }
    else
    {
        dir.name[0] = 'F';
        dir.name[1] = 'i';
        dir.name[2] = 'S';
        dir.name[3] = 'H';
    }

    dir.offset = sizeof(FshHeader) + sizeof(FshDirEntry);

    FshBmpEntry entry;
    ZeroMemory(&entry, sizeof(FshBmpEntry));
    entry.code = globals->fshCode;
    entry.width = pb->imageSize.h;
    entry.height = pb->imageSize.v;
    for (int m = 0; m < 4; m++)
    {
        entry.misc[m] = 0;
    }

    HANDLE hFile = reinterpret_cast<HANDLE>(pb->dataFork);

    e = WriteFshDir(hFile, dir);

    if (e == noErr)
    {
        e = WriteFshEntryDir(hFile, dir, entry);

        if (e == noErr)
        {
            e = WriteImageData(pb, dir, entry, globals);

            if (e == noErr)
            {
                // Update the header with the final length of the file.
                e = WriteFileLength(hFile);
            }
        }
    }
    pb->data = nullptr;
    SETRECT(pb->theRect, 0, 0, 0, 0);

    return e;
}

OSErr DoWriteFinish(FormatRecordPtr pb)
{
    if (inDataPtr != nullptr)
    {
        pb->bufferProcs->unlockProc(inDataBuf);
        pb->bufferProcs->freeProc(inDataBuf);
        inDataPtr = nullptr;
    }

    return noErr;
}
