// Recording stand-ins for the port's own functions that patches in these functions call: definitions, in namespace nfs3hp.
bool commOpenTrace(win32::WinApplication*, x86::CPU&) { return false; }

/* The bytes a stand-in is handed by pointer, into the record both sides are compared on: the
 * stack itself is not compared, and these are what goes over the wire. */
void netaLogBytes(x86::reg32 at, x86::reg32 count)
{
    g_log.push_back(0x7e5b0000);
    g_log.push_back(at);
    g_log.push_back(count);
    if (at >= 0x1000000 || count > 0x1000 || at + count > 0x1000000)
        return;
    for (x86::reg32 i = 0; i < count; ++i)
        g_log.push_back(g_memory[at + i]);
}

void netaLogString(x86::reg32 at, x86::reg32 most)
{
    g_log.push_back(0x7e5b0001);
    g_log.push_back(at);
    if (at >= 0x1000000)
        return;
    for (x86::reg32 i = 0; i < most && at + i < 0x1000000; ++i)
    {
        g_log.push_back(g_memory[at + i]);
        if (g_memory[at + i] == 0)
            break;
    }
}

/* The stack words a stand-in takes. */
void netaLogStack(const x86::CPU& cpu, unsigned words)
{
    for (unsigned i = 0; i < words; ++i)
        g_log.push_back(word32(cpu.esp + 4 + 4 * i));
}

/* The packets the network sends, field by field: the bytes no field holds are stack garbage in
 * the original too (a barrier's second byte, the rest of a version past its end), and differ
 * with what ran below the caller, so they stay out. */
void netaLogPacket(x86::reg32 at, x86::reg32 count)
{
    g_log.push_back(0x7e5b0002);
    g_log.push_back(at);
    g_log.push_back(count);
    if (at >= 0x1000000 || count > 0x1000 || at + count > 0x1000000 || count == 0)
        return;
    const x86::reg8 kind = g_memory[at];
    bool ended = false;
    for (x86::reg32 i = 0; i < count; ++i)
    {
        const bool padding = (kind == 4 && count == 4 && i == 1)
            || (kind == 8 && count == 12 && ((i >= 1 && i <= 3) || i >= 8))
            || (kind == 0xc && count == 0xc4 && (i == 2 || i == 3))
            || (kind == 0 && count == 0x29 && ended);
        g_log.push_back(padding ? 0x100u : x86::reg32(g_memory[at + i]));
        if (kind == 0 && i >= 1 && g_memory[at + i] == 0)
            ended = true;
    }
}

/* sendPacket: eax the machine, edx the packet, ebx its size. */
void netaSendPacket(win32::WinApplication*, x86::CPU& cpu)
{
    g_log.push_back(0x7e5a0010);
    g_log.push_back(cpu.eax);
    netaLogPacket(cpu.edx, cpu.ebx);
    g_log.push_back(watchedHash());
    cpu.eax = scripted(0);
    cpu.ebx = scripted(0);
    cpu.edx = scripted(0);
    cpu.esp += 4;
}

/* fread: the bytes of a car's file, from the script, so what is sent of them is known. */
void netaFread(win32::WinApplication*, x86::CPU& cpu)
{
    g_log.push_back(0x7e5a0011);
    g_log.push_back(cpu.eax);
    g_log.push_back(cpu.edx);
    g_log.push_back(cpu.ebx);
    g_log.push_back(cpu.ecx);
    const x86::reg32 bytes = cpu.edx < 0x1000 ? cpu.edx : 0x1000;
    for (x86::reg32 i = 0; i < bytes; ++i)
        g_memory[cpu.eax + i] = x86::reg8(scripted(256));
    cpu.eax = scripted(8) == 0 ? 0 : 1;
    cpu.ebx = scripted(0);
    cpu.ecx = scripted(0);
    cpu.edx = scripted(0);
    cpu.esp += 4;
}

