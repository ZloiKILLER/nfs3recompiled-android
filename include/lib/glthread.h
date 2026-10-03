#ifndef LIB_GLTHREAD_H_
#define LIB_GLTHREAD_H_

#include <functional>

namespace win32
{

class Renderer;

/* OpenGL on a thread of its own.
 *
 * The game thread used to make every GL call itself: the driver's work on each
 * batch, the upload of a frame's vertices, the swap -- about a sixth of the
 * thread in a race, on the one core that runs the whole game.  Now it only
 * writes down what is to be drawn, with a copy of everything the drawing needs,
 * and this thread, which holds the GL context, draws it in the same order while
 * the game goes on with its next frame.  It is at most one frame behind
 * (throttle): the picture is as late as a swap made it before, and no later.
 *
 * NFS_GL_THREAD=0 runs everything on the calling thread, as it was: post() then
 * makes the context current, runs the work and lets the context go. */
namespace glthread
{

/* Whether the GL work goes to the thread; read once. */
bool on();

/* `work`, with `renderer`'s context current: on the GL thread after everything
 * posted before it, or here and now. */
void post(Renderer* renderer, std::function<void()> work);

/* Returns once everything posted has run. */
void finish();

/* Returns once everything posted has run and the GL thread holds no context,
 * so that another thread may make one current on the window -- to create a
 * renderer -- or SDL may take the window's surface away (the app going to the
 * background). */
void release();

/* A present was posted; returns once at most `frames` of them are still to run
 * -- the game thread waits here instead of in the swap.  Returns how long it
 * waited, in nanoseconds. */
unsigned long long presentPosted(int frames);

/* Called by a posted present when it has run. */
void presentDone();

}

}

#endif
