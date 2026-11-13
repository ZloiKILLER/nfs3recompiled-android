// Recording stand-ins for the port's own functions that patches in these functions call: definitions, in namespace nfs3hp.
void mock(win32::WinApplication*, x86::CPU&, x86::reg32);

/* The port's trace of what is sent: off, as in a release (the generated code's hook is taken out). */
bool commSendTrace(win32::WinApplication*, x86::CPU&) { return false; }

/* The bytes a stand-in is handed by pointer, into the record both sides are compared on: what is
 * built on a frame is below the returned esp, where the memory is not compared.  An address that
 * cannot be one (a number where the call's other form has a pointer) is left at the address. */
void netcLogBytes(x86::reg32 at, x86::reg32 count)
{
    g_log.push_back(0x7e5c0000);
    g_log.push_back(at);
    g_log.push_back(count);
    if (at < 0x1000 || at >= 0x1000000)
        return;
    for (x86::reg32 i = 0; i < count && i < 0x400 && at + i < 0x1000000; ++i)
        g_log.push_back(g_memory[at + i]);
}

void netcLogString(x86::reg32 at)
{
    g_log.push_back(0x7e5c0001);
    g_log.push_back(at);
    if (at < 0x1000 || at >= 0x1000000)
        return;
    for (x86::reg32 i = 0; i < 0x100 && at + i < 0x1000000 && g_memory[at + i] != 0; ++i)
        g_log.push_back(g_memory[at + i]);
}

/* What the cases of netc have in guest memory, in what its watched range holds. */
constexpr x86::reg32 kNetcConn = 0xc00000, kNetcConnInfo = 0xc00400, kNetcGame = 0xc01000, kNetcTable = 0xc02000;
constexpr x86::reg32 kNetcOps = 0xc02100, kNetcPlayer = 0xc02200, kNetcOut = 0xc02300, kNetcInfo = 0xc02400;
constexpr x86::reg32 kNetcName = 0xc02500, kNetcLink = 0xc03000, kNetcPlain = 0xc03100, kNetcPlainBuffer = 0xc03200;
constexpr x86::reg32 kNetcRely = 0xc04000, kNetcRelyBuffer = 0xc04100, kNetcPackets = 0xc05000, kNetcPorts = 0xc08000;
constexpr x86::reg32 kNetcFrames = 0xc0d000, kNetcPool = 0xc10000;
// Functions that are not there, for what the game calls through a pointer.
constexpr x86::reg32 kNetcFnSend = 0x7e0300, kNetcFnBool = 0x7e0310, kNetcFnCallback = 0x7e0320, kNetcFnRoom = 0x7e0330;
constexpr x86::reg32 kNetcFnRead = 0x7e0340, kNetcFnWrite = 0x7e0350, kNetcFnPlain = 0x7e0360, kNetcFnAddress = 0x7e0370;