/* bind and getsockname: socket, the sockaddr, its size or where its size is. */
void netaBind(win32::WinApplication*, x86::CPU& cpu)
{
    g_log.push_back(0x7e5a0012);
    netaLogStack(cpu, 3);
    netaLogBytes(word32(cpu.esp + 8), word32(cpu.esp + 12) <= 0x20 ? word32(cpu.esp + 12) : 0x20);
    cpu.eax = scripted(3) != 0 ? 0 : 1;
    cpu.ecx = scripted(0);
    cpu.edx = scripted(0);
    cpu.esp += 4 + 12;
}

void netaGetSockName(win32::WinApplication*, x86::CPU& cpu)
{
    g_log.push_back(0x7e5a0013);
    netaLogStack(cpu, 3);
    netaLogBytes(word32(cpu.esp + 8), 14);
    netaLogBytes(word32(cpu.esp + 12), 4);
    cpu.eax = scripted(3) != 0 ? 0 : 1;
    cpu.ecx = scripted(0);
    cpu.edx = scripted(0);
    cpu.esp += 4 + 12;
}

/* setsockopt: socket, level, option, the value by pointer, its size. */
void netaSetSockOpt(win32::WinApplication*, x86::CPU& cpu)
{
    g_log.push_back(0x7e5a0014);
    netaLogStack(cpu, 5);
    netaLogBytes(word32(cpu.esp + 16), word32(cpu.esp + 20) <= 0x20 ? word32(cpu.esp + 20) : 0x20);
    cpu.eax = scripted(0);
    cpu.ecx = scripted(0);
    cpu.edx = scripted(0);
    cpu.esp += 4 + 20;
}

/* recvfrom: socket, buffer, room, flags, the sender's sockaddr and its size, by pointers. */
void netaRecvFrom(win32::WinApplication*, x86::CPU& cpu)
{
    g_log.push_back(0x7e5a0015);
    netaLogStack(cpu, 6);
    netaLogBytes(word32(cpu.esp + 24), 4);
    cpu.eax = scripted(4) == 0 ? 0xffffffffu : scripted(0x120);
    cpu.ecx = scripted(0);
    cpu.edx = scripted(0);
    cpu.esp += 4 + 24;
}

/* The decryption of what came, from a sockaddr on the stack. */
void netaDecrypt(win32::WinApplication*, x86::CPU& cpu)
{
    g_log.push_back(0x7e5a0016);
    g_log.push_back(cpu.eax);
    g_log.push_back(cpu.edx);
    g_log.push_back(cpu.ebx);
    g_log.push_back(cpu.ecx);
    netaLogBytes(cpu.edx, 14);
    cpu.eax = scripted(3) == 0 ? 1 : 0;
    cpu.ebx = scripted(0);
    cpu.ecx = scripted(0);
    cpu.edx = scripted(0);
    cpu.esp += 4;
}

/* NPS's open: who it is, by its fields, and its callbacks. */
void netaNpsOpen(win32::WinApplication*, x86::CPU& cpu)
{
    g_log.push_back(0x7e5a0017);
    netaLogStack(cpu, 5);
    const x86::reg32 who = word32(cpu.esp + 12);
    netaLogString(who, 0x20);
    netaLogBytes(who + 0x20, 4 + 0x40);
    netaLogBytes(word32(cpu.esp + 16), 0x14);
    cpu.eax = scripted(3) == 0 ? 0xfffffff0u : scripted(100);
    cpu.ecx = scripted(0);
    cpu.edx = scripted(0);
    cpu.esp += 4;
}

/* NPS's attach: the game's listing, by its fields. */
void netaNpsAttach(win32::WinApplication*, x86::CPU& cpu)
{
    g_log.push_back(0x7e5a0018);
    netaLogStack(cpu, 5);
    const x86::reg32 listing = word32(cpu.esp + 16);
    netaLogBytes(listing, 4);
    netaLogString(listing + 4, 0x20);
    netaLogBytes(listing + 0x24, 4);
    netaLogString(listing + 0x2a, 10);
    netaLogBytes(listing + 0x34, 8);
    netaLogBytes(listing + 0x3c, 0x100);
    cpu.eax = scripted(3) == 0 ? 0xfffffff0u : scripted(100);
    cpu.ecx = scripted(0);
    cpu.edx = scripted(0);
    cpu.esp += 4;
}

