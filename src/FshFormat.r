//*******************************************************************/
//*                                                                 */
//*                      ADOBE CONFIDENTIAL                         */
//*                   _ _ _ _ _ _ _ _ _ _ _ _ _                     */
//*                                                                 */
//* Copyright 1993 - 1999 Adobe Systems Incorporated                */
//* All Rights Reserved.                                            */
//*                                                                 */
//* NOTICE:  All information contained herein is, and remains the   */
//* property of Adobe Systems Incorporated and its suppliers, if    */
//* any.  The intellectual and technical concepts contained         */
//* herein are proprietary to Adobe Systems Incorporated and its    */
//* suppliers and may be covered by U.S. and Foreign Patents,       */
//* patents in process, and are protected by trade secret or        */
//* copyright law.  Dissemination of this information or            */
//* reproduction of this material is strictly forbidden unless      */
//* prior written permission is obtained from Adobe Systems         */
//* Incorporated.                                                   */
//*                                                                 */
//*******************************************************************/
//-------------------------------------------------------------------
//-------------------------------------------------------------------------------
//
//	File:
//		SimpleFormat.r
//
//	Description:
//		This file contains the resource definitions for the
//		File Format module SimpleFormat, 
//		which writes a flat file with merged document pixels.
//
//	Use:
//		Format modules are called from the Save, Save as,
//		and Save a copy dialogs.
//
//	Version history:
//		Version 1.0.0	1/1/1993	Created for Photoshop 3.0.
//			Written by Mark Hamburg
//
//		Version 2.0.0	5/27/1996	Updated for Photoshop 4.0.
//			Scripting added.
//
//		Version 2.1.0	6/28/1997	Updated for Photoshop 4.0.1.
//			Updated for new version of Photoshop and projects
//			moved to CodeWarrior Pro.
//
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
//	Definitions -- Required by include files.
//-------------------------------------------------------------------------------

// The About box and resources are created in DialogUtilities.r.
// You can easily override them, if you like.

#define plugInName			"FshFormat"
#define plugInDescription \
	"Fsh File Format Module for Adobe Photoshop."
//-------------------------------------------------------------------------------
//	Set up included files for Macintosh and Windows.
//-------------------------------------------------------------------------------

#include "PIDefines.h"
#include "scripting.h"

#ifdef __PIMac__
	#include "Types.r"
	#include "SysTypes.r"
	#include "PIGeneral.r"
	#include "PIUtilities.r"
	#include "DialogUtilities.r"
#elif defined(__PIWin__)
	#include "PIGeneral.h"
	#include "PIUtilities.r"
	#include "WinDialogUtils.r"
#endif

//-------------------------------------------------------------------------------
//	PiPL resource
//-------------------------------------------------------------------------------

resource 'PiPL' (ResourceID, plugInName " PiPL", purgeable)
{
    {
		Kind { ImageFormat },
		Name { plugInName },
		Version { (latestFormatVersion << 16) | latestFormatSubVersion },

#if WIN64
		CodeWin64X86 { "PluginMain" },
#else
		CodeWin32X86 { "PluginMain" },
#endif	
		HasTerminology { plugInClassID, 
		                 plugInEventID, 
						 ResourceID, 
						 vendorName " " plugInName },
	
		SupportedModes
		{
			noBitmap, noGrayScale,
			noIndexedColor, doesSupportRGBColor,
			noCMYKColor, noHSLColor,
			noHSBColor, noMultichannel,
			noDuotone, noLABColor
		},
			
		EnableInfo { "in( PSHOP_ImageMode, RGBMode)" },
	
		FmtFileType { '8B1F', '8BIM' },
		//ReadTypes { { '8B1F', '    ' } },
		FilteredTypes { { '8B1F', '    ' } },
		ReadExtensions { { 'FSH ' } },
		WriteExtensions { { 'FSH ' } },
		FilteredExtensions { { 'FSH ' } },
		FormatFlags { fmtDoesNotSaveImageResources, fmtCanRead, fmtCanWrite, fmtCanWriteIfRead, fmtCanWriteTransparency },
		/* canRead, canWrite, canWriteIfRead, savesResources */
		FormatMaxSize { { 32767, 32767 } },
		FormatMaxChannels { {   0, 0, 0, 4,0, 0, 
							   0, 0, 0, 0, 0, 0} }
	}
};

//-------------------------------------------------------------------------------
//	Dictionary (scripting) resource
//-------------------------------------------------------------------------------

resource 'aete' (ResourceID, plugInName " dictionary", purgeable)
{
	1, 0, english, roman,									/* aete version and language specifiers */
	{
		vendorName,											/* vendor suite name */
		"Fsh File format plugin",							/* optional description */
		plugInSuiteID,										/* suite ID */
		1,													/* suite code, must be 1 */
		1,													/* suite level, must be 1 */
		{},													/* structure for filters */
		{													/* non-filter plug-in class here */
			vendorName " fshFormat",						/* unique class name */
			plugInClassID,									/* class ID, must be unique or Suite ID */
			plugInAETEComment,								/* optional description */
			{												/* define inheritance */
				"<Inheritance>",							/* must be exactly this */
				keyInherits,								/* must be keyInherits */
				classFormat,								/* parent: Format, Import, Export */
				"parent class format",						/* optional description */
				flagsSingleProperty,						/* if properties, list below */
							
				"radioSelect",
				keyFshRadioSelect,
				fshFormatEnum,
				"Fsh Format",
				flagsEnumeratedParameter,
				
				"headerID",
				keyHeaderDirID,
				typeChar,
				"Fsh Header ID",
				flagsSingleProperty,

				"entryName",
				keyEntryDirName,
				typeChar,
				"Fsh Entry Name",
				flagsSingleProperty
				
				"fshWriteCompression",
				keyFshWriteComp,
				typeBoolean,
				"FshWrite Compression",
				flagsSingleProperty
				/* no properties */
			},
			{}, /* elements (not supported) */
			/* class descriptions */
		},
		{}, /* comparison ops (not supported) */
		{
			fshFormatEnum,										/* type disposition 'bmpT' */
			{
				"24-bit",									/* first value */
				type24BitRGB,							/* 'fmT0' */
				"24-bit RGB",								/* optional description */
				
				"32-bit",										/* second value */
				type32BitARGB,							/* 'moo1' */
				"32-bit ARGB",							/* optional description */
				
				"DXT1",										/* third value */
				typeDXT1,								/* 'moo2' */
				"DXT1 Compressed",								/* optional description */
				
				"DXT3",										/* fourth value */
				typeDXT3,							/* 'moo3' */
				"DXT3 Compressed"

				"16-bitRGB",										/* second value */
				type16BitRGB,							/* 'moo1' */
				"16-bit RGB",							/* optional description */
				
				"16-bitARGB",										/* third value */
				type16BitARGB,								/* 'moo2' */
				"16-bit ARGB",								/* optional description */
				
				"16-bit4x4",										/* fourth value */
				type16Bit4x4,							/* 'moo3' */
				"16-bit 4x4 packed"	

				/* optional description */
			}
		}	/* any enumerations */
	}
};


// end FshFormat.r
