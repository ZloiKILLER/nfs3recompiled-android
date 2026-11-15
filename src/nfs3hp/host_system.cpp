#include "host_system.h"
#include <SDL3/SDL.h>
#include <cctype>
#include <cstring>
#include <lib/file.h>
#include <lib/library.h>
#include <lib/memmap.h>
#include <lib/process.h>
#include <lib/registry.h>
#include <lib/thread.h>
#include <lib/winapp.h>
#include <lib/window.h>
#include <new>
#include <string>

namespace nfs3hp
{
namespace host
{

using namespace win32;

struct OSVERSIONINFOA
{
    DWORD dwOSVersionInfoSize;
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
    DWORD dwBuildNumber;
    DWORD dwPlatformId;
    CHAR  szCSDVersion[128];
};

struct OSVERSIONINFOEXA
{
    OSVERSIONINFOA versionInfoA;
    WORD  wServicePackMajor;
    WORD  wServicePackMinor;
    WORD  wSuiteMask;
    BYTE  wProductType;
    BYTE  wReserved;
};

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

BOOL closeHandle(WinApplication* app, x86::CPU& cpu,
                 HANDLE hObject)
{
    NFS2_USE(cpu);
    if (WinApplication::traceApi())
    {
        GenericResource* resource = hObject ? app->getResource(hObject) : nullptr;
        SDL_Log("[API] CloseHandle h=%u resource=%p file=%d thread=%d",
                unsigned(hObject), static_cast<void*>(resource),
                dynamic_cast<File*>(resource) != nullptr,
                dynamic_cast<Thread*>(resource) != nullptr);
    }
    if (hObject)
        app->freeResource(hObject);
    return 1;
}

void debugBreak(WinApplication* app, x86::CPU& cpu)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_ASSERT(false);
}

void exitProcess(WinApplication* app, x86::CPU& cpu,
                 UINT uExitCode)
{
    // Said, with the game's code that asked for it (PostQuitMessage has the same).
    SDL_Log("[EXIT] exitProcess(%u) from 0x%x", unsigned(uExitCode),
            unsigned(app->getMemory<x86::reg32>(cpu.esp)));
    win32::Window::postMessage(0, 0x0012, 0, 0);
    cpu.terminate = true;
}

BOOL freeEnvironmentStrings(WinApplication* app, x86::CPU& cpu,
                            LPSTR lpszEnvironmentBlock)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(lpszEnvironmentBlock
    );
    return 1;
}

BOOL freeLibrary(WinApplication* app, x86::CPU& cpu,
                 HMODULE hLibModule)
{
    NFS2_USE(cpu);
    app->freeResource(hLibModule);
    return 1;
}

UINT ansiCodePage(WinApplication* app, x86::CPU& cpu)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_ASSERT(false);
    return 0;
}

BOOL codePageInfo(WinApplication* app, x86::CPU& cpu,
                  UINT CodePage, LPCPINFO lpCPInfo)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(CodePage);
    lpCPInfo->MaxCharSize = 1;
    lpCPInfo->DefaultChar[0] = '?';
    lpCPInfo->DefaultChar[1] = 0;
    lpCPInfo->LeadByte[0] = 0;
    lpCPInfo->LeadByte[1] = 0;
    lpCPInfo->LeadByte[2] = 0;
    lpCPInfo->LeadByte[3] = 0;
    lpCPInfo->LeadByte[4] = 0;
    lpCPInfo->LeadByte[5] = 0;
    lpCPInfo->LeadByte[6] = 0;
    lpCPInfo->LeadByte[7] = 0;
    lpCPInfo->LeadByte[8] = 0;
    lpCPInfo->LeadByte[9] = 0;
    lpCPInfo->LeadByte[10] = 0;
    lpCPInfo->LeadByte[11] = 0;
    return 1;
}

x86::reg32 commandLine(WinApplication* app, x86::CPU& cpu)
{
    NFS2_USE(cpu);
    return app->getAppName();
}

x86::reg32 commandLineWide(WinApplication* app, x86::CPU& cpu)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    return 0;
}

BOOL consoleMode(WinApplication* app, x86::CPU& cpu,
                 HANDLE hConsoleHandle, LPDWORD lpMode)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(hConsoleHandle);
    *lpMode = 0;
    return 1;
}

HANDLE currentProcess(WinApplication* app, x86::CPU& cpu)
{
    NFS2_USE(cpu);
    return app->allocateResource(new win32::Process());
}

DWORD currentProcessId(WinApplication* app, x86::CPU& cpu)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_ASSERT(false);
    return 0;
}

