#include "host_files.h"
#include "host_system.h"
#include <SDL3/SDL.h>
#include <cstring>
#include <lib/file.h>
#include <lib/library.h>
#include <lib/memmap.h>
#include <lib/process.h>
#include <lib/thread.h>
#include <lib/window.h>
#include <new>

namespace nfs3hp
{
namespace host
{

using namespace win32;

LONG compareFileTime(WinApplication* app, x86::CPU& cpu,
                     const FILETIME *lpFileTime1, const FILETIME *lpFileTime2)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    if (lpFileTime1->dwHighDateTime < lpFileTime2->dwHighDateTime)
    {
        return -1;
    }
    else if (lpFileTime1->dwHighDateTime == lpFileTime2->dwHighDateTime)
    {
        if (lpFileTime1->dwLowDateTime < lpFileTime2->dwLowDateTime)
        {
            return -1;
        }
        else if (lpFileTime1->dwLowDateTime == lpFileTime2->dwLowDateTime)
        {
            return 0;
        }
        else
        {
            return 1;
        }
    }
    else
    {
        return 1;
    }
}

/* The game calls this once to make its save directory.  It used to be a stub
 * that asserted and reported failure, so the directory was never created and
 * every save written into it afterwards failed to open -- which is what a
 * tournament that cannot be loaded looks like from the outside. */
BOOL createDirectory(WinApplication* app, x86::CPU& cpu,
                     LPCSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    /* Security attributes have no meaning here: the port owns the whole data
     * directory and everything in it is created with the same permissions. */
    NFS2_USE(lpSecurityAttributes);
    if (!lpPathName)
        return 0;
    return File::createDirectory(lpPathName) ? 1 : 0;
}

HANDLE createFile(WinApplication* app, x86::CPU& cpu,
                  LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
                  LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition,
                  DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
    NFS2_USE(cpu);
    NFS2_USE(dwShareMode);
    NFS2_USE(lpSecurityAttributes);
    NFS2_USE(hTemplateFile);
    win32::File* file = new win32::File(lpFileName, dwDesiredAccess, dwFlagsAndAttributes, dwCreationDisposition);
    if (*file)
    {
        const HANDLE h = app->allocateResource(file);
        if (WinApplication::traceApi())
        {
            SDL_Log("[API] CreateFileA '%s' name@0x%x [%02x %02x %02x %02x] access=0x%x -> handle=%u",
                    lpFileName, unsigned(app->guestAddress(lpFileName)),
                    unsigned(x86::reg8(lpFileName[0])),
                    unsigned(x86::reg8(lpFileName[0] ? lpFileName[1] : 0)),
                    unsigned(x86::reg8(lpFileName[0] && lpFileName[1] ? lpFileName[2] : 0)),
                    unsigned(x86::reg8(lpFileName[0] && lpFileName[1] && lpFileName[2] ? lpFileName[3] : 0)),
                    unsigned(dwDesiredAccess), unsigned(h));
        }
        return h;
    }
    else
    {
        if (WinApplication::traceApi())
        {
            SDL_Log("[API] CreateFileA '%s' name@0x%x access=0x%x -> FAILED", lpFileName,
                    unsigned(app->guestAddress(lpFileName)), unsigned(dwDesiredAccess));
        }
        delete file;
        return HANDLE(-1);
    }
}

HANDLE createFileMapping(WinApplication* app, x86::CPU& cpu,
                         HANDLE hFile, LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
                         DWORD flProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow,
                         LPCSTR lpName)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(lpFileMappingAttributes);
    NFS2_USE(dwMaximumSizeHigh);
    NFS2_USE(dwMaximumSizeLow);
    NFS2_USE(lpName);
    NFS2_ASSERT(flProtect & 0x2);
    if (WinApplication::traceApi())
    {
        SDL_Log("[API] CreateFileMappingA hFile=%u flProtect=0x%x", unsigned(hFile), unsigned(flProtect));
    }
    File* file = new File(*dynamic_cast<File*>(app->getResource(hFile)));
    return app->allocateResource(file);
}

BOOL deleteFile(WinApplication* app, x86::CPU& cpu,
                LPCSTR lpFileName)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    return File::remove(lpFileName);
}

BOOL findClose(WinApplication* app, x86::CPU& cpu,
               HANDLE hFindFile)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    app->freeResource(hFindFile);
    return 1;
}

HANDLE findFirstFile(WinApplication* app, x86::CPU& cpu,
                     LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData)
{
    NFS2_USE(cpu);
    win32::FileEnumerator* enumerator = new win32::FileEnumerator(lpFileName);
    if (enumerator->findNext(lpFindFileData))
    {
        return app->allocateResource(enumerator);
    }
    else
    {
        delete enumerator;
        return -1;
    }
}

