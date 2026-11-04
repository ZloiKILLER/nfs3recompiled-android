// What frontend_checks.cpp's world adds for these functions: statements, at the end of makeWorld.
{
    // The calls these functions make that no other screen does: what each reads,
    // what its ret pops, what it changes.  kSpecs is the checks' own, and no group
    // has a place in it: these stand-ins are this group's only.
    static const bool connectSpecs = [] {
        auto& specs = const_cast<std::map<x86::reg32, Spec>&>(kSpecs);
        const std::uint8_t kArgs3 = EAX | EBX | EDX, kArgs4 = EAX | EBX | ECX | EDX, kAllBut = EBX | ECX | EDX;
        // The menus' own.
        specs[0x49c080] = { 0, 0, 0, 0, Kind::Plain };                 // the mouse's key
        specs[0x4dab90] = { 0, 0, 0, 0, Kind::Plain };                 // the pointer drawn
        specs[0x4e1620] = { kArgs3, 0, 0, EBX | EDX, Kind::Tuning };   // memory reserved: none, or room for a short list
        specs[0x4633d0] = { EAX, 0, 0, 0, Kind::Plain };               // a list's selection up
        specs[0x445750] = { kArgs4, 8, 0x20, kAllBut, Kind::Plain };   // a list opened
        specs[0x445150] = { 0, 0, 0, 0, Kind::KeyHandler };            // the list's focus, 0 to 11
        specs[0x44b170] = { 0, 0, 0, 0, Kind::Plain };                 // the menu redrawn
        specs[0x444a60] = { 0, 0, 0, 0, Kind::Plain };                 // the message ended
        specs[0x4eedf0] = { kArgs3, 0, 0, EBX | EDX, Kind::Plain, true };  // strncat
        specs[0x4cc970] = { 0, 0, 0, 0, Kind::Bool };                  // the next key
        specs[0x49f790] = { 0, 0, 0, 0, Kind::Bool };                  // dial-up networking there
        specs[0x49f880] = { EAX, 0, 0, 0, Kind::Bool };                // its phone book
        // The session's, under the group's own.
        specs[0x49f1c0] = { EAX, 0, 0, 0, Kind::Focus };               // a kind's place in the connections
        specs[0x4eb2e0] = { 0, 0, 0, 0, Kind::Plain };                 // a lock made
        specs[0x4f4e60] = { EAX, 0, 0, 0, Kind::Plain };
        specs[0x4f4d10] = { kArgs4, 3, 0xc, kAllBut, Kind::Bool };
        specs[0x4f4c70] = { kArgs4, 0, 0, kAllBut, Kind::Bool };
        specs[0x4f4eb0] = { kArgs4, 0, 0, kAllBut, Kind::Bool };
        specs[0x4f4f10] = { kArgs4, 0, 0, kAllBut, Kind::Bool };
        specs[0x4f4ed0] = { kArgs4, 0, 0, EBX | EDX, Kind::Bool };
        specs[0x4f5040] = { kArgs3, 0, 0, 0, Kind::Bool };
        specs[0x4f5050] = { kArgs4, 0, 0, 0, Kind::Bool };
        specs[0x4f5280] = { 0, 0, 0, 0, Kind::Plain };
        specs[0x4f5850] = { kArgs4, 0, 0, kAllBut, Kind::Bool };
        specs[0x4f63d0] = { EAX, 0, 0, 0, Kind::Plain };
        specs[0x4f7830] = { EAX, 0, 0, 0, Kind::Bool };
        specs[0x4e7520] = { kArgs3, 0, 0, EBX | EDX, Kind::Plain };    // a system task added
        specs[0x4e75e0] = { EAX, 0, 0, 0, Kind::Plain };               // and taken away
        specs[0x4e7630] = { EAX, 0, 0, 0, Kind::Plain };               // the tasks run
        specs[0x4ee310] = { EAX | EDX, 0, 0, EDX, Kind::Plain, true }; // strcmp
        specs[0x4e0820] = { EAX | EDX, 0, 0, EDX, Kind::Bool, true };  // strstr
        specs[0x4a6390] = { EAX, 0, 0, 0, Kind::Plain };
        specs[0x4a6500] = { 0, 0, 0, 0, Kind::Plain };
        specs[0x42d790] = { 0, 0, 0, 0, Kind::Plain };
        specs[0x42d7f0] = { 0, 0, 0, 0, Kind::Plain };
        specs[0x49a4d0] = { 0, 0, 0, 0, Kind::Plain };
        specs[0x4a65b0] = { 0, 0, 0, 0, Kind::Bool };
        specs[0x49eed0] = { 0, 0, 0, 0, Kind::Plain };
        specs[0x4a6ce0] = { EAX, 0, 0, 0, Kind::Bool };
        specs[0x4a60e0] = { kArgs3, 0, 0, 0, Kind::Plain };
        specs[0x4a66c0] = { 0, 0, 0, 0, Kind::Plain };
        specs[0x4fa5f0] = { EAX, 0, 0, 0, Kind::Bool };
        specs[0x4fa550] = { kArgs3, 0, 0, EBX | EDX, Kind::Bool };
        specs[0x4fa780] = { kArgs3, 0, 0, EBX | EDX, Kind::Bool };
        // Not the group's functions when it is not checking them (groups/connect.txt).
        specs[0x49e5f0] = { EAX | EDX, 0, 0, EDX, Kind::Bool };
        specs[0x49e9d0] = { EAX, 0, 0, 0, Kind::Bool };
        specs[0x49ea70] = { EAX | EDX, 0, 0, EDX, Kind::Bool };
        // The system's lock, through the import table: what is there in the file.
        specs[word32(0x5344b8)] = { 0, 1, 4, 0, Kind::NoEax };
        specs[word32(0x534588)] = { 0, 1, 4, 0, Kind::NoEax };
        return true;
    }();
    (void)connectSpecs;
}
