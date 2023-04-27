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
#include "scripting.h"

//-------------------------------------------------------------------------------
//
// DialogToScript
//
// Convert a dialog variable to a scripting variable.
//
//-------------------------------------------------------------------------------
static int32 DialogToScript(const FshBmpType dialog)
{
    switch (dialog)
    {
    case TwentyFourBit:
        return type24BitRGB;
    case ThirtyTwoBit:
        return type32BitARGB;
    case DXT1:
        return typeDXT1;
    case DXT3:
        return typeDXT3;
    case SixteenBit:
        return type16BitRGB;
    case SixteenBitAlpha:
        return type16BitARGB;
    case SixteenBit4x4:
        return type16Bit4x4;
    }

    return typeDXT3;
}

//-------------------------------------------------------------------------------
//
// ScriptToDialog
//
// Convert a scripting variable to a dialog variable.
//
//-------------------------------------------------------------------------------
static FshBmpType ScriptToDialog(const int32 script)
{
    switch (script)
    {
        case type24BitRGB:
            return TwentyFourBit;
        case type32BitARGB:
            return ThirtyTwoBit;
        case typeDXT1:
            return DXT1;
        case typeDXT3:
            return DXT3;
        case type16BitRGB:
            return SixteenBit;
        case type16BitARGB:
            return SixteenBitAlpha;
        case type16Bit4x4:
            return SixteenBit4x4;
    }

    return DXT3;
}

//-------------------------------------------------------------------------------
//
//	ReadScriptParamsOnWrite
//
//	Checks the parameters against scripting-returned parameters, if any, and
//	updates the globals to match ones provided by the scripting system.
//
//	Inputs:
//		FormatRecordPtr         Pointer to the FormatRecord
//      Globals			        Pointer to global structure.
//
//	Outputs:
//		returns TRUE		If you should pop your dialog.
//		returns FALSE		If you should not pop your dialog.
//
//		error				Will return any fatal error.
//
//-------------------------------------------------------------------------------

Boolean ReadScriptParamsOnWrite(FormatRecordPtr pb, Globals* globals, OSErr* error)
{
    DescriptorKeyID				key = 0;
    DescriptorTypeID			type = 0;
    DescriptorKeyIDArray		array = { keyFshFormat, keyHeaderDirID, keyEntryDirName, keyFshWriteComp, keyMipCount, keyMipPacked, NULLID };
    int32						flags = 0;

    *error = noErr;
    Boolean showDialog = TRUE;

    if (DescriptorSuiteAvaliable(pb))
    {
        ReadDescriptorProcs* readProcs = pb->descriptorParameters->readDescriptorProcs;

        PIReadDescriptor token = readProcs->openReadDescriptorProc(pb->descriptorParameters->descriptor, array);
        if (token != nullptr)
        {
            DescriptorEnumID format;
            Boolean b;
            int32 temp;

            while (readProcs->getKeyProc(token, &key, &type, &flags))
            {
                OSErr e = noErr;
                Handle h = nullptr;

                switch (key)
                {
                case keyFshFormat:
                    readProcs->getEnumeratedProc(token, &format);
                    globals->fshCode = ScriptToDialog(format);
                    break;
                case keyHeaderDirID:
                    e = readProcs->getTextProc(token, &h);
                    if (e == noErr && h != nullptr)
                    {
                        ZeroMemory(globals->headerDir, 4);

                        Ptr p = pb->handleProcs->lockProc(h, FALSE);
                        memcpy(globals->headerDir, p, 4);
                        pb->handleProcs->unlockProc(h);
                        pb->handleProcs->disposeProc(h);
                    }
                    break;
                case keyEntryDirName:
                    e = readProcs->getTextProc(token, &h);

                    if (e == noErr && h != nullptr)
                    {
                        ZeroMemory(globals->entryDir, 4);
                        Ptr p = pb->handleProcs->lockProc(h, FALSE);
                        memcpy(globals->entryDir, p, 4);
                        pb->handleProcs->unlockProc(h);
                        pb->handleProcs->disposeProc(h);
                    }
                    break;
                case keyFshWriteComp:
                    if (readProcs->getBooleanProc(token, &b) == noErr)
                    {
                        globals->fshWriteCompression = (b != 0);
                    }
                    break;
                case keyMipCount:
                    if (readProcs->getIntegerProc(token, &temp) == noErr)
                    {
                        globals->mipCount = temp;
                    }
                    break;
                case keyMipPacked:
                    if (readProcs->getBooleanProc(token, &b) == noErr)
                    {
                        globals->mipPacked = (b != 0);
                    }
                    break;
                }

            }

            *error = readProcs->closeReadDescriptorProc(token); // closes & disposes.

            if (*error == errMissingParameter)
            {
                // A missing parameter is not a fatal error.
                *error = noErr;
            }

            // Dispose the parameter block descriptor:
            pb->handleProcs->disposeProc(pb->descriptorParameters->descriptor);
            pb->descriptorParameters->descriptor = nullptr;

            showDialog = (pb->descriptorParameters->playInfo == plugInDialogDisplay);
        }
    }

    return showDialog;
}