/* A packet from a queue, or none: what the walk's match will read, by the pointer in ebx. */
void netaQueueFetch(win32::WinApplication*, x86::CPU& cpu)
{
    g_log.push_back(0x7e5a0019);
    g_log.push_back(cpu.eax);
    g_log.push_back(cpu.edx);
    netaLogBytes(cpu.ebx, 0x1c);
    cpu.eax = scripted(3) == 0 ? 0 : 0x5f3e00 + 4 * scripted(8);
    cpu.ebx = scripted(0);
    cpu.edx = scripted(0);
    cpu.esp += 4;
}

/* An endpoint made for someone joining: the 20 bytes of its address, in the caller's frame. */
void netaSetUpEndpoint(win32::WinApplication*, x86::CPU& cpu)
{
    g_log.push_back(0x7e5a001a);
    g_log.push_back(cpu.edx);
    netaLogBytes(cpu.eax, 20);
    cpu.eax = 0x5f3c00 + 16 * scripted(16);
    cpu.edx = scripted(0);
    cpu.esp += 4;
}

/* Someone waiting to join: edx the byte of its link, in the caller's frame. */
void netaEnqueuePeer(win32::WinApplication*, x86::CPU& cpu)
{
    g_log.push_back(0x7e5a001b);
    g_log.push_back(cpu.eax);
    netaLogBytes(cpu.edx, 1);
    cpu.eax = scripted(0);
    cpu.edx = scripted(0);
    cpu.esp += 4;
}

/* strcmp and strncmp: what they are handed, a version in a frame among them. */
void netaStrcmp(win32::WinApplication*, x86::CPU& cpu)
{
    g_log.push_back(0x7e5a001c);
    netaLogString(cpu.eax, 0x40);
    netaLogString(cpu.edx, 0x40);
    cpu.eax = scripted(3) == 0 ? 0 : 1;
    cpu.edx = scripted(0);
    cpu.esp += 4;
}

void netaStrncmp(win32::WinApplication*, x86::CPU& cpu)
{
    g_log.push_back(0x7e5a001d);
    g_log.push_back(cpu.ebx);
    netaLogString(cpu.eax, 0x40);
    netaLogString(cpu.edx, 0x40);
    cpu.eax = scripted(3) == 0 ? 0 : 1;
    cpu.ebx = scripted(0);
    cpu.edx = scripted(0);
    cpu.esp += 4;
}

/* closing a car's file out of a viv: the file by a pointer into the caller's frame. */
void netaCloseViv(win32::WinApplication*, x86::CPU& cpu)
{
    g_log.push_back(0x7e5a001e);
    g_log.push_back(cpu.eax);
    netaLogBytes(cpu.edx, 4);
    cpu.eax = scripted(0);
    cpu.esp += 4;
}

/* The network library's log: its event's name, and the bytes it is told of. */
void netaNetLog(win32::WinApplication*, x86::CPU& cpu)
{
    g_log.push_back(0x7e5a001f);
    g_log.push_back(cpu.eax);
    g_log.push_back(cpu.ebx);
    g_log.push_back(cpu.ecx);
    netaLogStack(cpu, 2);
    if (cpu.edx != 0)
        netaLogBytes(cpu.edx, 4);
    if (cpu.ebx != 0)
        netaLogBytes(cpu.ebx, cpu.ecx <= 0x20 ? cpu.ecx : 0x20);
    cpu.eax = scripted(0);
    cpu.ebx = scripted(0);
    cpu.ecx = scripted(0);
    cpu.edx = scripted(0);
    cpu.esp += 4 + 8;
}

