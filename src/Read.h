#include "FshIo.h"

OSErr GetFshBmpInfo(HANDLE file, FshDirEntry *dir, FshBmpEntry *entry, int i);
OSErr ReadFsh(FormatRecordPtr pb, FshDirEntry dir, FshBmpEntry entry)
