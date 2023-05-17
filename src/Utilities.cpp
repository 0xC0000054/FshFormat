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

#include "Utilities.h"
#include "FshFormatPS.h"
#include <PIChannelPortsSuite.h>
#include <stdlib.h>
#include <stdio.h>

#define RequiredChannelPortsVersion 1
#define RequiredChannelPortsCount   3

#define RequiredBufferProcsVersion  2
#define RequiredBufferProcsCount    5

#define RequiredDescriptorParametersVerson   0
#define RequiredReadDescriptorProcsVersion   0
#define RequiredReadDescriptorProcsCount     18
#define RequiredWriteDescriptorProcsVersion  0
#define RequiredWriteDescriptorProcsCount    16

#define RequiredHandleProcsVersion  1
#define RequiredHandleProcsCount    6

// From Photoshop 6 SDK PIUtilities.cpp
static Boolean HostChannelPortAvailable (ChannelPortProcs *procs, Boolean *outNewerVersion)
{
    Boolean available = TRUE;		// assume procs are available
    Boolean newerVersion = FALSE;	// assume we're running under correct version

    // We want to check for this stuff in a logical order, going from things
    // that should always be present to things that "may" be present.  It's
    // always a danger checking things that "may" be present because some
    // hosts may not leave them NULL if unavailable, instead pointing to
    // other structures to save space.  So first we'll check the main
    // proc pointer, then the version, the number of routines, then some
    // of the actual routines:

    if (procs == nullptr)
    {
        available = FALSE;
    }
    else if (procs->channelPortProcsVersion < RequiredChannelPortsVersion)
    {
        available = FALSE;
    }
    else if (procs->channelPortProcsVersion > RequiredChannelPortsVersion)
    {
        available = FALSE;
        newerVersion = TRUE;
    }
    else if (procs->numChannelPortProcs < RequiredChannelPortsCount)
    {
        available = FALSE;
    }
    else if (procs->readPixelsProc == nullptr ||
             procs->writeBasePixelsProc == nullptr ||
             procs->readPortForWritePortProc == nullptr)
    {
        available = FALSE;
    }

    if (newerVersion && outNewerVersion != nullptr)
        *outNewerVersion = newerVersion;

    return available;

} // end HostChannelPortAvailable


//-------------------------------------------------------------------------------
//
//	HostBufferProcsAvailable
//
//	Determines whether the BufferProcs callback is available.
//
//	Inputs:
//		BufferProcs *proc			Pointer to BufferProcs callback.
//
//	Outputs:
//		Boolean *outNewerVersion	TRUE if the host has a newer version
//									of the procs than the plug-in.
//									FALSE if the host has the same version
//									of the procs as the plug-in.
//
//		returns TRUE				If the BufferProcs callback is available.
//		returns FALSE				If the BufferProcs callback is absent.
//
//-------------------------------------------------------------------------------

static Boolean HostBufferProcsAvailable (BufferProcs *procs, Boolean *outNewerVersion)
{

    Boolean available = TRUE;		// assume procs are available
    Boolean newerVersion = FALSE;	// assume we're running under correct version


    // We want to check for this stuff in a logical order, going from things
    // that should always be present to things that "may" be present.  It's
    // always a danger checking things that "may" be present because some
    // hosts may not leave them NULL if unavailable, instead pointing to
    // other structures to save space.  So first we'll check the main
    // proc pointer, then the version, the number of routines, then some
    // of the actual routines:

    if (procs == nullptr)
    {
        available = FALSE;
    }
    else if (procs->bufferProcsVersion < RequiredBufferProcsVersion)
    {
        available = FALSE;
    }
    else if (procs->bufferProcsVersion > RequiredBufferProcsVersion)
    {
        available = FALSE;
        newerVersion = TRUE;
    }
    else if (procs->numBufferProcs < RequiredBufferProcsCount)
    {
        available = FALSE;
    }
    else if (procs->allocateProc == nullptr ||
             procs->lockProc == nullptr ||
             procs->unlockProc == nullptr ||
             procs->freeProc == nullptr ||
             procs->spaceProc == nullptr)
    {
        available = FALSE;
    }

    if (newerVersion && outNewerVersion != nullptr)
        *outNewerVersion = newerVersion;

    return available;

} // end HostBufferProcsAvailable

  //-------------------------------------------------------------------------------
  //
  //	HostDescriptorAvailable
  //
  //	Determines whether the PIDescriptorParameters callback is available.
  //
  //	The Descriptor Parameters suite are callbacks designed for
  //	scripting and automation.  See PIActions.h.
  //
  //	Inputs:
  //		PIDescriptorParameters *procs	Pointer to Descriptor Parameters suite.
  //
  //	Outputs:
  //		Boolean *outNewerVersion		TRUE if the host has a newer version
  //										of the procs than the plug-in.
  //										FALSE if the host has the same version
  //										of the procs as the plug-in.
  //
  //		returns TRUE					If PIDescriptorParameters is available.
  //		returns FALSE					If PIDescriptorParameters is absent.
  //
  //-------------------------------------------------------------------------------

