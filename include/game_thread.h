#ifndef GAME_THREAD_H_
#define GAME_THREAD_H_

#include <game_types.h>
#include <game_float.h>

namespace nfs
{

/* What a function the game calls through a pointer (its tables, callbacks, threads) is handed
 * and leaves: its register arguments by Watcom's convention, in and, for a caller that reads them
 * after the call, out; a float result where the function's is one.  The calling convention's
 * record, not a processor's registers: what is not passed stays 0. */
struct Args
{
    u32 eax = 0, edx = 0, ebx = 0, ecx = 0, esi = 0, edi = 0, ebp = 0;
    bool floatSet = false;
    double floatResult = 0.0;
    /* Float arguments by the math library's convention (the x87's top two, floatIn[0] the top),
     * how many were given, and how many the callee took off. */
    double floatIn[2] = {0.0, 0.0};
    unsigned floatsIn = 0, floatsTaken = 0;
};

/* A guest thread's state: no processor's any more, what the game's code, now C++, still shares
 * through its memory -- the top of its stack there, where it keeps its frames, records and the
 * words its calls pass; its thread block (what fs pointed to: the exception chain, the thread's
 * context lock); its floating-point environment (the control word's precision); and whether the game
 * is stopping. */
struct ThreadState
{
    u32 sp = 0;
    u32 threadBlock = 0;

    inline void init(u32 block)
    {
        fenv.init();
        terminate = false;
        threadBlock = block;
    }

    FloatEnv fenv;
    bool terminate = false;
};

}

#endif /* !GAME_THREAD_H_ */
