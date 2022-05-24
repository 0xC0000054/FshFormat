#ifndef FileIo_H // only include this once
#define FileIo_H

OSErr GetFilePosition(HANDLE hFile, DWORD* filePos);
OSErr SetFilePosition(HANDLE hFile, const DWORD posMode, const LONG posOff);
OSErr GetFileSize(HANDLE hFile, INT64* size);
OSErr ReadBytes(HANDLE hFile, void* buffPtr, const DWORD count); 
OSErr ReadInt32(HANDLE hFile, INT32* val);
OSErr ReadUInt16(HANDLE hFile, UINT16* val);
OSErr WriteBytes(HANDLE hFile, const void* buffPtr, const DWORD count); 
OSErr WriteInt32(HANDLE hFile, const INT32 val);
OSErr WriteUInt16(HANDLE hFile, const UINT16 val);

#endif