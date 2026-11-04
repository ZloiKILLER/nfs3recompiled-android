// Recording stand-ins for the port's own functions that patches in these functions call: definitions, in namespace nfs3hp.
void hudEditorDraw(win32::WinApplication*, x86::reg32 object, bool begin)
{
    g_log.push_back(0x7e4a0000 | (begin ? 1 : 0));
    g_log.push_back(object);
    g_log.push_back(watchedHash());
}

void hudEditorFrame(win32::WinApplication*, x86::reg32 object, bool drawn)
{
    g_log.push_back(0x7e4b0000 | (drawn ? 1 : 0));
    g_log.push_back(object);
    g_log.push_back(watchedHash());
}

/* The phone's settings: laid or not, from the script.  What the real one lays over the settings is
 * memory neither side's code reads back here. */
bool firstSettings(win32::WinApplication*)
{
    g_log.push_back(0x7e4c0000);
    g_log.push_back(watchedHash());
    return scripted(2) != 0;
}

/* A stand-in that also records where the stack stands: the frame the function under test keeps. */
void mock(win32::WinApplication*, x86::CPU& cpu, x86::reg32 address);

template <x86::reg32 Address>
void stackSeen(win32::WinApplication* app, x86::CPU& cpu)
{
    g_log.push_back(cpu.esp);
    mock(app, cpu, Address);
}
