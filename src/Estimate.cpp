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