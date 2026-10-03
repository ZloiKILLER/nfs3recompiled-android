#include <winapi/version.h>
#include <x86.h>
#include <SDL3/SDL.h>
#include <cstring>
#include <string>

namespace win32 { namespace version
{

/* The block GetFileVersionInfoA fills: the marker the game was always given,
 * then the file version string VerQueryValueA hands back for
 * \StringFileInfo\...\FileVersion.  The executable's resources are not in the
 * guest's memory, so the game names its version (setFileVersion).
 *
 * NFS III puts that string in 0x55e898 at start-up, sends it to every machine
 * joining a network race, and a joining machine waits until it has a version
 * that is not empty before comparing it with its own ("Game Versions differ").
 * With an empty one -- the 4 bytes 00 00 00 02 this used to give -- a phone
 * joining another stayed on Connecting for ever. */
namespace
{
constexpr DWORD kStringOffset = 4;
constexpr DWORD kStringRoom = 60;

std::string& fileVersion()
{
    static std::string value;
    return value;
}
}

void setFileVersion(const char* value)
{
    fileVersion() = value ? value : "";
    if (fileVersion().size() >= kStringRoom)
        fileVersion().resize(kStringRoom - 1);
}

BOOL GetFileVersionInfoA(WinApplication* app, x86::CPU& cpu, LPCSTR lptstrFilename,
                         DWORD dwHandle, DWORD dwLen, LPVOID lpData)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(lptstrFilename);
    NFS2_USE(dwHandle);
    *reinterpret_cast<x86::reg32*>(lpData) = 0x02000000;
    if (dwLen >= kStringOffset + kStringRoom)
    {
        char* text = reinterpret_cast<char*>(lpData) + kStringOffset;
        std::memset(text, 0, kStringRoom);
        std::memcpy(text, fileVersion().c_str(), fileVersion().size());
    }
    return 1;
}

DWORD GetFileVersionInfoSizeA(WinApplication* app, x86::CPU& cpu,
                              LPCSTR lptstrFilename, LPDWORD lpdwHandle)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(lptstrFilename);
    *lpdwHandle = 0;
    return kStringOffset + kStringRoom;
}

BOOL VerQueryValueA(WinApplication* app, x86::CPU& cpu,
                    Packed<void> pBlock, LPCSTR lpSubBlock, Packed<void>* lplpBuffer, UINT* puLen)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    static const char kFileVersion[] = "\\FileVersion";
    const size_t length = lpSubBlock ? std::strlen(lpSubBlock) : 0;
    const size_t suffix = sizeof(kFileVersion) - 1;
    if (!fileVersion().empty() && length >= suffix
        && SDL_strcasecmp(lpSubBlock + length - suffix, kFileVersion) == 0)
    {
        *lplpBuffer = Packed<void>(x86::reg32(pBlock) + kStringOffset);
        if (puLen)
            *puLen = UINT(fileVersion().size() + 1);
        return 1;
    }
    *lplpBuffer = pBlock;
    return 1;
}
}}
