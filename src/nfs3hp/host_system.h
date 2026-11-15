#ifndef NFS3HP_HOST_SYSTEM_H
#define NFS3HP_HOST_SYSTEM_H

#include <x86.h>
#include <lib/winapp.h>
#include <winapi/types.h>

/* The process, its memory, modules, environment, console, code pages and errors, the
 * registry, the version resource, and what was left of user32, ole32 and shell32, as the game
 * asks for them (plan step 6, round 2): the port's host code on src/lib, in the place of
 * Windows' kernel32, advapi32, version, user32, ole32 and shell32.  Each takes the words the
 * game's code pushes for the import it stands for, in its order, as host_files.h's do, and
 * gives back what the game reads in eax: the same values the port's Windows gave. */
namespace nfs3hp
{
namespace host
{

// Types of the imports' own that src/lib's Windows kept beside them.
struct EXCEPTION_RECORD;
typedef EXCEPTION_RECORD* PEXCEPTION_RECORD;
struct OSVERSIONINFOA;
typedef OSVERSIONINFOA* LPOSVERSIONINFOA;
typedef int HFILE;
typedef x86::reg32 HKEY;

win32::BOOL closeHandle(win32::WinApplication* app, x86::CPU& cpu,
                        win32::HANDLE hObject);
void debugBreak(win32::WinApplication* app, x86::CPU& cpu);
void exitProcess(win32::WinApplication* app, x86::CPU& cpu,
                 win32::UINT uExitCode);
win32::BOOL freeEnvironmentStrings(win32::WinApplication* app, x86::CPU& cpu,
                                   win32::LPSTR lpszEnvironmentBlock);
win32::BOOL freeLibrary(win32::WinApplication* app, x86::CPU& cpu,
                        win32::HMODULE hLibModule);
win32::UINT ansiCodePage(win32::WinApplication* app, x86::CPU& cpu);
win32::BOOL codePageInfo(win32::WinApplication* app, x86::CPU& cpu,
                         win32::UINT CodePage, win32::LPCPINFO lpCPInfo);
x86::reg32 commandLine(win32::WinApplication* app, x86::CPU& cpu);
x86::reg32 commandLineWide(win32::WinApplication* app, x86::CPU& cpu);
win32::BOOL consoleMode(win32::WinApplication* app, x86::CPU& cpu,
                        win32::HANDLE hConsoleHandle, win32::LPDWORD lpMode);
win32::HANDLE currentProcess(win32::WinApplication* app, x86::CPU& cpu);
win32::DWORD currentProcessId(win32::WinApplication* app, x86::CPU& cpu);
x86::reg32 environmentStrings(win32::WinApplication* app, x86::CPU& cpu);
win32::DWORD lastError(win32::WinApplication* app, x86::CPU& cpu);
win32::DWORD moduleFileName(win32::WinApplication* app, x86::CPU& cpu,
                            win32::HMODULE hModule, win32::LPSTR lpFilename, win32::DWORD nSize);
win32::DWORD moduleFileNameWide(win32::WinApplication* app, x86::CPU& cpu,
                                win32::HMODULE hModule, win32::LPWSTR lpFilename, win32::DWORD nSize);
win32::HMODULE moduleHandle(win32::WinApplication* app, x86::CPU& cpu,
                            win32::LPCSTR lpModuleName);
win32::UINT oemCodePage(win32::WinApplication* app, x86::CPU& cpu);
win32::FARPROC procAddress(win32::WinApplication* app, x86::CPU& cpu,
                           win32::HMODULE hModule, win32::LPCSTR lpProcName);
win32::HANDLE stdHandle(win32::WinApplication* app, x86::CPU& cpu,
                        win32::DWORD nStdHandle);
void systemInfo(win32::WinApplication* app, x86::CPU& cpu,
                win32::LPSYSTEM_INFO lpSystemInfo);
win32::DWORD windowsVersion(win32::WinApplication* app, x86::CPU& cpu);
win32::BOOL windowsVersionEx(win32::WinApplication* app, x86::CPU& cpu,
                             LPOSVERSIONINFOA lpVersionInformation);
void memoryStatus(win32::WinApplication* app, x86::CPU& cpu,
                  win32::LPMEMORYSTATUS lpBuffer);
win32::HMODULE loadLibrary(win32::WinApplication* app, x86::CPU& cpu,
                           win32::LPCSTR lpLibFileName);
int multiByteToWide(win32::WinApplication* app, x86::CPU& cpu,
                    win32::UINT CodePage, win32::DWORD dwFlags, win32::LPCCH lpMultiByteStr,
                    int cbMultiByte, win32::LPWSTR lpWideCharStr, int cchWideChar);
void debugString(win32::WinApplication* app, x86::CPU& cpu,
                 win32::LPCSTR lpOutputString);
win32::BOOL peekConsoleInput(win32::WinApplication* app, x86::CPU& cpu,
                             win32::HANDLE hConsoleInput, win32::PINPUT_RECORD lpBuffer,
                             win32::DWORD nLength, win32::LPDWORD lpNumberOfEventsRead);
win32::BOOL readConsoleInput(win32::WinApplication* app, x86::CPU& cpu,
                             win32::HANDLE hConsoleInput, win32::PINPUT_RECORD lpBuffer, win32::DWORD nLength,
                             win32::LPDWORD lpNumberOfEventsRead);
void unwind(win32::WinApplication* app, x86::CPU& cpu,
            win32::PVOID TargetFrame, win32::PVOID TargetIp, PEXCEPTION_RECORD ExceptionRecord, win32::PVOID ReturnValue);
win32::BOOL setConsoleCtrlHandler(win32::WinApplication* app, x86::CPU& cpu,
                                  win32::PHANDLER_ROUTINE HandlerRoutine, win32::BOOL Add);
win32::BOOL setConsoleMode(win32::WinApplication* app, x86::CPU& cpu,
                           win32::HANDLE hConsoleHandle, win32::DWORD  dwMode);
win32::BOOL setEnvironmentVariable(win32::WinApplication* app, x86::CPU& cpu,
                                   win32::LPCSTR lpName, win32::LPCSTR lpValue);
win32::BOOL setEnvironmentVariableWide(win32::WinApplication* app, x86::CPU& cpu,
                                       win32::LPCWSTR lpName, win32::LPCWSTR lpValue);
win32::UINT setErrorMode(win32::WinApplication* app, x86::CPU& cpu, win32::UINT uMode);
void setLastError(win32::WinApplication* app, x86::CPU& cpu,
                  win32::DWORD dwErrCode);
win32::BOOL setStdHandle(win32::WinApplication* app, x86::CPU& cpu,
                         win32::DWORD nStdHandle, win32::HANDLE hHandle);
win32::LPTOP_LEVEL_EXCEPTION_FILTER setUnhandledExceptionFilter(win32::WinApplication* app, x86::CPU& cpu,
                                                                win32::LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter);
win32::LONG unhandledExceptionFilter(win32::WinApplication* app, x86::CPU& cpu,
                                     struct _EXCEPTION_POINTERS *ExceptionInfo);
x86::reg32 virtualAlloc(win32::WinApplication* app, x86::CPU& cpu,
                        win32::LPVOID lpAddress, win32::SIZE_T dwSize, win32::DWORD flAllocationType, win32::DWORD flProtect);
win32::BOOL virtualFree(win32::WinApplication* app, x86::CPU& cpu,
                        win32::Packed<void> lpAddress, win32::SIZE_T dwSize, win32::DWORD dwFreeType);
win32::BOOL virtualProtect(win32::WinApplication* app, x86::CPU& cpu,
                           win32::Packed<void> lpAddress, win32::SIZE_T dwSize, win32::DWORD flNewProtect, win32::DWORD* lpflOldProtect);
win32::SIZE_T virtualQuery(win32::WinApplication* app, x86::CPU& cpu,
                           win32::Packed<const void> lpAddress, win32::PMEMORY_BASIC_INFORMATION lpBuffer, win32::SIZE_T dwLength);
int wideToMultiByte(win32::WinApplication* app, x86::CPU& cpu,
                    win32::UINT CodePage, win32::DWORD dwFlags, win32::LPCWCH lpWideCharStr, int cchWideChar,
                    win32::LPSTR lpMultiByteStr, int cbMultiByte, win32::LPCCH lpDefaultChar, win32::LPBOOL lpUsedDefaultChar);
win32::BOOL writeConsole(win32::WinApplication* app, x86::CPU& cpu,
                         win32::HANDLE hConsoleOutput, const void* lpBuffer, win32::DWORD nNumberOfCharsToWrite,
                         win32::LPDWORD lpNumberOfCharsWritten, win32::LPVOID  lpReserved);
win32::LSTATUS registryClose(win32::WinApplication* app, x86::CPU& cpu,
                             HKEY hKey);
win32::LSTATUS registryFlush(win32::WinApplication* app, x86::CPU& cpu,
                             HKEY hKey);
win32::LSTATUS registryOpen(win32::WinApplication* app, x86::CPU& cpu,
                            HKEY hKey, win32::LPCSTR lpSubKey, win32::DWORD ulOptions, win32::REGSAM samDesired, win32::PHKEY phkResult);
win32::LSTATUS registryQuery(win32::WinApplication* app, x86::CPU& cpu,
                             HKEY hKey, win32::LPCSTR lpValueName, win32::LPDWORD lpReserved, win32::LPDWORD lpType, win32::LPBYTE lpData, win32::LPDWORD lpcbData);
win32::BOOL fileVersionInfo(win32::WinApplication* app, x86::CPU& cpu, win32::LPCSTR lptstrFilename,
                            win32::DWORD dwHandle, win32::DWORD dwLen, win32::LPVOID lpData);
win32::DWORD fileVersionInfoSize(win32::WinApplication* app, x86::CPU& cpu,
                                 win32::LPCSTR lptstrFilename, win32::LPDWORD lpdwHandle);
win32::BOOL versionQueryValue(win32::WinApplication* app, x86::CPU& cpu,
                              win32::Packed<void> pBlock, win32::LPCSTR lpSubBlock, win32::Packed<void>* lplpBuffer, win32::UINT* puLen);
/* The FileVersion string of the game's executable, which VerQueryValueA
 * gives back: its resources are not in the guest's memory. */
void setFileVersion(const char* value);
x86::reg32 charUpper(win32::WinApplication* app, x86::CPU& cpu,
                     win32::LPSTR lpsz);
win32::DWORD charUpperBuffer(win32::WinApplication* app, x86::CPU& cpu,
                             win32::LPSTR lpsz, win32::DWORD cchLength);
int messageBox(win32::WinApplication* app, x86::CPU& cpu,
               win32::HWND hWnd, win32::LPCSTR lpText, win32::LPCSTR lpCaption, win32::UINT uType);
void formatString(win32::WinApplication* app, x86::CPU& cpu);
win32::HRESULT createGuid(win32::WinApplication* app, x86::CPU& cpu, win32::GUID *pguid);
win32::HINSTANCE shellExecute(win32::WinApplication* app, x86::CPU& cpu, win32::HWND hwnd, win32::LPCSTR lpOperation,
                              win32::LPCSTR lpFile, win32::LPCSTR lpParameters, win32::LPCSTR lpDirectory, int nShowCmd);

}  // namespace host
}  // namespace nfs3hp

#endif
