// Recording stand-ins for the port's own functions that patches in these functions call: definitions, in namespace nfs3hp.
/* How far past the screen's edge a HUD box may go: the object and side recorded, a small answer. */
x86::sreg32 hudDragSlack(win32::WinApplication*, x86::reg32 object, x86::reg32 side)
{
    g_log.push_back(0x7e4f0000);
    g_log.push_back(object);
    g_log.push_back(side);
    g_log.push_back(watchedHash());
    return x86::sreg32(scripted(8));
}

/* The mixers, jumped to by name from the functions at 0x4f0880 on: the extraction turns only calls
 * into calls by address. */
namespace dead
{
void stub_0x50356c(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x50356c);
}
void stub_0x503280(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x503280);
}
}
namespace mix
{
void copySwapped(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x5033e4);
}
}
