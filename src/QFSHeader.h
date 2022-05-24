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