static Boolean HostDescriptorAvailable(PIDescriptorParameters *procs, Boolean *outNewerVersion)
{

    Boolean available = TRUE;		// assume procs are available
    Boolean newerVersion = FALSE;	// assume we're running under correct version

                                    // We want to check for this stuff in a logical order, going from things
                                    // that should always be present to things that "may" be present.  It's
                                    // always a danger checking things that "may" be present because some
                                    // hosts may not leave them nullptr if unavailable, instead pointing to
                                    // other structures to save space.  So first we'll check the main
                                    // proc pointer, then the version, the number of routines, then some
                                    // of the actual routines:

    if (procs == nullptr)
    {
        available = FALSE;
    }
    else if (procs->descriptorParametersVersion < RequiredDescriptorParametersVerson)
    {
        available = FALSE;
    }
    else if (procs->descriptorParametersVersion > RequiredDescriptorParametersVerson)
    {
        available = FALSE;
        newerVersion = TRUE;
    }
    else if (procs->readDescriptorProcs == nullptr || procs->writeDescriptorProcs == nullptr)
    {
        available = FALSE;
    }
    else if (procs->readDescriptorProcs->readDescriptorProcsVersion < RequiredReadDescriptorProcsVersion)
    {
        available = FALSE;
    }
    else if (procs->readDescriptorProcs->readDescriptorProcsVersion > RequiredReadDescriptorProcsVersion)
    {
        available = FALSE;
        newerVersion = TRUE;
    }
    else if (procs->readDescriptorProcs->numReadDescriptorProcs < RequiredReadDescriptorProcsCount)
    {
        available = FALSE;
    }
    else if (procs->writeDescriptorProcs->writeDescriptorProcsVersion < RequiredWriteDescriptorProcsVersion)
    {
        available = FALSE;
    }
    else if (procs->writeDescriptorProcs->writeDescriptorProcsVersion > RequiredWriteDescriptorProcsVersion)
    {
        available = FALSE;
        newerVersion = TRUE;
    }
    else if (procs->writeDescriptorProcs->numWriteDescriptorProcs < RequiredWriteDescriptorProcsCount)
    {
        available = FALSE;
    }
    else if (procs->readDescriptorProcs->openReadDescriptorProc == nullptr ||
             procs->readDescriptorProcs->closeReadDescriptorProc == nullptr ||
             procs->readDescriptorProcs->getKeyProc == nullptr ||
             procs->readDescriptorProcs->getIntegerProc == nullptr ||
             procs->readDescriptorProcs->getFloatProc == nullptr ||
             procs->readDescriptorProcs->getUnitFloatProc == nullptr ||
             procs->readDescriptorProcs->getBooleanProc == nullptr ||
             procs->readDescriptorProcs->getTextProc == nullptr ||
             procs->readDescriptorProcs->getAliasProc == nullptr ||
             procs->readDescriptorProcs->getEnumeratedProc == nullptr ||
             procs->readDescriptorProcs->getClassProc == nullptr ||
             procs->readDescriptorProcs->getSimpleReferenceProc == nullptr ||
             procs->readDescriptorProcs->getObjectProc == nullptr ||
             procs->readDescriptorProcs->getCountProc == nullptr ||
             procs->readDescriptorProcs->getStringProc == nullptr ||
             procs->readDescriptorProcs->getPinnedIntegerProc == nullptr ||
             procs->readDescriptorProcs->getPinnedFloatProc == nullptr ||
             procs->readDescriptorProcs->getPinnedUnitFloatProc == nullptr)
    {
        available = FALSE;
    }
    else if (procs->writeDescriptorProcs->openWriteDescriptorProc == nullptr ||
             procs->writeDescriptorProcs->closeWriteDescriptorProc == nullptr ||
             procs->writeDescriptorProcs->putIntegerProc == nullptr ||
             procs->writeDescriptorProcs->putFloatProc == nullptr ||
             procs->writeDescriptorProcs->putUnitFloatProc == nullptr ||
             procs->writeDescriptorProcs->putBooleanProc == nullptr ||
             procs->writeDescriptorProcs->putTextProc == nullptr ||
             procs->writeDescriptorProcs->putAliasProc == nullptr ||
             procs->writeDescriptorProcs->putEnumeratedProc == nullptr ||
             procs->writeDescriptorProcs->putClassProc == nullptr ||
             procs->writeDescriptorProcs->putSimpleReferenceProc == nullptr ||
             procs->writeDescriptorProcs->putObjectProc == nullptr ||
             procs->writeDescriptorProcs->putCountProc == nullptr ||
             procs->writeDescriptorProcs->putStringProc == nullptr ||
             procs->writeDescriptorProcs->putScopedClassProc == nullptr ||
             procs->writeDescriptorProcs->putScopedObjectProc == nullptr)
    {
        available = FALSE;
    }

    if (newerVersion && (outNewerVersion != nullptr))
        *outNewerVersion = newerVersion;

    return available;

} // end HostDescriptorAvailable

