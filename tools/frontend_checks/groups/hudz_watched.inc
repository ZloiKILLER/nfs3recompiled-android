// Guest memory these functions write that kWatched does not hold: {from, to}, a line each.
{ 0x700000, 0x701000 }, // the views' counts of quads, the frames' queue
{ 0x724000, 0x726000 }, // the replay bar's quads, the alignments
{ 0x749000, 0x74a000 }, // the elements' boxes
{ 0x791000, 0x792000 }, // the batches' vertices and corners
{ 0x79c000, 0x7a0000 }, // the cases' items and corners
{ 0x7cd000, 0x7ce000 }, // the view's size, the replay bar's state
{ 0x7d3000, 0x7d4000 }, // the clip bounds
