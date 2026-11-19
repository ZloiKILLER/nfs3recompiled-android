#ifndef NFS3HP_PORT_SND_STREAM_CALLS_H
#define NFS3HP_PORT_SND_STREAM_CALLS_H

#include "native_guest.h"
#include "port.h"

/* port_snd_stream.cpp's typed cores that the menus call as C++ (guest::direct), and how the game's code calls
 * each by address.  Not in port_snd_stream.h, which names the
 * game's addresses the stream files call.  A Result is nullopt when the game stopped in a call.  A core's `esp` is the
 * frame base its thunk hands it: the game's esp on entry, at the return address.  A caller through
 * guest::direct passes ts.sp - 4 - 4 * N, N the stack words the game's call pushes: the esp the thunk hands
 * it, so the core's frame lies where the game's does; by address the call makes it. */
namespace nfs3hp
{
namespace port
{
namespace stream
{

// The game's 0x4e7470 (SNDstreamautovol): the stream's voice to volume `volume` over `ticks`; esp the frame base.
Result fadeStream(App* app, ThreadState& ts, u32 esp, int32_t ticks, int32_t volume);
// The game's 0x4e7520 (addsystemtask): `function` run every `period` ticks, the first `delay` ticks from now; the
// tick it runs first; esp the frame base.
Result scheduleTask(App* app, ThreadState& ts, u32 esp, u32 function, int32_t period, u32 delay);
// The game's 0x4e75e0: `function` no longer a task; its slot's offset, 0x100 for none.
u32 unscheduleTask(App* app, u32 function);
// The game's 0x4e7630: the tasks due run, each with `argument` and the ticks it is late; what they gave back,
// or-ed; esp the frame base.
Result runDueTasks(App* app, ThreadState& ts, u32 esp, u32 argument);

// The streamer (port_snd_stream.cpp), as the game's sound calls it.
// The game's 0x4e74f0 (SNDstreamgetvol): the stream's volume; esp the frame base.
Result streamVolume(App* app, ThreadState& ts, u32 esp);
// The game's 0x4e784c (SNDstreamrestore): the streamer done with; esp the frame base.
Result restoreStreamer(App* app, ThreadState& ts, u32 esp);
// The game's 0x4e7a14 (SNDstreaminterrupt): the stream cut to file `name` at `offset`; esp the frame base.
Result interruptStream(App* app, ThreadState& ts, u32 esp, u32 name, u32 offset);
// The game's 0x4e7b28 (SNDstreampurge): the stream stopped and what it held dropped; esp the frame base.
Result purgeStreamer(App* app, ThreadState& ts, u32 esp);
// The game's 0x4e8320: the end chunks the streamer has seen.
u32 endsSeen(App* app);

// The CD streams (port_snd_cdstream.cpp).
// The game's 0x4e4790: the first CD stream made, of `size` bytes in pool `pool`, blocks of `blockSize` read at
// once: the stream; esp the frame base.
Result makeFirstStream(App* app, ThreadState& ts, u32 esp, u32 size, u32 blockSize, u32 pool);
// The game's 0x4e4b50 (delstream): CD stream `stream` freed.  edx, ebx and ecx are what the caller has in them:
// the code keeps them (pushes them first), the function takes them.  esp the frame base.
Result deleteCdStream(App* app, ThreadState& ts, u32 esp, u32 stream, u32 edx, u32 ebx, u32 ecx);

// The packet players (port_snd_strm.cpp), as the mixer calls them.
// The game's 0x5118fc: packet player `index`'s next packet, its frames into [framesAt]: its data, 0 for none; esp
// the frame base.
Result packetTaken(App* app, ThreadState& ts, u32 esp, int32_t index, u32 framesAt);
// The game's 0x511998: `frames` frames of packet player `index` played, its frames callback told: eax as the
// callback leaves it (the index without one); esp the frame base.
Result packetFramesPlayedOf(App* app, ThreadState& ts, u32 esp, int32_t index, u32 frames);

// The movies' sound streams (port_snd_strm.cpp), as the movie player calls them.
// The game's 0x4f3a10: the bytes a stream of `requests` requests and `packets` packets takes.
u32 streamMemoryOf(App* app, u32 requests, u32 packets);
// The game's 0x4f3a60: a stream on queue `queue`, the play's `options`, `requests` requests, `packets` packets, in
// the `size` bytes at `memory`: its index; esp the frame base (two words pushed).
Result createQueued(App* app, ThreadState& ts, u32 esp, u32 queue, u32 options, u32 requests, u32 packets,
                    u32 memory, u32 size);
// The game's 0x4f3a80: a request on stream `index` for time `time` of the chunks `chunks`: its handle; esp the
// frame base.
Result requestData(App* app, ThreadState& ts, u32 esp, u32 index, u32 time, u32 chunks);
// The game's 0x4f2f50 (SNDSTRM_requeststatus): request `handle`'s state into the four words at `out`; esp the
// frame base.
Result requestStatusOf(App* app, ThreadState& ts, u32 esp, u32 handle, u32 out);
// The game's 0x4f43c0 (SNDSTRM_destroy): stream `index` gone; esp the frame base.
Result destroyStreamOf(App* app, ThreadState& ts, u32 esp, int32_t index);

// Pathfinder (port_snd_path.cpp).
// The game's 0x4e6dc8 (SNDpathinit): Pathfinder set up on map `map`; esp the frame base.
Result pathInit(App* app, ThreadState& ts, u32 esp, u32 map);
// The game's 0x4e6f0c (SNDpathstartstream): the music started on CD stream `stream` from file `name`, `time`
// buffered ahead; esp the frame base.
Result startPathStream(App* app, ThreadState& ts, u32 esp, u32 stream, u32 name, u32 time);
// The game's 0x4e705c (SNDpathcontrol): the controller to `value`; esp the frame base.
Result pathControl(App* app, ThreadState& ts, u32 esp, int32_t value);
// The game's 0x4e70fc (SNDpathevent): event `event` of kind `kind`; esp the frame base.
Result pathEvent(App* app, ThreadState& ts, u32 esp, int32_t event, int32_t kind);
// The game's 0x4e7208 (SNDpatheventinterrupt): the stream cut over to event `event`'s node; esp the frame base.
Result pathEventInterrupt(App* app, ThreadState& ts, u32 esp, int32_t event);
// The game's 0x4e7388 (SNDpathrestore): Pathfinder done with; esp the frame base.
Result pathRestore(App* app, ThreadState& ts, u32 esp);

}  // namespace stream
}  // namespace port

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
