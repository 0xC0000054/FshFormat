#ifndef QFS_H
#define QFS_H

OSErr QFSDecompress(const BYTE* inData, const DWORD inLength, BYTE* outData, const DWORD outLength);
OSErr GetUncompressedSize(const BYTE* inData, const DWORD inLength, int* uncompressedSize);
OSErr GetUncompressedSize(HANDLE hFile, LONG offset, int* uncompressedSize);
OSErr IsQFSCompressed(HANDLE hFile, LONG offset, bool* isCompressed);

extern BufferID qfsBufferID;
extern BYTE* qfsBuffer;

#endif