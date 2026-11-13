// Recording stand-ins for the port's own functions that patches in these functions call: definitions, in namespace nfs3hp.
// No patch in them calls any.  This stands in for a game function that needs more than kSpecs' kinds.
void mock(win32::WinApplication*, x86::CPU&, x86::reg32);

/* sub_501050, the [INIT] log: channel, level, format and what the format takes, recorded;
 * the caller cleans the stack.  It changes nothing else, or sub_5089d0's edx would be lost. */
void rendfLog(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 format = word32(cpu.esp + 12);
    const unsigned extra = format == 0x54f4c8 || format == 0x54f4ec ? 3 : format == 0x54f518 ? 1 : 0;
    for (unsigned i = 0; i < 3 + extra; ++i)
        g_log.push_back(word32(cpu.esp + 4 + 4 * i));
    g_log.push_back(cpu.eax);
    g_log.push_back(cpu.edx);
    mock(app, cpu, 0x501050);
}

/* sub_518fc0's jmp to sub_4e1890 is, in the generated code, a tail call by name to the
 * port's native, a form extract.py does not turn into a call by address. */
namespace ea
{
void purgememadr(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x4e1890);
}
}
