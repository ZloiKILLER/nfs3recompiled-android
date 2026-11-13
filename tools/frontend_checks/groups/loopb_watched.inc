// Guest memory these functions write that kWatched does not hold: {from, to}, a line each.
{ 0x5dd000, 0x5de000 }, // the slices of a lap
{ 0x5ef000, 0x5f0000 }, // the lists of cars and their counts
{ 0x749000, 0x74a000 }, // the strip the pointer goes along
{ 0x79c000, 0x79d000 }, // the controls of this tick, the real tick
{ 0x79f000, 0x7a0000 }, // the x87's control words
{ 0x7a4000, 0x7ce000 }, // the replay's data, its table, its players' blocks, the views
{ 0x7d3000, 0x7d4000 }, // the tick and the race's end
{ 0x7f7000, 0x7f8000 }, // the performance lines shown
{ 0xb00000, 0xb10000 }, // this group's made-up cars and records
