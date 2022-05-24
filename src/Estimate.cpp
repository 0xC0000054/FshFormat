#include "FshFormatPS.h"

static int32 CalculateFshSize(const int imageWidth, const int imageHeight, const Globals* globals)
{
    int32 size = sizeof(FshHeader) + sizeof(FshDirEntry) + sizeof(FshBmpEntry);

    size += GetImageDataSize(imageWidth, imageHeight, globals->fshCode);

    for (int i = 1; i <= globals->mipCount; i++)
    {
        const int mipWidth = imageWidth >> i;
        const int mipHeight = imageHeight >> i;

        size += GetImageDataSize(mipWidth, mipHeight, globals->fshCode);

        if (!globals->mipPacked && globals->fshCode != DXT1 || globals->fshCode == DXT1 && (mipWidth == 1 || mipHeight == 1))
        {
            // Pad the size to a multiple of 16 bytes.
            int padSize = ((16 - size) & 15);
            if (padSize > 0)
            {
                size += padSize;
            }
        }
    }

    return size;
}

OSErr DoEstimatePrepare(FormatRecordPtr pb)
{
    pb->maxData = 0;

    return noErr;
}

OSErr DoEstimateStart(FormatRecordPtr pb, const Globals* globals)
{
    pb->minDataBytes = pb->maxDataBytes = CalculateFshSize(pb->imageSize.h, pb->imageSize.v, globals);

    pb->data = nullptr;
    SETRECT(pb->theRect, 0, 0, 0, 0);

    return noErr;
}

OSErr DoEstimateContinue(FormatRecordPtr pb)
{
    UNREFERENCED_PARAMETER(pb);

    return noErr;
}

OSErr DoEstimateFinish()
{
    return noErr;
}