x86::reg32 environmentStrings(WinApplication* app, x86::CPU& cpu)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    return app->getEnv();
}

DWORD lastError(WinApplication* app, x86::CPU& cpu)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    //NFS2_ASSERT(false);
    return 0;
}

DWORD moduleFileName(WinApplication* app, x86::CPU& cpu,
                     HMODULE hModule, LPSTR lpFilename, DWORD nSize)
{
    NFS2_USE(cpu);
    NFS2_USE(nSize);
    *(lpFilename++) = 'C';
    *(lpFilename++) = ':';
    *(lpFilename++) = '\\';
    if (!hModule)
    {
        const char* exeFilename = app->getAppNameRaw();
        DWORD result = 0;
        while (*exeFilename)
        {
            *(lpFilename++) = *(exeFilename++);
            result++;
        }
        *lpFilename = 0;
        return result;
    }
    else
    {
        NFS2_ASSERT(false);
        return 0;
    }
}

DWORD moduleFileNameWide(WinApplication* app, x86::CPU& cpu,
                         HMODULE hModule, LPWSTR lpFilename, DWORD nSize)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(hModule);
    NFS2_USE(lpFilename);
    NFS2_USE(nSize);
    NFS2_ASSERT(false);
    return 0;
}

HMODULE moduleHandle(WinApplication* app, x86::CPU& cpu,
                     LPCSTR lpModuleName)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    if (lpModuleName)
    {
        NFS2_ASSERT(false);
        return 0xffffffff;
    }
    else
    {
        return 1;
    }
}

UINT oemCodePage(WinApplication* app, x86::CPU& cpu)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    return 850;
}

FARPROC procAddress(WinApplication* app, x86::CPU& cpu,
                    HMODULE hModule, LPCSTR lpProcName)
{
    NFS2_USE(cpu);
    /* NULL for a module or a name the port does not have, as Windows answers;
     * every caller in the game checks for it. */
    win32::LibraryHandle* libraryHandle = dynamic_cast<win32::LibraryHandle*>(app->getResource(hModule));
    win32::Library* l = libraryHandle ? libraryHandle->getLibrary() : nullptr;
    if (!l)
        return 0;
    win32::Library::Symbol s = (*l)[lpProcName];
    if (!s.first)
        SDL_Log("GetProcAddress: no %s in the port", lpProcName);
    return s.first;
}

HANDLE stdHandle(WinApplication* app, x86::CPU& cpu,
                 DWORD nStdHandle)
{
    NFS2_USE(cpu);
    switch(nStdHandle)
    {
    case x86::reg32(-10):
        {
            // stdin
            win32::File* h = new win32::File(0, "stdin");
            return app->allocateResource(h);
        }
    case x86::reg32(-11):
        {
            // stdout
            win32::File* h = new win32::File(1, "stdout");
            return app->allocateResource(h);
        }
    case x86::reg32(-12):
        {
            // stderr
            win32::File* h = new win32::File(2, "stderr");
            return app->allocateResource(h);
        }
    default:
        NFS2_ASSERT(false);
        return HANDLE(-1);
    }
}

void systemInfo(WinApplication* app, x86::CPU& cpu,
                LPSYSTEM_INFO lpSystemInfo)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    lpSystemInfo->wProcessorArchitecture = 0;
    lpSystemInfo->wReserved = 0;
    lpSystemInfo->dwPageSize = 4096;
    lpSystemInfo->lpMinimumApplicationAddress = 0x2000;
    lpSystemInfo->lpMinimumApplicationAddress = 0xffffffff;
    lpSystemInfo->dwActiveProcessorMask = 1;
    lpSystemInfo->dwNumberOfProcessors = 1;
    lpSystemInfo->dwProcessorType = 2;
    lpSystemInfo->dwAllocationGranularity = 4096;
    lpSystemInfo->wProcessorLevel = 5;
    lpSystemInfo->wProcessorRevision = 0xff50;
}

DWORD windowsVersion(WinApplication* app, x86::CPU& cpu)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    return 0x80000304;
}

BOOL windowsVersionEx(WinApplication* app, x86::CPU& cpu,
                      LPOSVERSIONINFOA lpVersionInformation)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    lpVersionInformation->dwMajorVersion = 5;
    lpVersionInformation->dwMinorVersion = 1;
    lpVersionInformation->dwBuildNumber = 2600;
    lpVersionInformation->dwPlatformId = 2;
    strcpy(lpVersionInformation->szCSDVersion, "Service Pack 3");
    if (lpVersionInformation->dwOSVersionInfoSize == sizeof(OSVERSIONINFOEXA))
    {
        OSVERSIONINFOEXA* info = reinterpret_cast<OSVERSIONINFOEXA*>(lpVersionInformation);
        info->wProductType = 1;
        info->wServicePackMajor = 3;
        info->wServicePackMinor = 0;
    }
    return 1;
}

