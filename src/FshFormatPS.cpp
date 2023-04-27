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

static OSErr CreateGlobals(FormatRecordPtr pb, intptr_t* data)
{
    OSErr e = noErr;

    if (CheckForRequiredSuites(pb))
    {
        Handle h = pb->handleProcs->newProc(sizeof(Globals));
        if (h != nullptr)
        {
            *data = reinterpret_cast<intptr_t>(h);
        }
        else
        {
            e = memFullErr;
        }
    }
    else
    {
        e = errPlugInHostInsufficient;
    }

    return e;
}

static OSErr DoFilterFile(FormatRecordPtr pb)
{
    return IsValidFshFile(pb);
}

DLLExport MACPASCAL void PluginMain (const short selector, FormatRecordPtr pb, intptr_t* data, short* result)
{
    if (selector == formatSelectorAbout)
    {
        DoAbout((AboutRecordPtr)pb);
    }
    else
    {
        Globals* globals;
        if (*data == 0)
        {
            *result = CreateGlobals(pb, data);
            if (*result != noErr)
            {
                return;
            }
            globals = reinterpret_cast<Globals*>(pb->handleProcs->lockProc(reinterpret_cast<Handle>(*data), FALSE));
            ZeroMemory(globals, sizeof(Globals));
            globals->fshCode = DXT1;
            globals->fshWriteCompression = true;
        }
        else
        {
            globals = reinterpret_cast<Globals*>(pb->handleProcs->lockProc(reinterpret_cast<Handle>(*data), FALSE));
        }

        switch (selector)
        {
            case formatSelectorReadPrepare:
                *result = DoReadPrepare(pb);
                break;
            case formatSelectorReadStart:
                *result = DoReadStart(pb);
                break;
            case formatSelectorReadContinue:
                *result = DoReadContinue(pb);
                break;
            case formatSelectorReadFinish:
                *result = DoReadFinish(pb);
                break;

            case formatSelectorOptionsPrepare:
                *result = DoOptionsPrepare(pb);
                break;
            case formatSelectorOptionsStart:
                *result = DoOptionsStart(pb, globals);
                break;
            case formatSelectorOptionsContinue:
                *result = DoOptionsContinue(pb);
                break;
            case formatSelectorOptionsFinish:
                *result = DoOptionsFinish(pb, globals);
                break;

            case formatSelectorEstimatePrepare:
                *result = DoEstimatePrepare(pb);
                break;
            case formatSelectorEstimateStart:
                *result = DoEstimateStart(pb, globals);
                break;
            case formatSelectorEstimateContinue:
                *result = DoEstimateContinue(pb);
                break;
            case formatSelectorEstimateFinish:
                *result = DoEstimateFinish();
                break;

            case formatSelectorWritePrepare:
                *result = DoWritePrepare(pb);
                break;
            case formatSelectorWriteStart:
                *result = DoWriteStart(pb, globals);
                break;
            case formatSelectorWriteContinue:
                *result = DoWriteContinue(pb, globals);
                break;
            case formatSelectorWriteFinish:
                *result = DoWriteFinish(pb);
                break;

            case formatSelectorFilterFile:
                *result = DoFilterFile(pb);
                break;

            default:
                *result = formatBadParameters;
        }

        pb->handleProcs->unlockProc(reinterpret_cast<Handle>(*data));
    }
}