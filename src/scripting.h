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

#ifndef SCRIPTING_H
#define SCRIPTING_H

#include "PIActions.h"
#include "PITerminology.h"

#ifndef NULLID
#define NULLID 0
#endif

#define vendorName			"FshFmt"
#define plugInAETEComment	"Fsh File format module"
#define plugInSuiteID		'fshF'
#define plugInClassID		plugInSuiteID
#define plugInEventID		typeNull



#define keyFshFormat 'fshT'
#define keyHeaderDirID 'fshD'
#define keyEntryDirName  'eDir'
#define keyFshWriteComp 'fshW'
#define keyMipCount  'mipC'
#define keyMipPacked  'mipP'

#define fshFormatEnum			'bmpT'

#define type24BitRGB	'fmT0'
#define type32BitARGB		'fmT1'
#define typeDXT1		'fmT2'
#define typeDXT3		'fmT3'
#define type16BitRGB		'fmT4'
#define type16BitARGB		'fmT5'
#define type16Bit4x4		'fmT6'

#endif