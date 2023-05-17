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

#include "QFSHeader.h"
#include "FileIo.h"

enum QFSHeaderFlags
{
    QFS_HEADER_FLAG_COMPRESSED_SIZE_PRESENT = 0x00000001,
    QFS_HEADER_FLAG_UNKNOWN = 0x00000040,
    QFS_HEADER_FLAG_LARGE_FILES = 0x00000080,
    QFS_HEADER_FLAG_MASK = ~(QFS_HEADER_FLAG_COMPRESSED_SIZE_PRESENT | QFS_HEADER_FLAG_UNKNOWN | QFS_HEADER_FLAG_LARGE_FILES)
};

QFSHeader::QFSHeader(const BYTE* data, const DWORD dataLength)
{
    if (dataLength < 5)
    {
        throw (OSErr)formatCannotRead; // Truncated or invalid data.
    }

    int signatureOffset;

    // The Need For Speed compression does not prefix the compressed data length unlike SC4.
    if ((data[0] & QFS_HEADER_FLAG_MASK) == 0x10 && data[1] == 0xFB)
    {
        signatureOffset = 0;
    }
    else
    {
        if ((data[4] & QFS_HEADER_FLAG_MASK) == 0x10 && data[5] == 0xFB)
        {
            signatureOffset = 4;
        }
        else
        {
            throw (OSErr)formatCannotRead; // Unknown compression format.
        }
    }

    int index = signatureOffset + 2;

    // The first byte of the QFS header contains the compression flags.
    // See http://wiki.niotso.org/RefPack#Header

    // Sim City 4 introduced support for 32-bit file sizes.
    // TODO: Test if the game actually supports this.

    const bool largeFileLength = (data[signatureOffset] & QFS_HEADER_FLAG_LARGE_FILES) != 0;

    const int sizeFieldByteCount = largeFileLength ? 4 : 3;

    if ((data[signatureOffset] & QFS_HEADER_FLAG_COMPRESSED_SIZE_PRESENT) != 0)
    {
        // Some files may prefix the compressed file size after the signature.
        index += sizeFieldByteCount;
    }

    dataStartOffset = index + sizeFieldByteCount;

    if (static_cast<DWORD>(dataStartOffset) >= dataLength)
    {
        throw (OSErr)formatCannotRead; // Truncated or invalid data.
    }

    if (largeFileLength)
    {
        uncompressedSize = ((data[index] << 24) | (data[index + 1] << 16) | (data[index + 2] << 8) | data[index + 3]);
    }
    else
    {
        uncompressedSize = ((data[index] << 16) | (data[index + 1] << 8) | data[index + 2]);
    }
}

QFSHeader::QFSHeader(const HANDLE hFile, const LONG offset)
{
    OSErr error = SetFilePosition(hFile, FILE_BEGIN, offset);
    if (error != noErr)
    {
        throw error;
    }

    BYTE signature[2];

    error = ReadBytes(hFile, signature, 2);
    if (error != noErr)
    {
        throw error;
    }

    int signatureOffset;

    if ((signature[0] & QFS_HEADER_FLAG_MASK) == 0x10 && signature[1] == 0xFB)
    {
        signatureOffset = 0;
    }
    else
    {
        error = SetFilePosition(hFile, FILE_CURRENT, 4);
        if (error != noErr)
        {
            throw error;
        }

        error = ReadBytes(hFile, signature, 2);
        if (error != noErr)
        {
            throw error;
        }

        if ((signature[0] & QFS_HEADER_FLAG_MASK) == 0x10 && signature[1] == 0xFB)
        {
            signatureOffset = 4;
        }
        else
        {
            throw (OSErr)formatCannotRead; // Unknown compression format
        }
    }


    int uncompressedSizeOffset = signatureOffset + 2;

    // The first byte of the QFS header contains the compression flags.
    // See http://wiki.niotso.org/RefPack#Header

    // Sim City 4 introduced support for 32-bit file sizes.

    const bool largeFileLength = (signature[0] & QFS_HEADER_FLAG_LARGE_FILES) != 0;

    const int sizeFieldByteCount = largeFileLength ? 4 : 3;

    if ((signature[0] & QFS_HEADER_FLAG_COMPRESSED_SIZE_PRESENT) != 0)
    {
        // Some files may prefix the compressed file size after the signature.
        uncompressedSizeOffset += sizeFieldByteCount;
    }

    error = SetFilePosition(hFile, FILE_BEGIN, offset + static_cast<LONG>(uncompressedSizeOffset));
    if (error != noErr)
    {
        throw error;
    }

    BYTE sizeBuffer[4] = { 0, 0, 0, 0 };

    error = ReadBytes(hFile, sizeBuffer, sizeFieldByteCount);
    if (error != noErr)
    {
        throw error;
    }

    if (largeFileLength)
    {
        dataStartOffset = uncompressedSizeOffset + 4;
        uncompressedSize = ((sizeBuffer[0] << 24) | (sizeBuffer[1] << 16) | (sizeBuffer[2] << 8) | sizeBuffer[3]);
    }
    else
    {
        dataStartOffset = uncompressedSizeOffset + 3;
        uncompressedSize = ((sizeBuffer[0] << 16) | (sizeBuffer[1] << 8) | sizeBuffer[2]);
    }
}

bool QFSHeader::CheckSignature(const BYTE (&data)[2])
{
    return ((data[0] & QFS_HEADER_FLAG_MASK) == 0x10 && data[1] == 0xFB);
}

int QFSHeader::GetDataStartOffset() const
{
    return dataStartOffset;
}

int QFSHeader::GetUncompressedSize() const
{
    return uncompressedSize;
}