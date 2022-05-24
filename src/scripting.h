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