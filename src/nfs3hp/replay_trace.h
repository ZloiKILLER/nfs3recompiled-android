#ifndef NFS3HP_REPLAY_TRACE_H
#define NFS3HP_REPLAY_TRACE_H

#include <lib/winapp.h>

namespace nfs3hp
{

// NFS_REPLAY_TRACE=1: every car, every physics step, to logs/ (replay_trace.cpp).
void startReplayTrace();
// The race is over: the file closed.
void endReplayTrace(win32::WinApplication* app);

}  // namespace nfs3hp

#endif
