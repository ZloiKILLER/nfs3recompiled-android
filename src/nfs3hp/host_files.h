#ifndef NFS3HP_HOST_FILES_H
#define NFS3HP_HOST_FILES_H

#include <x86.h>
#include <lib/winapp.h>
#include <winapi/types.h>

/* Files, directories and drives as the game asks for them (plan step 6, round 2): the
 * port's host code on src/lib's File, in the place of Windows' kernel32.  Each takes the words
 * the game's code pushes for the import it stands for, in its order (a pointer one as a pointer
 * into the game's memory), so a call site hands them as they are (guest::windows<kSlot,
 * &host::fn>, native_windows.h), and gives back what the game reads in eax: the same values the
 * port's Windows gave.  A file's handle is the file in the application's resource table. */
namespace nfs3hp
{
namespace host
{

typedef int HFILE;

win32::LONG compareFileTime(win32::WinApplication* app, x86::CPU& cpu,
                            const win32::FILETIME *lpFileTime1, const win32::FILETIME *lpFileTime2);
win32::BOOL createDirectory(win32::WinApplication* app, x86::CPU& cpu,
                            win32::LPCSTR lpPathName, win32::LPSECURITY_ATTRIBUTES lpSecurityAttributes);
win32::HANDLE createFile(win32::WinApplication* app, x86::CPU& cpu,
                         win32::LPCSTR lpFileName, win32::DWORD dwDesiredAccess, win32::DWORD dwShareMode,
                         win32::LPSECURITY_ATTRIBUTES lpSecurityAttributes, win32::DWORD dwCreationDisposition,
                         win32::DWORD dwFlagsAndAttributes, win32::HANDLE hTemplateFile);
win32::HANDLE createFileMapping(win32::WinApplication* app, x86::CPU& cpu,
                                win32::HANDLE hFile, win32::LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
                                win32::DWORD flProtect, win32::DWORD dwMaximumSizeHigh, win32::DWORD dwMaximumSizeLow,
                                win32::LPCSTR lpName);
win32::BOOL deleteFile(win32::WinApplication* app, x86::CPU& cpu,
                       win32::LPCSTR lpFileName);
win32::BOOL findClose(win32::WinApplication* app, x86::CPU& cpu,
                      win32::HANDLE hFindFile);
win32::HANDLE findFirstFile(win32::WinApplication* app, x86::CPU& cpu,
                            win32::LPCSTR lpFileName, win32::LPWIN32_FIND_DATAA lpFindFileData);
win32::BOOL findNextFile(win32::WinApplication* app, x86::CPU& cpu,
                         win32::HANDLE hFindFile, win32::LPWIN32_FIND_DATAA lpFindFileData);
win32::BOOL flushFile(win32::WinApplication* app, x86::CPU& cpu,
                      win32::HANDLE hFile);
win32::DWORD currentDirectory(win32::WinApplication* app, x86::CPU& cpu,
                              win32::DWORD nBufferLength, win32::LPSTR lpBuffer);
win32::BOOL diskFreeSpace(win32::WinApplication* app, x86::CPU& cpu,
                          win32::LPCSTR lpRootPathName, win32::LPDWORD lpSectorsPerCluster, win32::LPDWORD lpBytesPerSector,
                          win32::LPDWORD lpNumberOfFreeClusters, win32::LPDWORD lpTotalNumberOfClusters);
win32::UINT driveType(win32::WinApplication* app, x86::CPU& cpu,
                      win32::LPCSTR lpRootPathName);
win32::DWORD fileAttributes(win32::WinApplication* app, x86::CPU& cpu,
                            win32::LPCSTR lpFileName);
win32::DWORD fileSize(win32::WinApplication* app, x86::CPU& cpu,
                      win32::HANDLE hFile, win32::LPDWORD lpFileSizeHigh);
win32::BOOL fileTime(win32::WinApplication* app, x86::CPU& cpu,
                     win32::HANDLE hFile, win32::LPFILETIME lpCreationTime, win32::LPFILETIME lpLastAccessTime, win32::LPFILETIME lpLastWriteTime);
win32::DWORD fileType(win32::WinApplication* app, x86::CPU& cpu,
                      win32::HANDLE hFile);
win32::DWORD fullPathName(win32::WinApplication* app, x86::CPU& cpu,
                          win32::LPCSTR lpFileName, win32::DWORD nBufferLength, win32::LPSTR lpBuffer, x86::reg32* lpFilePart);
win32::DWORD logicalDriveStrings(win32::WinApplication* app, x86::CPU& cpu,
                                 win32::DWORD nBufferLength, win32::LPSTR lpBuffer);
win32::DWORD logicalDrives(win32::WinApplication* app, x86::CPU& cpu);
win32::BOOL volumeInformation(win32::WinApplication* app, x86::CPU& cpu,
                              win32::LPCSTR lpRootPathName, win32::LPSTR lpVolumeNameBuffer, win32::DWORD nVolumeNameSize,
                              win32::LPDWORD lpVolumeSerialNumber, win32::LPDWORD lpMaximumComponentLength,
                              win32::LPDWORD lpFileSystemFlags, win32::LPSTR lpFileSystemNameBuffer,
                              win32::DWORD nFileSystemNameSize);
win32::UINT windowsDirectory(win32::WinApplication* app, x86::CPU& cpu,
                             win32::LPSTR lpBuffer, win32::UINT uSize);
x86::reg32 mapViewOfFile(win32::WinApplication* app, x86::CPU& cpu,
                         win32::HANDLE hFileMappingObject, win32::DWORD dwDesiredAccess, win32::DWORD dwFileOffsetHigh,
                         win32::DWORD dwFileOffsetLow, win32::SIZE_T dwNumberOfBytesToMap);
win32::BOOL moveFile(win32::WinApplication* app, x86::CPU& cpu,
                     win32::LPCSTR lpExistingFileName, win32::LPCSTR lpNewFileName);
win32::BOOL readFile(win32::WinApplication* app, x86::CPU& cpu,
                     win32::HANDLE hFile, win32::LPVOID lpBuffer, win32::DWORD nNumberOfBytesToRead,
                     win32::LPDWORD lpNumberOfBytesRead, win32::LPOVERLAPPED lpOverlapped);
win32::BOOL removeDirectory(win32::WinApplication* app, x86::CPU& cpu, win32::LPCSTR lpPathName);
win32::BOOL setCurrentDirectory(win32::WinApplication* app, x86::CPU& cpu,
                                win32::LPCSTR lpPathName);
win32::BOOL setEndOfFile(win32::WinApplication* app, x86::CPU& cpu,
                         win32::HANDLE hFile);
win32::BOOL setFileAttributes(win32::WinApplication* app, x86::CPU& cpu,
                              win32::LPCSTR lpFileName, win32::DWORD  dwFileAttributes);
win32::DWORD setFilePointer(win32::WinApplication* app, x86::CPU& cpu,
                            win32::HANDLE hFile, win32::LONG lDistanceToMove, win32::PLONG lpDistanceToMoveHigh, win32::DWORD dwMoveMethod);
win32::BOOL unmapViewOfFile(win32::WinApplication* app, x86::CPU& cpu,
                            win32::Packed<const void> lpBaseAddress);
win32::BOOL writeFile(win32::WinApplication* app, x86::CPU& cpu,
                      win32::HANDLE hFile, win32::LPCVOID lpBuffer, win32::DWORD nNumberOfBytesToWrite,
                      win32::LPDWORD lpNumberOfBytesWritten, win32::LPOVERLAPPED lpOverlapped);
HFILE closeFileByName(win32::WinApplication* app, x86::CPU& cpu, HFILE hFile);
HFILE openFileByName(win32::WinApplication* app, x86::CPU& cpu, win32::LPCSTR lpPathName, int iReadWrite);

}  // namespace host
}  // namespace nfs3hp

#endif
