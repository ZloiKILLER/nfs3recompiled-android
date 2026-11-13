// What frontend_checks.cpp's world adds for these functions: statements, at the end of makeWorld.
{
    // sub_501050, the [INIT] log: its own hook, to record every argument of its format.
    g_methods[0x501050] = &nfs3hp::rendfLog;
    setWord32(0x567dc8, random(2) ? 0x517610 : 0);
    g_memory[0x564390] = x86::reg8(random(256));
    setWord32(0x5643f4, random(0));
    setWord32(0x9f38e4, random(0));
    setWord32(0x9f38e8, random(0));
    setWord32(0x9f38ec, random(0));
}
