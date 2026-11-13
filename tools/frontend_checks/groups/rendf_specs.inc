{ 0x501050, { 0, 0, 0, 0, Kind::Plain } },  // its arguments recorded by rendfLog, by its format
{ 0x4e1620, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },
{ 0x507f30, { EAX, 0, 0, 0, Kind::Bool } },  // 0 now and then, or sub_5089d0 never looks again
{ 0x508170, { 0, 0, 0, 0, Kind::Plain } },  // those calls may read EAX | EBX
{ 0x5082f0, { 0, 0, 0, 0, Kind::Plain } },  // those calls may read ECX
{ 0x508390, { EAX, 0, 0, 0, Kind::Bool } },  // 0 now and then: the mode cannot be opened
{ 0x517610, { EAX | EBX | EDX, 0, 0, EBX | EDX, Kind::Plain } },  // [0x567dc8], the screen's function for a mode
