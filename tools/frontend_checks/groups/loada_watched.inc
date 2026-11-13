// Guest memory these functions write that kWatched does not hold: {from, to}, a line each.
{ 0x5dd000, 0x5de000 }, // the road's slices: their count and table
{ 0x5ef000, 0x5f0000 }, // the race's lists of cars and their counts
{ 0x679000, 0x67a000 }, // the all-cops race's last colour
{ 0x6ff000, 0x701000 }, // the horizon's pieces and heights
{ 0x7a4000, 0x7a6000 }, // the replay's state, past kWatched's
{ 0x7cd000, 0x7ce000 }, // the replay's flags
{ 0x7d3000, 0x7d4000 }, // the cop's light
{ 0xb00000, 0xb20000 }, // this group's made-up cars, strings, lists, slices, race cars and saved race
