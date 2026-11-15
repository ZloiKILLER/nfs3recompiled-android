#ifndef user32_H_
#define user32_H_

#include <x86.h>
#include <winapi/types.h>


namespace win32 { class WinApplication; }

/* What is left of user32 for round 2 of plan step 6.  The window, its messages, the cursor and
 * the keys are the port's own host code (src/nfs3hp/host_window.h). */
namespace win32 { namespace user32
{

x86::reg32 CharUpperA(WinApplication* app, x86::CPU& cpu,
                      LPSTR lpsz);
DWORD CharUpperBuffA(WinApplication* app, x86::CPU& cpu,
                     LPSTR lpsz, DWORD cchLength);
int MessageBoxA(WinApplication* app, x86::CPU& cpu,
                HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType);
void wsprintfA(WinApplication* app, x86::CPU& cpu);

}}

#endif
