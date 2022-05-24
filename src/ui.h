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