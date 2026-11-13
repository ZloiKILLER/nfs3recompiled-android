// Recording stand-ins for the port's own functions that patches in these functions call: definitions, in namespace nfs3hp.
void mock(win32::WinApplication*, x86::CPU&, x86::reg32);

/* The port's log of the network's lines (native_vertices.cpp): off, as it is unless asked. */
bool commEventTrace(win32::WinApplication*, x86::CPU&) { return false; }

/* The bytes a mocked function is handed by address, into the record both sides are compared
 * on: memory below esp is not compared, so a packet built in a frame shows only here. */
void netbRecord(x86::reg32 at, x86::reg32 bytes)
{
    g_log.push_back(0x7e5c0000);
    g_log.push_back(at);
    g_log.push_back(bytes);
    if (at == 0)
        return;
    for (x86::reg32 i = 0; i < bytes && i < 0x2000 && at + i < kMemory; ++i)
        g_log.push_back(g_memory[at + i]);
}

/* The same for a stack word that holds the address and one that holds the length. */
void netbRecordStack(const x86::CPU& cpu, unsigned addressWord, unsigned lengthWord)
{
    netbRecord(word32(cpu.esp + 4 * addressWord), word32(cpu.esp + 4 * lengthWord));
}

/* A mocked function whose answer is steered, the same on both sides: a third of the time 0, a
 * third of the time -1, else what the script says. */
template <x86::reg32 Address>
void netbThreeWays(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, Address);
    const x86::reg32 pick = scripted(3);
    if (pick == 0)
        cpu.eax = 0;
    else if (pick == 1)
        cpu.eax = 0xffffffff;
}

/* Winsock's last error: those the code tells apart, and some it does not. */
void netbLastError(win32::WinApplication* app, x86::CPU& cpu)
{
    static const x86::reg32 kErrors[] = { 0, 0x2733, 0x2733, 0x2733, 0x2747, 0x2700, 0x271d, 0x271e, 0x271f, 0x2725, 0x2726, 0x2727,
                                          0x273e, 0x273f, 0x2740, 0x2741, 0x2742, 0x2743, 0x2744, 0x2747, 0x2748, 0x2749,
                                          0x274c, 0x274d, 0x274e, 0x274f, 0x2750, 0x2751, 0x2752 };
    mock(app, cpu, 0x533e38);
    cpu.eax = kErrors[scripted(std::size(kErrors))];
}

/* Time goes by while a case sleeps: what its waits wait for comes, sooner or later. */
void netbSleep(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x4df8e0);
    if (scripted(2) == 0)
    {
        setWord32(0x9f4d98, 0);
        setWord32(0xa0b0b8, 1);
        setWord32(0xa0b0b4, 1);
        setWord32(0xb00004, 1);
        if (word32(0x9f4d84) >= 0xb00000)
            setWord32(word32(0x9f4d84) + 0x34, 0);
    }
}

/* A message dispatched: the modem's thread may be told to stop by it. */
void netbDispatch(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, word32(0x53472c));
    if (scripted(2) == 0 && word32(0x9f4d84) >= 0xb00000)
        setWord32(word32(0x9f4d84) + 0x34, 0);
}

void netbWaitTasks(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x4e7720);
    if (scripted(2) == 0)
        setWord32(0xb00004, 1);
}

/* htons as it is: its answer is compared with what a packet holds. */
void netbHtons(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 value = word32(cpu.esp + 4) & 0xffff;
    mock(app, cpu, 0x533e4a);
    cpu.eax = ((value & 0xff) << 8) | (value >> 8);
}

/* TAPI's answers in the block it was handed: how much it needs, where its data is. */
void netbLineGetId(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 block = word32(cpu.esp + 20);
    netbRecord(block, 4);
    mock(app, cpu, 0x530a4e);
    setWord32(block + 4, scripted(2) ? 0x10 : 0x40);
    setWord32(block + 0x14, 0x20);
}

void netbLineGetConfig(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 block = word32(cpu.esp + 8);
    netbRecord(block, 4);
    mock(app, cpu, 0x53083d);
    setWord32(block + 4, scripted(3) ? 0x10 : 0x4000);
    setWord32(block + 0x10, 0x20 + scripted(0x40));
    setWord32(block + 0x14, 0x20);
}

void netbLineGetCaps(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 block = word32(cpu.esp + 20);
    netbRecord(block, 4);
    mock(app, cpu, 0x530747);
    setWord32(block + 4, scripted(2) ? 0x10 : 0x100);
    setWord32(block + 0x20, scripted(0x10));
    setWord32(block + 0x24, 0x40);
    setWord32(block + 0x3c, scripted(0));
}

