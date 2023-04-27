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

#ifndef QFSHEADER_H
#define QFSHEADER_H

#include <Windows.h>

class QFSHeader
{
public:
	QFSHeader(const BYTE* data, const DWORD dataLength);
	QFSHeader(const HANDLE hFile, const LONG offset);

	static bool CheckSignature(const BYTE (&data)[2]);

	int GetDataStartOffset() const;
	int GetUncompressedSize() const;

private:
	QFSHeader(const QFSHeader& copyMe);

	int uncompressedSize;
	int dataStartOffset;
};

#endif