static OSErr PutPIText(FormatRecordPtr pb, const PIWriteDescriptor token, const DescriptorKeyID key, const char *s, const int length)
{
    OSErr e = noErr;

    Handle h = pb->handleProcs->newProc(length);

    if (h != nullptr)
    {
        Ptr p = pb->handleProcs->lockProc(h, FALSE);
        memcpy(p, s, length);
        pb->handleProcs->unlockProc(h);

        e = pb->descriptorParameters->writeDescriptorProcs->putTextProc(token, key, h);
        pb->handleProcs->disposeProc(h);
    }
    else
    {
        e = memFullErr;
    }

    return e;
}

//-------------------------------------------------------------------------------
//
//	WriteScriptParamsOnWrite
//
//	Takes our parameters from our global variables and writes them out to the
//	scripting system, which hands it all back to the host.
//
//	Inputs:
//		FormatRecordPtr         Pointer to the FormatRecord
//      Globals			        Pointer to global structure.
//
//	Outputs:
//		returns an OSErr		If there was a serious error.
//		returns noErr			If everything worked fine.
//
//-------------------------------------------------------------------------------

OSErr WriteScriptParamsOnWrite(FormatRecordPtr pb, const Globals *globals)
{
    OSErr						err = noErr;
    PIDescriptorHandle			h;

    if (DescriptorSuiteAvaliable(pb))
    {
        WriteDescriptorProcs* writeProcs = pb->descriptorParameters->writeDescriptorProcs;

        PIWriteDescriptor token = writeProcs->openWriteDescriptorProc();
        if (token != nullptr)
        {
            writeProcs->putEnumeratedProc(token, keyFshFormat, fshFormatEnum, DialogToScript(globals->fshCode));
            if (globals->headerDir[0] != 0)
            {
                PutPIText(pb, token, keyHeaderDirID, globals->headerDir, 4);
            }

            if (globals->entryDir[0] != 0)
            {
                PutPIText(pb, token, keyEntryDirName, globals->entryDir, 4);
            }

            if (!globals->fshWriteCompression)
            {
                writeProcs->putBooleanProc(token, keyFshWriteComp, FALSE);
            }

            if (globals->mipCount > 0)
            {
                writeProcs->putIntegerProc(token, keyMipCount, globals->mipCount);
                if (globals->mipPacked)
                {
                    writeProcs->putBooleanProc(token, keyMipPacked, TRUE);
                }
            }

            pb->handleProcs->disposeProc(pb->descriptorParameters->descriptor);
            err = writeProcs->closeWriteDescriptorProc(token, &h);
            pb->descriptorParameters->descriptor = h;
        }
    }

    return err;
}