BOOL findNextFile(WinApplication* app, x86::CPU& cpu,
                  HANDLE hFindFile, LPWIN32_FIND_DATAA lpFindFileData)
{
    NFS2_USE(cpu);
    FileEnumerator* enumerator = dynamic_cast<FileEnumerator*>(app->getResource(hFindFile));
    return enumerator->findNext(lpFindFileData);
}

BOOL flushFile(WinApplication* app, x86::CPU& cpu,
               HANDLE hFile)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(hFile);
    NFS2_ASSERT(false);
    return 0;
}

DWORD currentDirectory(WinApplication* app, x86::CPU& cpu,
                       DWORD nBufferLength, LPSTR lpBuffer)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(nBufferLength);
    NFS2_ASSERT(lpBuffer);
    strcpy(lpBuffer, File::getCurrentDirectory().c_str());
    return static_cast<DWORD>(File::getCurrentDirectory().length());
}

BOOL diskFreeSpace(WinApplication* app, x86::CPU& cpu,
                   LPCSTR lpRootPathName, LPDWORD lpSectorsPerCluster, LPDWORD lpBytesPerSector,
                   LPDWORD lpNumberOfFreeClusters, LPDWORD lpTotalNumberOfClusters)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(lpRootPathName);
    if (lpSectorsPerCluster) *lpSectorsPerCluster = 1;
    if (lpBytesPerSector) *lpBytesPerSector = 512;
    if (lpNumberOfFreeClusters) *lpNumberOfFreeClusters = 512*1024;
    if (lpTotalNumberOfClusters) *lpTotalNumberOfClusters = 1024*1024;
    if (WinApplication::traceApi())
    {
        SDL_Log("[API] GetDiskFreeSpaceA root='%s' root@0x%x spc=%u bps=%u free=%u total=%u",
                lpRootPathName ? lpRootPathName : "(null)",
                unsigned(app->guestAddress(lpRootPathName)),
                lpSectorsPerCluster ? unsigned(*lpSectorsPerCluster) : 0,
                lpBytesPerSector ? unsigned(*lpBytesPerSector) : 0,
                lpNumberOfFreeClusters ? unsigned(*lpNumberOfFreeClusters) : 0,
                lpTotalNumberOfClusters ? unsigned(*lpTotalNumberOfClusters) : 0);
    }
    return 1;
}

UINT driveType(WinApplication* app, x86::CPU& cpu,
               LPCSTR lpRootPathName)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    switch(*lpRootPathName)
    {
    case 'C':
        return 3; //DISK
    case 'D':
        return 5; //CDROM
    default:
        return 1; //INVALID
    }
}

DWORD fileAttributes(WinApplication* app, x86::CPU& cpu,
                     LPCSTR lpFileName)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    File f(lpFileName, GENERIC_READ, 0, OPEN_EXISTING);
    if (f)
    {
        return 0x1;
    }
    else
    {
        return 0xffffffff;
    }
}

BOOL fileTime(WinApplication* app, x86::CPU& cpu,
              HANDLE hFile, LPFILETIME lpCreationTime, LPFILETIME lpLastAccessTime, LPFILETIME lpLastWriteTime)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(hFile);
    if (lpCreationTime)
    {
        lpCreationTime->dwLowDateTime = 0;
        lpCreationTime->dwHighDateTime = 0;
    }
    if (lpLastAccessTime)
    {
        lpLastAccessTime->dwLowDateTime = 0;
        lpLastAccessTime->dwHighDateTime = 0;
    }
    if (lpLastWriteTime)
    {
        lpLastWriteTime->dwLowDateTime = 0;
        lpLastWriteTime->dwHighDateTime = 0;
    }
    return 0;
}

DWORD fileSize(WinApplication* app, x86::CPU& cpu,
               HANDLE hFile, LPDWORD lpFileSizeHigh)
{
    NFS2_USE(cpu);
    win32::File* file = dynamic_cast<win32::File*>(app->getResource(hFile));
    if (lpFileSizeHigh)
        *lpFileSizeHigh = 0;
    const DWORD sz = file ? file->size() : DWORD(-1);
    if (WinApplication::traceApi())
    {
        SDL_Log("[API] GetFileSize hFile=%u file=%p size=%u", unsigned(hFile),
                static_cast<void*>(file), unsigned(sz));
    }
    return sz;
}

DWORD fileType(WinApplication* app, x86::CPU& cpu,
               HANDLE hFile)
{
    NFS2_USE(cpu);
    win32::File* f = dynamic_cast<win32::File*>(app->getResource(hFile));
    return f ? 1 : 0;
}