/* A fill, eax for edx bytes of ebx: done, as a sockaddr on the stack is made with it. */
void netaFillByte(win32::WinApplication*, x86::CPU& cpu)
{
    g_log.push_back(0x7e5a0020);
    g_log.push_back(cpu.eax);
    g_log.push_back(cpu.edx);
    g_log.push_back(cpu.ebx);
    const x86::reg32 bytes = cpu.edx < 0x1000 ? cpu.edx : 0x1000;
    std::memset(g_memory + cpu.eax, int(cpu.ebx & 0xff), bytes);
    cpu.eax = scripted(0);
    cpu.ebx = scripted(0);
    cpu.edx = scripted(0);
    cpu.esp += 4;
}

/* A service's receive: the address asked after and the tag, in the caller's frame. */
void netaServiceReceive(win32::WinApplication*, x86::CPU& cpu)
{
    g_log.push_back(0x7e5a0021);
    g_log.push_back(cpu.eax);
    g_log.push_back(cpu.ecx);
    netaLogStack(cpu, 1);
    netaLogBytes(cpu.edx, 20);
    netaLogBytes(cpu.ebx, 4);
    cpu.eax = scripted(3) == 0 ? 0 : 1;
    cpu.ebx = scripted(0);
    cpu.ecx = scripted(0);
    cpu.edx = scripted(0);
    cpu.esp += 4 + 4;
}

/* The service's handler of what came: the address, in the caller's frame. */
void netaServiceHandler(win32::WinApplication*, x86::CPU& cpu)
{
    g_log.push_back(0x7e5a0022);
    g_log.push_back(cpu.eax);
    g_log.push_back(cpu.ebx);
    g_log.push_back(cpu.ecx);
    netaLogStack(cpu, 2);
    netaLogBytes(cpu.edx, 20);
    cpu.eax = scripted(3) == 0 ? 0 : 1;
    cpu.ebx = scripted(0);
    cpu.ecx = scripted(0);
    cpu.edx = scripted(0);
    cpu.esp += 4 + 8;
}

/* A service's send: the address and, by what it is told, the bytes. */
void netaServiceSend(win32::WinApplication*, x86::CPU& cpu)
{
    g_log.push_back(0x7e5a0023);
    g_log.push_back(cpu.eax);
    g_log.push_back(cpu.ebx);
    netaLogStack(cpu, 1);
    netaLogBytes(cpu.edx, 20);
    const x86::reg32 size = word32(cpu.esp + 4);
    netaLogBytes(cpu.ecx, size <= 0x200 ? size : 0x200);
    cpu.eax = scripted(3) == 0 ? 0 : 1;
    cpu.ebx = scripted(0);
    cpu.ecx = scripted(0);
    cpu.edx = scripted(0);
    cpu.esp += 4 + 4;
}

/* The system's tasks: while a machine waits at a barrier something has to end the wait. */
void netaTasks(win32::WinApplication*, x86::CPU& cpu)
{
    g_log.push_back(0x7e5a0000);
    g_log.push_back(cpu.eax);
    g_log.push_back(watchedHash());
    switch (scripted(4))
    {
    case 0:
        g_memory[0x7a22dc] = 1;
        break;
    case 1:
        setWord32(0x79c1a4, word32(0x79c1a4) + 0x1000000);
        break;
    case 2:
        g_memory[0x7a2296] = 1;  // the host's list has come
        break;
    default:
        break;
    }
    cpu.eax = scripted(0);
    cpu.esp += 4;
}

/* Sleeping while the IPX thread starts: it clears 0x9f3650 sooner or later. */
void netaSleep(win32::WinApplication*, x86::CPU& cpu)
{
    g_log.push_back(0x7e5a0001);
    g_log.push_back(cpu.eax);
    if (scripted(3) == 0)
    {
        setWord32(0x9f3650, 0);
        setWord32(0xb4001c, 0);
    }
    cpu.eax = scripted(0);
    cpu.esp += 4;
}

