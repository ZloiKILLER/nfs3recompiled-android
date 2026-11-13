// Guest memory these functions write that kWatched does not hold: {from, to}, a line each.
{ 0x700000, 0x701000 }, // the markers' flags, the rectangles' counts
{ 0x724000, 0x726000 }, // the fixed quads, the kept quads, the standings' rows, the pursuit rows
{ 0x749000, 0x74a000 }, // the elements' boxes: noise, read as arguments
{ 0x791000, 0x792000 }, // the batches' addresses and first numbers
{ 0x900000, 0x90f000 }, // the batches, as rendbNewBatch hands them out
{ 0x930000, 0x931000 }, // the frame of element 9