//-------------------------------------------------------------------------------
//
//	HostHandleProcsAvailable
//
//	Determines whether the HandleProcs callback is available.
//
//	The HandleProcs are cross-platform master pointers that point to
//	pointers that point to data that is allocated in the Photoshop
//	virtual memory structure.  They're reference counted and
//	managed more efficiently than the operating system calls.
//
//	WARNING:  Do not mix operating system handle creation, deletion,
//			  and sizing routines with these callback routines.  They
//			  operate differently, allocate memory differently, and,
//			  while you won't crash, you can cause memory to be
//			  allocated on the global heap and never deallocated.
//
//	Inputs:
//		HandleProcs *proc			Pointer to HandleProcs callback.
//
//	Outputs:
//		Boolean *outNewerVersion	TRUE if the host has a newer version
//									of the procs than the plug-in.
//									FALSE if the host has the same version
//									of the procs as the plug-in.
//
//		returns TRUE				If the HandleProcs callback is available.
//		returns FALSE				If the HandleProcs callback is absent.
//
//-------------------------------------------------------------------------------

static Boolean HostHandleProcsAvailable (HandleProcs *procs, Boolean *outNewerVersion)
{
    Boolean available = TRUE;		// assume procs are available
    Boolean newerVersion = FALSE;	// assume we're running under correct version

    if (procs == nullptr)
    {
        available = FALSE;
    }
    else if (procs->handleProcsVersion < RequiredHandleProcsVersion)
    {
        available = FALSE;
    }
    else if (procs->handleProcsVersion > RequiredHandleProcsVersion)
    {
        available = FALSE;
        newerVersion = TRUE;
    }
    else if (procs->numHandleProcs < RequiredHandleProcsCount)
    {
        available = FALSE;
    }
    else if (procs->newProc == nullptr ||
             procs->disposeProc == nullptr ||
             procs->getSizeProc == nullptr ||
             procs->setSizeProc == nullptr ||
             procs->lockProc == nullptr ||
             procs->unlockProc == nullptr)
    {
        available = FALSE;
    }

    if (newerVersion && outNewerVersion != nullptr)
        *outNewerVersion = newerVersion;

    return available;

} // end HostHandleProcsAvailable