DWORD fullPathName(WinApplication* app, x86::CPU& cpu,
                   LPCSTR lpFileName, DWORD nBufferLength, LPSTR lpBuffer, x86::reg32* lpFilePart)
{
    NFS2_USE(nBufferLength);
    // The buffer's address in the game's memory, from the pointer (not the stack: a C++ caller,
    // guest::windows, pushes none).
    NFS2_USE(cpu);
    const x86::reg32 lpBufferAddress = app->guestAddress(lpBuffer);
    strcpy(lpBuffer, "C:\\nfs2se\\");
    *lpFilePart = lpBufferAddress + static_cast<x86::reg32>(strlen(lpBuffer));
    strcat(lpBuffer, lpFileName);
    return static_cast<DWORD>(strlen(lpBuffer));
}

DWORD logicalDriveStrings(WinApplication* app, x86::CPU& cpu,
                          DWORD nBufferLength, LPSTR lpBuffer)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    if (nBufferLength)
    {
        lpBuffer[0] = 'C';
        lpBuffer[1] = 'D';
        lpBuffer[2] = 0;
    }
    return 2;
}

DWORD logicalDrives(WinApplication* app, x86::CPU& cpu)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    return 0x2 | 0x4;
}

BOOL volumeInformation(WinApplication* app, x86::CPU& cpu,
                       LPCSTR lpRootPathName, LPSTR lpVolumeNameBuffer, DWORD nVolumeNameSize,
                       LPDWORD lpVolumeSerialNumber, LPDWORD lpMaximumComponentLength,
                       LPDWORD lpFileSystemFlags, LPSTR lpFileSystemNameBuffer,
                       DWORD nFileSystemNameSize)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(lpRootPathName);
    if (lpVolumeNameBuffer && nVolumeNameSize)
    {
        strcpy(lpVolumeNameBuffer, "DISK");
    }
    if (lpVolumeSerialNumber)
        *lpVolumeSerialNumber = 0x12345678;
    if (lpMaximumComponentLength)
        *lpMaximumComponentLength = 255;
    if (lpFileSystemFlags)
        *lpFileSystemFlags = 1|2|4;
    if (lpFileSystemNameBuffer && nFileSystemNameSize)
        strcpy(lpFileSystemNameBuffer, "FAT");
    return 1;
}

UINT windowsDirectory(WinApplication* app, x86::CPU& cpu,
                      LPSTR lpBuffer, UINT uSize)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(lpBuffer);
    NFS2_USE(uSize);
    NFS2_ASSERT(false);
    return 0;
}

x86::reg32 mapViewOfFile(WinApplication* app, x86::CPU& cpu,
                         HANDLE hFileMappingObject, DWORD dwDesiredAccess, DWORD dwFileOffsetHigh,
                         DWORD dwFileOffsetLow, SIZE_T dwNumberOfBytesToMap)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(dwDesiredAccess);
    NFS2_ASSERT(dwFileOffsetHigh == 0);
    app->unlockContext(cpu);
    File* file = dynamic_cast<File*>(app->getResource(hFileMappingObject));
    MemMap* result = nullptr;
    try { result = new MemMap(dwNumberOfBytesToMap); }
    catch (const std::bad_alloc&) { app->lockContext(cpu); return 0; }
    void* data = &app->getMemory<void>(result->getBlockStart());
    file->seek(0, x86::sreg32(dwFileOffsetLow));
    x86::reg32 bytesRead = 0;
    file->read(data, dwNumberOfBytesToMap, &bytesRead);
    NFS2_ASSERT(bytesRead == dwNumberOfBytesToMap);
    app->lockContext(cpu);
    const x86::reg32 address = result->getBlockStart();
    if (WinApplication::traceApi())
    {
        x86::reg32 first = 0;
        if (bytesRead >= sizeof(first))
            memcpy(&first, data, sizeof(first));
        SDL_Log("[API] MapViewOfFile h=%u offset=%u requested=%u read=%u -> addr=0x%x first4=0x%08x",
                unsigned(hFileMappingObject), unsigned(dwFileOffsetLow),
                unsigned(dwNumberOfBytesToMap), unsigned(bytesRead), unsigned(address),
                unsigned(first));
    }
    return address;
}

BOOL moveFile(WinApplication* app, x86::CPU& cpu,
              LPCSTR lpExistingFileName, LPCSTR lpNewFileName)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(lpExistingFileName);
    NFS2_USE(lpNewFileName);
    NFS2_ASSERT(false);
    return 0;
}

