// Recording stand-ins for the port's own functions that patches in these functions call: definitions, in namespace nfs3hp.
void mock(win32::WinApplication*, x86::CPU& cpu, x86::reg32 address);

/* The jmp of sub_4f2a78 is to the runtime's atexit by name. */
namespace watcom
{
void atexit(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x4f7c20);
}
}
