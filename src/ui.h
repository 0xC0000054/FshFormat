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

#ifndef UI_H 
#define UI_H
#include "FshFormatPS.h"

struct SaveDialogOptions
{
	FshBmpType fshType;
	char entryDirName[4];
	bool fshWriteCompression;
	bool embedMipmaps;
};

bool LoadFshDlg(FormatRecordPtr pb, const FshHeader& header, int* selectedIndex);
bool SaveFshDlg(FormatRecordPtr pb, const Globals* globals, SaveDialogOptions* params);

#endif