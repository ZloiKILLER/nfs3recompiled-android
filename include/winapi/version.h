#ifndef version_H_
#define version_H_

#include <x86.h>
#include <lib/winapp.h>
#include <winapi/types.h>

namespace win32 { namespace version
{

BOOL GetFileVersionInfoA(WinApplication* app, x86::CPU& cpu, LPCSTR lptstrFilename,
                         DWORD dwHandle, DWORD dwLen, LPVOID lpData);
DWORD GetFileVersionInfoSizeA(WinApplication* app, x86::CPU& cpu,
                              LPCSTR lptstrFilename, LPDWORD lpdwHandle);
BOOL VerQueryValueA(WinApplication* app, x86::CPU& cpu,
                    Packed<void> pBlock, LPCSTR lpSubBlock, Packed<void>* lplpBuffer, UINT* puLen);

/* The FileVersion string of the game's executable, which VerQueryValueA
 * gives back: its resources are not in the guest's memory. */
void setFileVersion(const char* value);

}}

#endif