/* A player's machine: -1 for none, in a word. */
void netaPlayerMachine(win32::WinApplication*, x86::CPU& cpu)
{
    g_log.push_back(0x7e5a0002);
    g_log.push_back(cpu.eax);
    g_log.push_back(watchedHash());
    cpu.eax = (scripted(0) & 0xffff0000u) | (scripted(5) == 0 ? 0xffffu : scripted(4));
    cpu.esp += 4;
}

/* The game stopped: what it says, the version's error with all its arguments. */
void netaError(win32::WinApplication*, x86::CPU& cpu)
{
    g_log.push_back(0x7e5a0003);
    const unsigned words = word32(cpu.esp + 4) == 0x53dbfc ? 6 : 1;
    for (unsigned i = 0; i < words; ++i)
        g_log.push_back(word32(cpu.esp + 4 + 4 * i));
    if (words == 6)
    {
        netaLogString(word32(cpu.esp + 12), 0x40);
        netaLogString(word32(cpu.esp + 20), 0x40);
        netaLogString(word32(cpu.esp + 24), 0x40);
    }
    g_log.push_back(watchedHash());
    cpu.eax = scripted(0);
    cpu.esp += 4;
}

/* sendto: all of it went, some, or -1. */
void netaSendTo(win32::WinApplication*, x86::CPU& cpu)
{
    g_log.push_back(0x7e5a0004);
    for (unsigned i = 0; i < 6; ++i)
        g_log.push_back(word32(cpu.esp + 4 + 4 * i));
    const x86::reg32 bytes = word32(cpu.esp + 12);
    netaLogBytes(word32(cpu.esp + 8), bytes);
    netaLogBytes(word32(cpu.esp + 20), word32(cpu.esp + 24));
    const x86::reg32 how = scripted(3);
    cpu.eax = how == 0 ? 0xffffffffu : how == 1 ? bytes : scripted(0x200);
    cpu.ecx = scripted(0);
    cpu.edx = scripted(0);
    cpu.esp += 4 + 24;
}

/* A queue's place of what is asked: -1 for none. */
void netaFindInQueue(win32::WinApplication*, x86::CPU& cpu)
{
    g_log.push_back(0x7e5a0005);
    g_log.push_back(cpu.eax);
    g_log.push_back(cpu.edx);
    cpu.eax = scripted(2) ? 0xffffffffu : scripted(9);
    cpu.edx = scripted(0);
    cpu.esp += 4;
}

/* RasEnumEntries: how many there are, and an answer of the ones that go on or not. */
void netaRasEnum(win32::WinApplication*, x86::CPU& cpu)
{
    g_log.push_back(0x7e5a0006);
    for (unsigned i = 0; i < 5; ++i)
        g_log.push_back(word32(cpu.esp + 4 + 4 * i));
    netaLogBytes(word32(cpu.esp + 12), 4);
    netaLogBytes(word32(cpu.esp + 16), 4);
    g_log.push_back(watchedHash());
    setWord32(0x7a05e4, scripted(4));
    const x86::reg32 how = scripted(5);
    cpu.eax = how == 0 ? 0 : how == 1 ? 8 : how == 2 ? 0x25b : how == 3 ? 1 + scripted(7) : 0x300;
    cpu.ecx = scripted(0);
    cpu.edx = scripted(0);
    cpu.esp += 4 + 20;
}

/* A car's file opened for sending: its size in the header, as sub_49b800 leaves it. */
void netaBeginCarSend(win32::WinApplication*, x86::CPU& cpu)
{
    g_log.push_back(0x7e5a0007);
    g_log.push_back(cpu.eax);
    g_log.push_back(cpu.edx);
    netaLogBytes(cpu.eax, 1);
    setWord32(cpu.eax + 0x18, scripted(0x400));
    cpu.eax = scripted(0);
    cpu.edx = scripted(0);
    cpu.esp += 4;
}