void memoryStatus(WinApplication* app, x86::CPU& cpu,
                  LPMEMORYSTATUS lpBuffer)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    lpBuffer->dwLength = sizeof(MEMORYSTATUS);
    lpBuffer->dwMemoryLoad = 0;
    lpBuffer->dwTotalPhys = 268435456;
    lpBuffer->dwAvailPhys = 268435456;
    lpBuffer->dwTotalPageFile = 268435456;
    lpBuffer->dwAvailPageFile = 268435456;
    lpBuffer->dwTotalVirtual = 0x7fffffff;
    lpBuffer->dwAvailVirtual = 0x7fffffff;
}

HMODULE loadLibrary(WinApplication* app, x86::CPU& cpu,
                    LPCSTR lpLibFileName)
{
    NFS2_USE(cpu);
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Loading library %s", lpLibFileName);
    win32::LibraryHandle* lib = new win32::LibraryHandle(lpLibFileName);
    if (*lib)
    {
        //lib->getLibrary()->registerSymbols(app);
        return app->allocateResource(lib);
    }
    else
    {
        /* NULL, as Windows answers a library it cannot load, and as every
         * caller in the game checks: the TCP/IP screen asks for rasapi32.dll
         * to show the dial-up address and falls back to the host name. */
        delete lib;
        return 0;
    }
}

int multiByteToWide(WinApplication* app, x86::CPU& cpu,
                    UINT CodePage, DWORD dwFlags, LPCCH lpMultiByteStr,
                    int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(CodePage);
    NFS2_USE(dwFlags);
    NFS2_USE(cchWideChar);
    int result = 0;
    if (lpWideCharStr)
        *lpWideCharStr++ = 0xfffe;
    for (int i = 0; i < cbMultiByte; ++i)
    {
        result++;
        if (lpWideCharStr)
            *lpWideCharStr++ = *lpMultiByteStr++;
        if (!*lpMultiByteStr)
            break;
    }
    return result;
}

void debugString(WinApplication* app, x86::CPU& cpu,
                 LPCSTR lpOutputString)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    SDL_Log("%s", lpOutputString);
}

BOOL peekConsoleInput(WinApplication* app, x86::CPU& cpu,
                      HANDLE hConsoleInput, PINPUT_RECORD lpBuffer,
                      DWORD nLength, LPDWORD lpNumberOfEventsRead)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(hConsoleInput);
    NFS2_USE(lpBuffer);
    NFS2_USE(nLength);
    NFS2_USE(lpNumberOfEventsRead);
    //NFS2_ASSERT(false);
    *lpNumberOfEventsRead = 0;
    return 1;
}

BOOL readConsoleInput(WinApplication* app, x86::CPU& cpu,
                      HANDLE hConsoleInput, PINPUT_RECORD lpBuffer, DWORD nLength,
                      LPDWORD lpNumberOfEventsRead)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(hConsoleInput);
    NFS2_USE(lpBuffer);
    NFS2_USE(nLength);
    //NFS2_ASSERT(false);
    *lpNumberOfEventsRead = 0;
    return 1;
}

void unwind(WinApplication* app, x86::CPU& cpu,
            PVOID TargetFrame, PVOID TargetIp, PEXCEPTION_RECORD ExceptionRecord, PVOID ReturnValue)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(TargetFrame);
    NFS2_USE(TargetIp);
    NFS2_USE(ExceptionRecord);
    NFS2_USE(ReturnValue);
    NFS2_ASSERT(false);
}

BOOL setConsoleCtrlHandler(WinApplication* app, x86::CPU& cpu,
                           PHANDLER_ROUTINE HandlerRoutine, BOOL Add)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(HandlerRoutine);
    NFS2_USE(Add);
    NFS2_ASSERT(false);
    return 0;
}

BOOL setConsoleMode(WinApplication* app, x86::CPU& cpu,
                    HANDLE hConsoleHandle, DWORD  dwMode)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(hConsoleHandle);
    NFS2_USE(dwMode);
    //NFS2_ASSERT(false);
    return 1;
}

BOOL setEnvironmentVariable(WinApplication* app, x86::CPU& cpu,
                            LPCSTR lpName, LPCSTR lpValue)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(lpName);
    NFS2_USE(lpValue);
    // TODO?
    return 0;
}

