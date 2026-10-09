#ifndef LIB_RENDERER_H_
#define LIB_RENDERER_H_

#include <lib/winapp.h>
#include <winapi/types.h>
#include <lib/memmap.h>
#include <vector>

struct SDL_Renderer;
struct SDL_Texture;

namespace win32
{

class Window;
class GlideRenderer;
class ThrashRenderer;

class Renderer: public GenericResource
{
    friend class GlideRenderer;
    friend class ThrashRenderer;
public:
    Renderer(WinApplication* application, Window* window);
    ~Renderer();

    x86::reg32 lock(x86::reg32 index);
    void unlock(x86::reg32 index);

    void swap();
    void setVideoMode(x86::reg32 w, x86::reg32 h, x86::reg32 bpp);
    void updatePalette(x86::reg32 colorCount, const x86::reg32* colors);
    x86::reg32 getVideoMemorySize() const { return m_videoMemory->getBlockSize(); }

    x86::reg32 getWidth() const     { return m_width; }
    x86::reg32 getHeight() const    { return m_height; }

    /* The on-screen rectangle the game's framebuffer is letterboxed into --
     * recomputed by present() only when the window's pixel size actually
     * changes (see m_lastWindowW/H).  Touch input needs exactly this rect to
     * map a tap's window coordinates back into game framebuffer coordinates,
     * so it is exposed here rather than kept private to present().
     *
     * Counted from the top of the window, the way a finger is reported, and
     * not from the bottom, the way OpenGL wants its viewport.  The two agree
     * while the letterbox is even, which is why they went unremarked -- until
     * the keyboard raised the picture and the finger, mapped through a
     * rectangle that had moved the other way, pressed twice as far above
     * itself as the picture had risen. */
    /* Worked out on the GL thread and read from the input threads, so under a
     * lock of its own: a rectangle read half before and half after a resize
     * would send a tap somewhere neither rectangle has it. */
    void getViewportRect(int& x, int& y, int& w, int& h) const
    {
        SDL_LockSpinlock(&m_viewportLock);
        x = m_vpX; y = m_lastWindowH - (m_vpY + m_keyboardShift + m_vpH); w = m_vpW; h = m_vpH;
        SDL_UnlockSpinlock(&m_viewportLock);
    }

    /* A frame of a movie, RGBA w x h, shown at (x, y) as dw x dh of the
     * game's picture (scaled by the GPU, filtered), the rest of it black.
     * The MAD player's software output skips the 16-bit surface for this
     * (native_movie.cpp). */
    void presentMovieFrame(std::vector<x86::reg8> rgba, x86::reg32 w, x86::reg32 h,
                           int x, int y, int dw, int dh);

    /* The renderer that last showed the game's DirectDraw picture: the one a
     * movie is shown through. */
    static Renderer* active();

    /* This renderer's context current on the calling thread, and let go of --
     * glthread's business, and the constructor's. */
    void setCurrent();
    void clearCurrent();

private: // friend class GlideRenderer
    void present();
    /* present()'s GL work: the letterboxed blit of the frame and the swap. */
    void presentFrame(x86::reg32 width, x86::reg32 height);
    /* Sets vsync and works out the frame period from NFS_FPS_CAP, once.  Every
     * path that presents goes through here rather than calling
     * SDL_GL_SetSwapInterval itself. */
    void ensureFramePacing();
    /* Holds a new frame back until its time comes: one frame period after the
     * last one's.  Called by the two paths that swap -- the Glide swap and the
     * DirectDraw flip -- with the guest lock let go. */
    void paceFrame();

private:
    void update();
    /* Builds the shader, VAO and VBO that blit m_texture to the window.  The
     * old path used glBegin/glVertex3f and the fixed-function matrix stack,
     * none of which exists in GLES. */
    void initBlit();
    x86::reg32 getFrontBuffer() const;
    x86::reg32 getBackBuffer() const;

private:
    WinApplication* m_application;
    Window*         m_window;
    void*           m_renderer;
    unsigned int    m_texture;
    unsigned int    m_blitProgram;
    /* Where u_gamma lives in the blit program, or -1 if the driver optimised
     * it away because the value never differs from one. */
    int             m_blitGammaUniform;
    int             m_blitBrightnessUniform;
    int             m_blitContrastUniform;
    unsigned int    m_blitVertexArray;
    unsigned int    m_blitVertexBuffer;
    MemMap*         m_videoMemory;
    x86::reg32      m_currentBuffer;
    x86::reg32      m_width;
    x86::reg32      m_height;
    x86::reg32      m_depth;
    /* 0 until ensureFramePacing() has set vsync; 1 after. */
    int             m_swapInterval;
    /* The frame period NFS_FPS_CAP asks for, in nanoseconds, 0 for none; and
     * when the next frame is due. */
    Uint64          m_frameNs;
    Uint64          m_nextFrameNs;
    /* For the performance hint (paceFrame): when this frame's work began --
     * the end of the last wait for its time -- and how much of it since went
     * waiting on the GPU and the display in present(). */
    Uint64          m_frameStartNs;
    Uint64          m_presentNs;
    /* Destination of the conversion to RGBA of a DirectDraw frame, sized in
     * setVideoMode().  The GL side's own: the conversion runs there, on the
     * copy of the guest's frame update() hands it. */
    x86::reg8*      m_convertBuffer;
    /* The 16-bit guest frame as an RGB565 texture, and the framebuffers that
     * copy it into m_texture on the GPU (update()). */
    unsigned int    m_frame565;
    unsigned int    m_frameRead;
    unsigned int    m_frameDraw;
    /* A movie frame, and its size (presentMovieFrame). */
    unsigned int    m_movieTexture = 0;
    x86::reg32      m_movieWidth = 0;
    x86::reg32      m_movieHeight = 0;
    x86::reg32      m_colorPalette[256];
    /* Letterbox viewport, recomputed in present() only when the window's
     * pixel size changes -- see getViewportRect(). m_lastWindowW/H start at
     * -1 so the first present() always computes it. */
    int             m_lastWindowW;
    int             m_lastWindowH;
    int             m_vpX, m_vpY, m_vpW, m_vpH;
    /* Pixels the picture is raised by while the on-screen keyboard is up, so
     * the game's own text field clears it.  Kept apart from m_vpY because that
     * one is cached until the window resizes, and a shift folded into it would
     * outlive the keyboard. */
    int             m_keyboardShift;
    mutable SDL_SpinLock m_viewportLock = 0;
};

}

#endif