static Boolean HostSPBasicSuiteAvailable(SPBasicSuite* proc)
{
    Boolean available = TRUE;

    if (proc == nullptr)
    {
        available = FALSE;
    }
    else if (proc->AcquireSuite == nullptr ||
             proc->ReleaseSuite == nullptr ||
             proc->IsEqual == nullptr ||
             proc->AllocateBlock == nullptr ||
             proc->FreeBlock == nullptr ||
             proc->ReallocateBlock == nullptr ||
             proc->Undefined == nullptr)
    {
        available = FALSE;
    }

    return available;
}

bool ChannelPortsSuiteAvailable(FormatRecordPtr pb)
{
    bool available = false;

    if (HostChannelPortAvailable(pb->channelPortProcs, nullptr) && pb->documentInfo != nullptr)
    {
        available = true;
    }
    else if (HostSPBasicSuiteAvailable(pb->sSPBasic))
    {
        PSChannelPortsSuite1* suite = nullptr;
        SPErr spErr = pb->sSPBasic->AcquireSuite(kPSChannelPortsSuite,
                                                 kPSChannelPortsSuiteVersion2,
                                                 const_cast<const void**>(reinterpret_cast<void**>(&suite)));
        if (spErr == kSPNoError)
        {
            if (suite != nullptr &&
                suite->New != nullptr &&
                suite->Dispose != nullptr &&
                suite->WritePixelsToBaseLevel != nullptr &&
                suite->ReadScaledPixels != nullptr)
            {
                available = true;
            }
            pb->sSPBasic->ReleaseSuite(kPSChannelPortsSuite, kPSChannelPortsSuiteVersion2);
        }
    }

    return available;
}

bool DescriptorSuiteAvaliable(FormatRecordPtr pb)
{
    bool available = false;

    if (HostDescriptorAvailable(pb->descriptorParameters, nullptr))
    {
        available = true;
    }

    return available;
}

OSErr ShowErrorMessage(FormatRecordPtr pb, const UINT resourceId)
{
    wchar_t buffer[256];
    ZeroMemory(buffer, sizeof(buffer));

    if (LoadStringW(reinterpret_cast<HINSTANCE>(hDllInstance), resourceId, buffer, _countof(buffer)) > 0)
    {
        PlatformData* platform = reinterpret_cast<PlatformData*>(pb->platformData);

        if (MessageBoxW(reinterpret_cast<HWND>(platform->hwnd), buffer, L"Fsh Format", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_APPLMODAL) == IDOK)
        {
            // Any positive integer is a plugin handled error.
            return 1;
        }
    }

    return formatBadParameters;
}

OSErr ShowErrorMessageFormat(FormatRecordPtr pb, const UINT resourceId, ...)
{
    wchar_t format[256];
    ZeroMemory(format, sizeof(format));

    if (LoadStringW(reinterpret_cast<HINSTANCE>(hDllInstance), resourceId, format, _countof(format)) > 0)
    {
        va_list args;
        va_start(args, resourceId);

        wchar_t buffer[256];
        int retVal = _vsnwprintf_s(buffer, _countof(buffer), format, args);

        va_end(args);

        if (retVal > 0)
        {
            PlatformData* platform = reinterpret_cast<PlatformData*>(pb->platformData);

            if (MessageBoxW(reinterpret_cast<HWND>(platform->hwnd), buffer, L"Fsh Format", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_APPLMODAL) == IDOK)
            {
                // Any positive integer is a plugin handled error.
                return 1;
            }
        }
    }

    return formatBadParameters;
}

bool CheckForRequiredSuites(FormatRecordPtr pb)
{
    bool available = false;

    if (HostBufferProcsAvailable(pb->bufferProcs, nullptr) && HostHandleProcsAvailable(pb->handleProcs, nullptr))
    {
        available = true;
    }

    return available;
}