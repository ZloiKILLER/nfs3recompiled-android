"""Download Car taken off the car screens; never patch input EXEs.

Both car screens -- MENUS/CAR1.MNU and CAR2.MNU, player one's and split screen's
player two's -- list a Download Car tab (codelink CAR_DOWNLOAD) that fetched new
cars from the game's website.  There is nothing left to download, so the tab
only ever led to an error.  The screens are set up by sub_426d60, which looks up
each item by its codelink (sub_442a40) and gives CAR_DOWNLOAD its click handler
(sub_452e60) at +0x64; instead the item is hidden the way the game hides one of
its own -- `hidethis`, a few instructions further on (0x426e12), sets bit 0x10
of the flags byte at +5 -- so it is neither drawn nor reached by the keys.  It
is the last tab in its column, so nothing below it moves up into a gap.  The
menu files themselves come with the player's game data and stay as they are.
"""
# (generated file, address and bytes of the instruction, original code, patched code)
# sub_426d60 is native whole, the change made in it (native_frontend_cars.cpp);
# tools/frontend_checks/generate.py makes it to the game's code it is held against.
SITES = [
    # sub_426d60: CAR_DOWNLOAD found; hidden rather than given its handler.
    ("nfs3hp.6.cpp", "00426dfb  c74064602e4500",
     "    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(100) /* 0x64 */) = 4533856 /*0x452e60*/;",
     "    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */) |= x86::reg8(16 /*0x10*/); "
     "/* port: Download Car hidden, as hidethis is (tools/apply_hide_download.py) */"),
]