BOOL readFile(WinApplication* app, x86::CPU& cpu,
              HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead,
              LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped)
{
    NFS2_USE(cpu);
    NFS2_USE(lpOverlapped);
    win32::File* f = dynamic_cast<win32::File*>(app->getResource(hFile));
    if (!f)
    {
        if (WinApplication::traceApi())
        {
            SDL_Log("[API] ReadFile h=%u requested=%u overlapped=%p -> INVALID_HANDLE",
                    unsigned(hFile), unsigned(nNumberOfBytesToRead),
                    static_cast<void*>(lpOverlapped));
        }
        return 0;
    }
    else
    {
        app->unlockContext(cpu);
        BOOL result = f->read(lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead);
        app->lockContext(cpu);
        if (WinApplication::traceApi())
        {
            const DWORD bytesRead = lpNumberOfBytesRead ? *lpNumberOfBytesRead : 0;
            DWORD first = 0;
            if (lpBuffer && bytesRead >= sizeof(first))
                memcpy(&first, lpBuffer, sizeof(first));
            SDL_Log("[API] ReadFile h=%u requested=%u overlapped=%p -> ok=%u read=%u first4=0x%08x",
                    unsigned(hFile), unsigned(nNumberOfBytesToRead),
                    static_cast<void*>(lpOverlapped), unsigned(result), unsigned(bytesRead),
                    unsigned(first));
        }
        return result;
    }
}

BOOL removeDirectory(WinApplication* app, x86::CPU& cpu, LPCSTR lpPathName)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(lpPathName);
    NFS2_ASSERT(false);
    return 0;
}

BOOL setCurrentDirectory(WinApplication* app, x86::CPU& cpu,
                         LPCSTR lpPathName)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    /* Worth a line: the value is remembered but relative paths do not resolve
     * against it, so anything the game reaches by setting a directory and then
     * using a bare name looks in the data root instead. */
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "SetCurrentDirectory %s",
                lpPathName ? lpPathName : "(null)");
    File::setCurrentDirectory(lpPathName);
    return 1;
}

BOOL setEndOfFile(WinApplication* app, x86::CPU& cpu,
                  HANDLE hFile)
{
    NFS2_USE(cpu);
    File* file = dynamic_cast<File*>(app->getResource(hFile));
    return file->truncate();
}

BOOL setFileAttributes(WinApplication* app, x86::CPU& cpu,
                       LPCSTR lpFileName, DWORD  dwFileAttributes)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(lpFileName);
    NFS2_USE(dwFileAttributes);
    //NFS2_ASSERT(false);
    return 0;
}

DWORD setFilePointer(WinApplication* app, x86::CPU& cpu,
                     HANDLE hFile, LONG lDistanceToMove, PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod)
{
    NFS2_USE(cpu);
    win32::File* file= dynamic_cast<win32::File*>(app->getResource(hFile));
    NFS2_ASSERT(lpDistanceToMoveHigh == 0);
    const DWORD result = file ? file->seek(dwMoveMethod, lDistanceToMove) : DWORD(-1);
    if (WinApplication::traceApi())
    {
        SDL_Log("[API] SetFilePointer h=%u method=%u distance=%d -> %u",
                unsigned(hFile), unsigned(dwMoveMethod), int(lDistanceToMove), unsigned(result));
    }
    return result;
}

BOOL unmapViewOfFile(WinApplication* app, x86::CPU& cpu,
                     Packed<const void> lpBaseAddress)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    if (lpBaseAddress)
    {
        MemMap* memBlock = MemMap::findBlock(lpBaseAddress);
        if (WinApplication::traceApi())
        {
            SDL_Log("[API] UnmapViewOfFile addr=0x%x block=%p",
                    unsigned(x86::reg32(lpBaseAddress)), static_cast<void*>(memBlock));
        }
        delete memBlock;
        return 1;
    }
    else
    {
        if (WinApplication::traceApi())
            SDL_Log("[API] UnmapViewOfFile addr=0 -> FALSE");
        return 0;
    }
}

BOOL writeFile(WinApplication* app, x86::CPU& cpu,
               HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite,
               LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped)
{
    NFS2_USE(cpu);
    NFS2_ASSERT(lpOverlapped == nullptr);
    win32::File* f = dynamic_cast<win32::File*>(app->getResource(hFile));
    if (!f)
    {
        return 0;
    }
    else
    {
        app->unlockContext(cpu);
        x86::reg32 result = f->write(lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten);
        app->lockContext(cpu);
        return result;
    }
}

HFILE closeFileByName(WinApplication* app, x86::CPU& cpu, HFILE hFile)
{
    NFS2_USE(cpu);
    app->freeResource(hFile);
    return hFile;
}

HFILE openFileByName(WinApplication* app, x86::CPU& cpu, LPCSTR lpPathName, int iReadWrite)
{
    NFS2_USE(cpu);
    NFS2_ASSERT(iReadWrite == 0);
    win32::File* file = new win32::File(lpPathName, 0x80000000, 0, 3);
    return app->allocateResource(file);
}

}  // namespace host
}  // namespace nfs3hp
