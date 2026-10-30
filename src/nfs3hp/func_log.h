#pragma once

namespace nfs3hp
{
/* Debug mode's "Live functions log" (NFS_FUNC_LOG=1): starts sampling which of
 * the game's functions run, and writes it to functions.bin in the game's
 * folder as the game plays.  See func_log.cpp. */
void startFunctionLog();
}
