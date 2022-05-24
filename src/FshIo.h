#ifndef FshIo_H // only include this once
#define FshIo_H

	struct FshHeader
	{
		char SHPI[4];
		INT32 size;
		INT32 numBmps;
		char dirID[4];
	};

	struct FshDirEntry
	{
		char name[4];
		INT32 offset;
	};

	struct FshBmpEntry
	{
		INT32 code;
		UINT16 width;
		UINT16 height;
		UINT16 misc[4];
	};

	static_assert(sizeof(FshHeader) == 16, "sizeof(FshHeader) != 16");
	static_assert(sizeof(FshDirEntry) == 8, "sizeof(FshDirEntry) != 8");
	static_assert(sizeof(FshBmpEntry) == 16, "sizeof(FshBmpEntry) != 16");


	enum FshBmpType
	{
		// DXT1 Compressed, 4x4 packed with 1-bit alpha.
		DXT1 = 0x60,
		// DXT3 Compressed, 4x4 packed with 4-bit alpha.
		DXT3 = 0x61,
		// 32-bit ARGB (8:8:8:8)
		ThirtyTwoBit = 0x7D,
		// 24-bit RGB (0:8:8:8)
		TwentyFourBit = 0x7F,
		// 16-bit RGB (0:5:6:5)
		SixteenBit = 0x78,
		// 16-bit ARGB (1:5:5:5)
		SixteenBitAlpha = 0x7E,
		// 16-bit ARGB (4:4:4:4)
		SixteenBit4x4 = 0x6D,
		// 8-bit Indexed, unsupported but included for completeness.
		// EightBit = 0x7B
	};

	// Counts the number of mipmaps contained in the specified image entry.
	OSErr CountEntryMipMaps(HANDLE file, const FshHeader& header, const FshDirEntry& dir, const FshBmpEntry& entry, int* count, bool* packed);

	// Checks the file for unsupported image formats.
	OSErr CheckFshBmpTypes(HANDLE file, const FshHeader& header);
	// Decompresses a QFS compressed file.
	OSErr DecompressFsh(FormatRecordPtr pb, HANDLE file);

	OSErr GetFshBmpInfo(HANDLE file, const int index, FshBmpEntry* entry);
	int GetImageDataSize(const int width, const int height, const FshBmpType format);

	// Reads the header and validates the file signature.
	OSErr ReadFshHeader(HANDLE file, FshHeader* header);
	// Reads the file directory at the specified index.
	OSErr ReadFshDir(HANDLE file, const int index, FshDirEntry* dir);
	// Reads the image entry.
	OSErr ReadFshEntryDir(HANDLE file, const FshDirEntry& dir, FshBmpEntry* entry);
	// Reads the image data.
	OSErr ReadFshImageData(FormatRecordPtr pb,
						   const FshHeader& header,
						   const FshDirEntry& dir,
						   const FshBmpEntry& entry,
						   void* outData,
						   const DWORD outLength);
	// Writes the header.
	OSErr WriteFshHeader(HANDLE file, const FshHeader& header);
	// Writes the file directory entry.
	OSErr WriteFshDir(HANDLE file, const FshDirEntry& dir);
	// Writes the image entry.
	OSErr WriteFshEntryDir(HANDLE file, const FshDirEntry& dir, const FshBmpEntry& entry);
	// Writes the image data.
	OSErr WriteFshImageData(HANDLE file, const void* inData, const int length);

	// Determines weather the image is a valid Fsh file.
	OSErr IsValidFshFile(FormatRecordPtr pb);

#endif