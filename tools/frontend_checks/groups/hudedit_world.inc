// What frontend_checks.cpp's world adds for these functions: statements, at the end of makeWorld.
// The stand-ins these functions call that the shared table has not: kSpecs takes
// no group's rows yet, so they go in here the first time (emplace: a row of the
// table's own wins).
{
    static const bool hudeditSpecsAdded = [] {
        auto& specs = const_cast<std::map<x86::reg32, Spec>&>(kSpecs);
        specs.emplace(0x422bd0, Spec{ EAX, 0, 0, 0, Kind::Shape });                  // a camera's target: memory to read
        specs.emplace(0x423960, Spec{ EAX | EDX, 0, 0, EDX, Kind::Plain });
        specs.emplace(0x4800a0, Spec{ EAX, 0, 0, 0, Kind::Plain });
        specs.emplace(0x482030, Spec{ EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain });
        specs.emplace(0x4844c0, Spec{ EAX, 0, 0, 0, Kind::Plain });
        specs.emplace(0x4853c0, Spec{ EAX | EDX, 0, 0, EDX, Kind::Plain });
        specs.emplace(0x45c9d0, Spec{ EAX, 0, 0, 0, Kind::NoEax });
        specs.emplace(0x4ee310, Spec{ EAX | EDX, 0, 0, EDX, Kind::Bool, true });     // strcmp: 0 the same
        specs.emplace(0x4e0ec0, Spec{ EAX | EDX, 0, 0, EDX, Kind::Bool, true });     // a file, 0 for none
        specs.emplace(0x4eff60, Spec{ EAX | EDX, 0, 0, EDX, Kind::Plain });
        specs.emplace(0x4f0548, Spec{ 0, 3, 0, 0, Kind::Plain });
        // The table has sub_4ef1e0 changing edi; it keeps it, and sub_45f4c0 counts on that.
        specs.find(0x4ef1e0)->second.clobbers = EDX;
        return true;
    }();
    (void)hudeditSpecsAdded;
}
