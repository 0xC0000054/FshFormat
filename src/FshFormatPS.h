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

#ifndef FSHFORMATPS_H
#define FSHFORMATPS_H

#include "FshIo.h"

extern HANDLE hDllInstance;

struct RevertInfo
{
	FshBmpType fshCode;
	int mipCount;
	bool mipPacked;
	char headerDir[4];
	char entryDir[4];
	int loadIndex;
};

struct Globals
{
	FshBmpType fshCode;
	bool fshWriteCompression;
	int mipCount;
	bool mipPacked;
	char headerDir[4];
	char entryDir[4];
};

//-------------------------------------------------------------------------------
//	Prototypes
//-------------------------------------------------------------------------------

// Everything comes in and out of PluginMain. It must be first routine in source:
DLLExport MACPASCAL void PluginMain (const short selector,
									 FormatRecordPtr formatParamBlock,
									 intptr_t* data,
									 short* result);

void DoAbout (AboutRecordPtr about); 	   		// Pop about box.

OSErr DoReadPrepare(FormatRecordPtr pb);
OSErr DoReadStart(FormatRecordPtr pb);
OSErr DoReadContinue(FormatRecordPtr pb);
OSErr DoReadFinish(FormatRecordPtr pb);

OSErr DoEstimatePrepare(FormatRecordPtr pb);
OSErr DoEstimateStart(FormatRecordPtr pb, const Globals* globals);
OSErr DoEstimateContinue(FormatRecordPtr pb);
OSErr DoEstimateFinish();

OSErr DoOptionsPrepare(FormatRecordPtr pb);
OSErr DoOptionsStart(FormatRecordPtr pb, Globals* globals);
OSErr DoOptionsContinue(FormatRecordPtr pb);
OSErr DoOptionsFinish(FormatRecordPtr pb, const Globals* globals);

OSErr DoWritePrepare(FormatRecordPtr pb);
OSErr DoWriteStart(FormatRecordPtr pb, const Globals* globals);
OSErr DoWriteContinue(FormatRecordPtr pb, const Globals* globals);
OSErr DoWriteFinish(FormatRecordPtr pb);

// Scripting
Boolean ReadScriptParamsOnWrite(FormatRecordPtr pb, Globals* globals, OSErr* error);
OSErr WriteScriptParamsOnWrite(FormatRecordPtr pb, const Globals* globals);

#define SETRECT(rect,l,t,r,b) ((rect).left=(l),(rect).top=(t),(rect).right=(r),(rect).bottom=(b))

#endif // FSHPSTEST_H