/* recvfrom: a packet of four bytes now and then, a request or not. */
void netbRecvFrom(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 buffer = word32(cpu.esp + 8);
    mock(app, cpu, 0x533e5c);
    g_memory[buffer] = 0;
    g_memory[buffer + 1] = x86::reg8(scripted(2));
    g_memory[buffer + 2] = x86::reg8(scripted(2) ? 0x08 : 0xff);
    g_memory[buffer + 3] = x86::reg8(scripted(2) ? 0xbe : 0xff);
    if (scripted(2) == 0)
        cpu.eax = 4;
}

/* recv: nothing, an error, or what was asked for, a length of two bytes in network order first. */
void netbRecv(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 buffer = word32(cpu.esp + 8), wanted = word32(cpu.esp + 12);
    mock(app, cpu, 0x533e2c);
    const x86::reg32 pick = scripted(5);
    g_memory[buffer] = 0;
    g_memory[buffer + 1] = x86::reg8(scripted(0x40));
    cpu.eax = pick == 0 ? 0 : pick == 1 ? 0xffffffff : wanted;
}

/* fill: as it is, its bytes are read back. */
void netbFillByte(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 to = cpu.eax, bytes = cpu.edx, value = cpu.ebx;
    mock(app, cpu, 0x4e0716);
    std::memset(g_memory + to, int(value & 0xff), bytes < 0x1000 ? bytes : 0x1000);
}

/* A report: the words its format asks for. */
void netbError(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 format = word32(cpu.esp + 4);
    if (format == 0x54fbcc)
    {
        g_log.push_back(word32(cpu.esp + 8));
        g_log.push_back(word32(cpu.esp + 12));
    }
    else if (format == 0x54d30c)
    {
        g_log.push_back(word32(cpu.esp + 8));
    }
    mock(app, cpu, 0x401010);
}

/* Bytes given to the port: what is at ebx, edx of them.  Bytes taken from it: written there. */
void netbPortSend(win32::WinApplication* app, x86::CPU& cpu)
{
    netbRecord(cpu.ebx, cpu.edx < 0x100 ? cpu.edx : 0x100);
    mock(app, cpu, 0x51ba50);
}

void netbPortReceive(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 to = cpu.ebx, bytes = cpu.edx < 0x100 ? cpu.edx : 0x100;
    mock(app, cpu, 0x51b530);
    for (x86::reg32 i = 0; i < bytes; ++i)
        g_memory[to + i] = x86::reg8(scripted(0x100));
}

/* select: its sets' counts and sockets, the wait. */
void netbSelect(win32::WinApplication* app, x86::CPU& cpu)
{
    for (const x86::reg32 word : { 2u, 3u, 4u })
    {
        const x86::reg32 set = word32(cpu.esp + 4 * word);
        const x86::reg32 count = set ? word32(set) : 0;
        netbRecord(set, 4 + 4 * (count < 0x41 ? count : 0x41));
    }
    netbRecord(word32(cpu.esp + 20), 8);
    mock(app, cpu, 0x533e20);
    // The cases that run a thread end: it is told to stop, sooner or later.
    if (scripted(2) == 0)
    {
        g_memory[word32(0x9f37f0) + 0x46] = 1;
        setWord32(0xa0b0b8, 0);
    }
    if (scripted(4) == 0)
        cpu.eax = 0xffffffff;
}

template <x86::reg32 Address, unsigned AddressWord, unsigned LengthWord>
void netbBufferCall(win32::WinApplication* app, x86::CPU& cpu)
{
    netbRecordStack(cpu, AddressWord, LengthWord);
    mock(app, cpu, Address);
}

/* send: the bytes, and an answer of all sent, some, none or an error. */
void netbSend(win32::WinApplication* app, x86::CPU& cpu)
{
    netbRecordStack(cpu, 2, 3);
    const x86::reg32 bytes = word32(cpu.esp + 12);
    mock(app, cpu, 0x533e26);
    const x86::reg32 pick = scripted(4);
    cpu.eax = pick == 0 ? 0 : pick == 1 ? 0xffffffff : pick == 2 ? scripted(bytes + 1) : bytes;
}

/* What the world's lambdas need that is simplest said here. */
void netbRecordLength(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 address)
{
    netbRecord(word32(cpu.esp + 12), 4);
    mock(app, cpu, address);
}