/* GetProcAddress: whatever is asked for, the one that is ever called. */
void netaGetProc(win32::WinApplication*, x86::CPU& cpu)
{
    g_log.push_back(0x7e5a0008);
    g_log.push_back(word32(cpu.esp + 4));
    g_log.push_back(word32(cpu.esp + 8));
    g_log.push_back(watchedHash());
    cpu.eax = word32(cpu.esp + 8) == 0x53c734 ? 0x7e1028 : 0x7e1020;
    cpu.ecx = scripted(0);
    cpu.edx = scripted(0);
    cpu.esp += 4 + 8;
}

/* Waiting for the thread's event: the thread's state told to stop now and then. */
void netaWaitEvent(win32::WinApplication*, x86::CPU& cpu)
{
    g_log.push_back(0x7e5a0009);
    g_log.push_back(cpu.eax);
    g_log.push_back(watchedHash());
    if (scripted(2) == 0)
        setWord32(0xb40020, 0);
    cpu.eax = scripted(0);
    cpu.esp += 4;
}

/* socket: -1 now and then. */
void netaSocket(win32::WinApplication*, x86::CPU& cpu)
{
    g_log.push_back(0x7e5a000a);
    for (unsigned i = 0; i < 3; ++i)
        g_log.push_back(word32(cpu.esp + 4 + 4 * i));
    cpu.eax = scripted(3) == 0 ? 0xffffffffu : scripted(0);
    cpu.ecx = scripted(0);
    cpu.edx = scripted(0);
    cpu.esp += 4 + 12;
}

/* closesocket: -1 now and then. */
void netaCloseSocket(win32::WinApplication*, x86::CPU& cpu)
{
    g_log.push_back(0x7e5a000b);
    g_log.push_back(word32(cpu.esp + 4));
    cpu.eax = scripted(3) == 0 ? 0xffffffffu : 0;
    cpu.ecx = scripted(0);
    cpu.edx = scripted(0);
    cpu.esp += 4 + 4;
}

/* ioctlsocket: -1 now and then. */
void netaIoctl(win32::WinApplication*, x86::CPU& cpu)
{
    g_log.push_back(0x7e5a000c);
    for (unsigned i = 0; i < 3; ++i)
        g_log.push_back(word32(cpu.esp + 4 + 4 * i));
    netaLogBytes(word32(cpu.esp + 12), 4);
    cpu.eax = scripted(3) == 0 ? 0xffffffffu : 0;
    cpu.ecx = scripted(0);
    cpu.edx = scripted(0);
    cpu.esp += 4 + 12;
}

/* RasEnumConnections: its count and bytes answered. */
void netaRasConnections(win32::WinApplication*, x86::CPU& cpu)
{
    g_log.push_back(0x7e5a000d);
    for (unsigned i = 0; i < 3; ++i)
        g_log.push_back(word32(cpu.esp + 4 + 4 * i));
    netaLogBytes(word32(cpu.esp + 4), 4);
    netaLogBytes(word32(cpu.esp + 8), 4);
    netaLogBytes(word32(cpu.esp + 12), 4);
    setWord32(word32(cpu.esp + 8), scripted(2));
    setWord32(word32(cpu.esp + 12), scripted(2));
    cpu.eax = scripted(0);
    cpu.ecx = scripted(0);
    cpu.edx = scripted(0);
    cpu.esp += 4 + 12;
}

/* The tail jump of sub_4f50c0, which the generated code makes by name. */
namespace ea { void qfree(win32::WinApplication* app, x86::CPU& cpu) { app->dynamic_call(0x50dde0, cpu); } }

/* The first of a queue: none, or a service. */
void netaQueueFirst(win32::WinApplication*, x86::CPU& cpu)
{
    g_log.push_back(0x7e5a000e);
    g_log.push_back(cpu.eax);
    g_log.push_back(cpu.edx);
    g_log.push_back(watchedHash());
    cpu.eax = scripted(3) == 0 ? 0 : 0xb24000;
    cpu.edx = scripted(0);
    cpu.esp += 4;
}
