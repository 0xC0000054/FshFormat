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
#include "ui.h"
#include <new>

OSErr DoOptionsPrepare(FormatRecordPtr pb)
{
    pb->maxData = 0;

    return noErr;
}

OSErr DoOptionsStart(FormatRecordPtr pb, Globals* globals)
{
    // Check that the current image is 8-bit RGB.
    if (pb->imageMode != plugInModeRGBColor || pb->depth != 8)
    {
        return formatBadParameters;
    }

    if (pb->revertInfo != nullptr)
    {
        if (pb->handleProcs->getSizeProc(pb->revertInfo) == sizeof(RevertInfo))
        {
            RevertInfo* rev = reinterpret_cast<RevertInfo*>(pb->handleProcs->lockProc(pb->revertInfo, FALSE));
            globals->fshCode = rev->fshCode;
            memcpy(globals->entryDir, rev->entryDir, 4);
            memcpy(globals->headerDir, rev->headerDir, 4);
            globals->mipCount = rev->mipCount;
            globals->mipPacked = rev->mipPacked;

            pb->handleProcs->unlockProc(pb->revertInfo);
        }
    }

    OSErr e = noErr;

    Boolean	showDialog = ReadScriptParamsOnWrite(pb, globals, &e);

    if (e == noErr && showDialog)
    {
        SaveDialogOptions options;
        ZeroMemory(&options, sizeof(SaveDialogOptions));

        if (SaveFshDlg(pb, globals, &options))
        {
            globals->fshCode = options.fshType;
            globals->fshWriteCompression = options.fshWriteCompression;
            if (options.entryDirName[0] != 0)
            {
                memcpy(globals->entryDir, options.entryDirName, 4);
            }

            if (options.embedMipmaps && globals->mipCount == 0)
            {
                int width = pb->imageSize.h;
                int height = pb->imageSize.v;
                int numScales = 0;

                while (width > 1 && height > 1)
                {
                    numScales++;
                    width >>= 1;
                    height >>= 1;
                }

                globals->mipCount = numScales;
            }
        }
        else
        {
            e = userCanceledErr;
        }
    }
    pb->data = nullptr;

    return e;
}

OSErr DoOptionsContinue(FormatRecordPtr pb)
{
    UNREFERENCED_PARAMETER(pb);

    return noErr;
}

OSErr DoOptionsFinish(FormatRecordPtr pb, const Globals *globals)
{
    return WriteScriptParamsOnWrite(pb, globals);
}