BOOL setEnvironmentVariableWide(WinApplication* app, x86::CPU& cpu,
                                LPCWSTR lpName, LPCWSTR lpValue)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(lpName);
    NFS2_USE(lpValue);
    NFS2_ASSERT(false);
    return 0;
}

UINT setErrorMode(WinApplication* app, x86::CPU& cpu, UINT uMode)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(uMode);
    return 0;
}

void setLastError(WinApplication* app, x86::CPU& cpu,
                  DWORD dwErrCode)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(dwErrCode);
}

BOOL setStdHandle(WinApplication* app, x86::CPU& cpu,
                  DWORD nStdHandle, HANDLE hHandle)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(nStdHandle);
    NFS2_USE(hHandle);
    NFS2_ASSERT(false);
    return 0;
}

LPTOP_LEVEL_EXCEPTION_FILTER setUnhandledExceptionFilter(WinApplication* app, x86::CPU& cpu,
                                                         LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(lpTopLevelExceptionFilter);
    return 0;
}

LONG unhandledExceptionFilter(WinApplication* app, x86::CPU& cpu,
                              struct _EXCEPTION_POINTERS *ExceptionInfo)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(ExceptionInfo);
    NFS2_ASSERT(false);
    return 0;
}

x86::reg32 virtualAlloc(WinApplication* app, x86::CPU& cpu,
                        LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect)
{
    NFS2_USE(cpu);
    NFS2_USE(flAllocationType);
    NFS2_USE(flProtect);
    NFS2_ASSERT(lpAddress == nullptr);
    MemMap* map = nullptr;
    try { map = new MemMap(dwSize); }
    catch (const std::bad_alloc&)
    {
        SDL_Log("[API] VirtualAlloc failed: size=%u", unsigned(dwSize));
        return 0;
    }
    void* data = &app->getMemory<void>(map->getBlockStart());
    memset(data, 0, dwSize);
    if (WinApplication::traceApi())
    {
        SDL_Log("[API] VirtualAlloc size=%u -> 0x%x..0x%x", unsigned(dwSize),
                unsigned(map->getBlockStart()),
                unsigned(map->getBlockStart() + map->getBlockSize()));
    }
    return map->getBlockStart();
}

BOOL virtualFree(WinApplication* app, x86::CPU& cpu,
                 Packed<void> lpAddress, SIZE_T dwSize, DWORD dwFreeType)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(dwSize);
    NFS2_ASSERT(dwFreeType == 0x8000);
    MemMap* memBlock = MemMap::findBlock(lpAddress);
    if (WinApplication::traceApi())
    {
        SDL_Log("[API] VirtualFree addr=0x%x -> block 0x%x..0x%x", unsigned(x86::reg32(lpAddress)),
                memBlock ? unsigned(memBlock->getBlockStart()) : 0u,
                memBlock ? unsigned(memBlock->getBlockStart() + memBlock->getBlockSize()) : 0u);
    }
    delete memBlock;
    return 1;
}

BOOL virtualProtect(WinApplication* app, x86::CPU& cpu,
                    Packed<void> lpAddress, SIZE_T dwSize, DWORD flNewProtect, DWORD* lpflOldProtect)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(lpAddress);
    NFS2_USE(dwSize);
    NFS2_USE(flNewProtect);
    NFS2_USE(lpflOldProtect);
    // TODO?
    return 1;
}

SIZE_T virtualQuery(WinApplication* app, x86::CPU& cpu,
                    Packed<const void> lpAddress, PMEMORY_BASIC_INFORMATION lpBuffer, SIZE_T dwLength)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    MemMap* memBlock = MemMap::findBlock(lpAddress);
    NFS2_ASSERT(dwLength == sizeof(MEMORY_BASIC_INFORMATION));
    lpBuffer->BaseAddress = memBlock->getBlockStart();
    lpBuffer->AllocationBase = memBlock->getBlockStart();
    lpBuffer->AllocationProtect = 0x04; //READWRITE
    lpBuffer->RegionSize = memBlock->getBlockSize();
    lpBuffer->State = 0x1000; //COMMIT
    lpBuffer->Protect = 0x04; //READWRITE
    lpBuffer->Type = 0x20000; //PRIVATE
    return sizeof(*lpBuffer);
}

