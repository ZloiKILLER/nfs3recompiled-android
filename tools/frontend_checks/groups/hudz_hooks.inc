// Recording stand-ins for the port's own functions that patches in these functions call: definitions, in namespace nfs3hp.
/* sub_459db0's, after the layout is written: where and which element.  What the real one narrows
 * for a wide screen is memory neither side's code reads back here. */
void hudLayoutDesigned(win32::WinApplication*, x86::reg32 rect, x86::reg32 element)
{
    g_log.push_back(0x7e4d0000);
    g_log.push_back(rect);
    g_log.push_back(element);
    g_log.push_back(watchedHash());
}

/* Round the test sub_459db0 calls: the test's arguments before it, its answer after.  After it
 * the real one (nfs3hp_main.cpp) reads eax alone: ebx, ecx and edx there are what the test left
 * by address and what a direct call keeps, which nothing reads. */
void hudLayoutsDrawn(win32::WinApplication*, x86::CPU& cpu, bool drawn)
{
    g_log.push_back(0x7e4e0000 | (drawn ? 1 : 0));
    g_log.push_back(cpu.eax);
    if (drawn)
    {
        g_log.push_back(cpu.ebx);
        g_log.push_back(cpu.ecx);
        g_log.push_back(cpu.edx);
    }
    g_log.push_back(watchedHash());
}
