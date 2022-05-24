#ifndef FSHFORMATPS_H
#define FSHFORMATPS_H

#pragma warning(push)
#pragma warning(disable: 4121)

#include "PIDefines.h"
#include "PIFormat.h"
#include "PIAbout.h"

#pragma warning(pop)

#include "FileIo.h"
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

// Utility functions
bool ChannelPortsSuiteAvailable(FormatRecordPtr pb);
bool DescriptorSuiteAvaliable(FormatRecordPtr pb);
OSErr ShowErrorMessage(FormatRecordPtr pb, const UINT resourceId);
OSErr ShowErrorMessageFormat(FormatRecordPtr pb, const UINT resourceId, ...);
bool CheckForRequiredSuites(FormatRecordPtr pb);

#define SETRECT(rect,l,t,r,b) ((rect).left=(l),(rect).top=(t),(rect).right=(r),(rect).bottom=(b))

#endif // FSHPSTEST_H