int wideToMultiByte(WinApplication* app, x86::CPU& cpu,
                    UINT CodePage, DWORD dwFlags, LPCWCH lpWideCharStr, int cchWideChar,
                    LPSTR lpMultiByteStr, int cbMultiByte, LPCCH lpDefaultChar, LPBOOL lpUsedDefaultChar)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(CodePage);
    NFS2_USE(dwFlags);
    NFS2_USE(lpWideCharStr);
    NFS2_USE(cchWideChar);
    NFS2_USE(lpMultiByteStr);
    NFS2_USE(cbMultiByte);
    NFS2_USE(lpDefaultChar);
    NFS2_USE(lpUsedDefaultChar);
    NFS2_ASSERT(false);
    return 0;
}

BOOL writeConsole(WinApplication* app, x86::CPU& cpu,
                  HANDLE hConsoleOutput, const void* lpBuffer, DWORD nNumberOfCharsToWrite,
                  LPDWORD lpNumberOfCharsWritten, LPVOID  lpReserved)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(hConsoleOutput);
    NFS2_USE(lpBuffer);
    NFS2_USE(nNumberOfCharsToWrite);
    NFS2_USE(lpNumberOfCharsWritten);
    NFS2_USE(lpReserved);
    NFS2_ASSERT(false);
    return 0;
}

LSTATUS registryClose(WinApplication* app, x86::CPU& cpu,
                      HKEY hKey)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(hKey);
    return 0;
}

LSTATUS registryFlush(WinApplication* app, x86::CPU& cpu,
                      HKEY hKey)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(hKey);
    return 1;
}

LSTATUS registryOpen(WinApplication* app, x86::CPU& cpu,
                     HKEY hKey, LPCSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult)
{
    NFS2_USE(cpu);
    NFS2_USE(ulOptions);
    NFS2_USE(samDesired);
    RegistryKey* key = dynamic_cast<RegistryKey*>(app->getResource(hKey));
    if (key)
    {
        const char* subk = lpSubKey;
        while (*subk)
        {
            const char* subkEnd;
            for (subkEnd = subk; *subkEnd && *subkEnd != '\\'; ++subkEnd)
                /* nothing */;
            std::string subkey = std::string(subk, subkEnd);
            key = key->getChild(subkey);
            if (!key)
                return 1;
            subk = subkEnd;
            if (*subk) subk++;
        }
        *phkResult = key->getResourceIndex();
        return 0;
    }
    else
    {
        return 1;
    }
}

LSTATUS registryQuery(WinApplication* app, x86::CPU& cpu,
                      HKEY hKey, LPCSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData)
{
    NFS2_USE(cpu);
    NFS2_USE(lpReserved);
    RegistryKey* key = dynamic_cast<RegistryKey*>(app->getResource(hKey));
    if (key)
    {
        RegistryValue* value = key->getValue(lpValueName);
        if (!value)
        {
            return 1;
        }
        else
        {
            return value->query(lpType, lpData, lpcbData);
        }
    }
    else
    {
        return 1;
    }
}

void setFileVersion(const char* value)
{
    fileVersion() = value ? value : "";
    if (fileVersion().size() >= kStringRoom)
        fileVersion().resize(kStringRoom - 1);
}

BOOL fileVersionInfo(WinApplication* app, x86::CPU& cpu, LPCSTR lptstrFilename,
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

DWORD fileVersionInfoSize(WinApplication* app, x86::CPU& cpu,
                          LPCSTR lptstrFilename, LPDWORD lpdwHandle)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(lptstrFilename);
    *lpdwHandle = 0;
    return kStringOffset + kStringRoom;
}

BOOL versionQueryValue(WinApplication* app, x86::CPU& cpu,
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

x86::reg32 charUpper(WinApplication* app, x86::CPU& cpu,
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

DWORD charUpperBuffer(WinApplication* app, x86::CPU& cpu,
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

int messageBox(WinApplication* app, x86::CPU& cpu,
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

void formatString(WinApplication* app, x86::CPU& cpu)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_ASSERT(false);
}

HRESULT createGuid(WinApplication* app, x86::CPU& cpu, GUID *pguid)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(pguid);
    NFS2_ASSERT(false);
    return -1;
}

HINSTANCE shellExecute(WinApplication* app, x86::CPU& cpu, HWND hwnd, LPCSTR lpOperation,
                       LPCSTR lpFile, LPCSTR lpParameters, LPCSTR lpDirectory, int nShowCmd)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(hwnd);
    NFS2_USE(lpOperation);
    NFS2_USE(lpFile);
    NFS2_USE(lpParameters);
    NFS2_USE(lpDirectory);
    NFS2_USE(nShowCmd);
    NFS2_ASSERT(false);
    return -1;
}

}  // namespace host
}  // namespace nfs3hp
