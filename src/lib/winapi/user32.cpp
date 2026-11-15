#include <winapi/user32.h>
#include <x86.h>
#include <lib/winapp.h>
#include <cctype>
#include <SDL3/SDL.h>

namespace win32 { namespace user32
{

x86::reg32 CharUpperA(WinApplication* app, x86::CPU& cpu,
                      LPSTR lpsz)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    for (char* c = lpsz; *c; ++c)
        *c = char(toupper(*c));
    // The string's own address, as Windows gives it back: from the pointer, not the stack, so
    // that a C++ caller (guest::windows) gets it too.
    return app->guestAddress(lpsz);
}

DWORD CharUpperBuffA(WinApplication* app, x86::CPU& cpu,
                     LPSTR lpsz, DWORD cchLength)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    for (x86::reg32 i = 0; i < cchLength; ++i)
    {
        lpsz[i] = char(toupper(lpsz[i]));
    }
    return cchLength;
}

int MessageBoxA(WinApplication* app, x86::CPU& cpu,
                HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(hWnd);
    Uint32 type = 0;
    if (uType & 0x0001) type |= SDL_MESSAGEBOX_ERROR;
    if (uType & 0x0002) type |= SDL_MESSAGEBOX_WARNING;
    if (uType & 0x0000) type |= SDL_MESSAGEBOX_INFORMATION;
    SDL_ShowSimpleMessageBox(type, lpCaption, lpText, nullptr);
    return 0;
}

void wsprintfA(WinApplication* app, x86::CPU& cpu)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_ASSERT(false);
}

}}
