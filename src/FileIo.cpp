#include "PITypes.h"
#include "FileIo.h"

OSErr GetFilePosition(HANDLE hFile, DWORD* filePos)
{
    DWORD moved = SetFilePointer(hFile, 0, nullptr, FILE_CURRENT);

    if (moved == INVALID_SET_FILE_POINTER)
    {
        return ioErr;
    }

    *filePos = moved;

    return noErr;
}

OSErr SetFilePosition(HANDLE hFile, const DWORD posMode, const LONG posOff)
{
    DWORD moved = SetFilePointer(hFile, posOff, nullptr, posMode);

    if (moved == INVALID_SET_FILE_POINTER)
    {
        return ioErr;
    }

    return noErr;
}

OSErr GetFileSize(HANDLE hFile, INT64* size)
{
    LARGE_INTEGER fileSize;

    if (!GetFileSizeEx(hFile, &fileSize))
    {
        return ioErr;
    }
    
    *size = fileSize.QuadPart;

    return noErr;
}

OSErr ReadBytes(HANDLE hFile, void* buffPtr, const DWORD count)
{
    DWORD bytesRead = 0;

    if (!ReadFile(hFile, buffPtr, count, &bytesRead, nullptr))
    {
        return ioErr;
    }

    if (bytesRead != count)
    {
        if (bytesRead == 0)
        {
            return eofErr;
        }
        
        return ioErr;
    }

    return noErr;
}

OSErr ReadInt32(HANDLE hFile, INT32* val)
{
    BYTE buf[4];

    OSErr e = ReadBytes(hFile, buf, 4);

    if (e == noErr)
    {
        *val = static_cast<INT32>(((buf[3] << 24) | (buf[2] << 16) | (buf[1] << 8)) | buf[0]);
    }
    
    return e;
}

OSErr ReadUInt16(HANDLE hFile, UINT16* val)
{
    BYTE buf[2];

    OSErr e = ReadBytes(hFile, buf, 2);

    if (e == noErr)
    {
        *val = static_cast<UINT16>((buf[1] << 8) | buf[0]);
    }

    return e;
}

OSErr WriteBytes(HANDLE hFile, const void* buffPtr, const DWORD count)
{
    DWORD bytesWritten = 0;
    
    if (!WriteFile(hFile, buffPtr, count, &bytesWritten, nullptr))
    {	
        return writErr;
    }

    if (bytesWritten != count)
    {
        return ioErr;
    }

    return noErr;
}

OSErr WriteInt32(HANDLE hFile, const INT32 val)
{
    BYTE buf[4];

    buf[0] = static_cast<BYTE>(val);
    buf[1] = static_cast<BYTE>(val >> 8);
    buf[2] = static_cast<BYTE>(val >> 16);
    buf[3] = static_cast<BYTE>(val >> 24);

    OSErr e = WriteBytes(hFile, buf, 4);

    return e;
}

OSErr WriteUInt16(HANDLE hFile, const UINT16 val)
{
    BYTE buf[2];
    
    buf[0] = static_cast<BYTE>(val);
    buf[1] = static_cast<BYTE>(val >> 8);

    OSErr e = WriteBytes(hFile, buf, 2);
    
    return e;
}
