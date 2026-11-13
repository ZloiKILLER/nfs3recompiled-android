// Guest memory these functions write that kWatched does not hold: {from, to}, a line each.
{ 0x79c000, 0x79d000 }, // the clock
{ 0x9f2000, 0x9f5000 }, // the connections' queues, TAPI's state
{ 0xa0b000, 0xa0c000 }, // the advertising thread's state, the log's
{ 0xb00000, 0xb10000 }, // the cases' objects
