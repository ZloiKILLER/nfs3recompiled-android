// Recording stand-ins for the port's own functions that patches in these functions call: definitions, in namespace nfs3hp.
// No patch in them calls any.  These stand in for game functions that need more than
// kSpecs' kinds: what a line is handed in the frame, a pane step that changes the panes.
void mock(win32::WinApplication*, x86::CPU&, x86::reg32);

/* sub_4c0b20: its two vertices, all of them recorded: they live in the caller's frame,
 * which nothing else looks at. */
void hudrSegment(win32::WinApplication* app, x86::CPU& cpu)
{
    for (const x86::reg32 vertex : { cpu.eax, cpu.edx })
        for (x86::reg32 at = 0; at < 0x20; at += 4)
            g_log.push_back(word32(vertex + at));
    mock(app, cpu, 0x4c0b20);
}

/* sub_4c11b0: its three. */
void hudrTriangle(win32::WinApplication* app, x86::CPU& cpu)
{
    for (const x86::reg32 vertex : { cpu.eax, cpu.edx, cpu.ebx })
        for (x86::reg32 at = 0; at < 0x20; at += 4)
            g_log.push_back(word32(vertex + at));
    mock(app, cpu, 0x4c11b0);
}

/* sub_48e790: a step of the map's pane.  The real one moves the style and the rectangles
 * sub_48e920 waits on; here, one time in three, it brings them together, or sub_48e920
 * would never end. */
void hudrPaneStep(win32::WinApplication* app, x86::CPU& cpu)
{
    mock(app, cpu, 0x48e790);
    if (scripted(3) != 0)
        return;
    const x86::reg32 style = scripted(4), top = scripted(2) ? 0 : 0x80000000u + scripted(2);
    for (const x86::reg32 record : { 0x6fbf98u, 0x6fc2e0u })
        for (const x86::reg32 form : { 0u, 0x1a4u })
        {
            setWord32(record + form + 0x20, style);
            setWord32(record + form + 0x24 + 17 * 16 + 4, top);
            setWord32(record + form + 0x24 + 22 * 16 + 4, top);
        }
}
