#ifndef LIB_RENDERER_H_
#define LIB_RENDERER_H_

#include <lib/winapp.h>
#include <winapi/types.h>
#include <lib/memmap.h>

struct SDL_Renderer;
struct SDL_Texture;

namespace win32
{

class Window;
class GlideRenderer;

class Renderer: public GenericResource
{
    friend class GlideRenderer;
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
     * so it is exposed here rather than kept private to present(). */
    void getViewportRect(int& x, int& y, int& w, int& h) const
    {
        x = m_vpX; y = m_vpY; w = m_vpW; h = m_vpH;
    }

private: // friend class GlideRenderer
    void present();
    void setCurrent();
    void clearCurrent();
    /* Negotiates the swap interval once and re-asserts it afterwards.  The
     * interval is context-global state that several code paths used to set
     * behind each other's backs, so every path that presents has to go
     * through here instead of calling SDL_GL_SetSwapInterval directly. */
    void ensureFramePacing();

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
    unsigned int    m_blitVertexArray;
    unsigned int    m_blitVertexBuffer;
    MemMap*         m_videoMemory;
    x86::reg32      m_currentBuffer;
    x86::reg32      m_width;
    x86::reg32      m_height;
    x86::reg32      m_depth;
    /* 0 until ensureFramePacing() has negotiated with the driver; >= 1 after. */
    int             m_swapInterval;
    /* Milliseconds to wait after presenting, to make up the refreshes the swap
     * interval could not hold.  Zero whenever the driver accepted the interval
     * we asked for, which is the normal case. */
    int             m_extraWaitMs;
    /* Destination of the palette->RGBA conversion in the 8-bit path, sized in
     * setVideoMode().  Null in 16-bit modes, which upload straight from guest
     * memory and need no conversion. */
    x86::reg8*      m_convertBuffer;
    x86::reg32      m_colorPalette[256];
    /* Letterbox viewport, recomputed in present() only when the window's
     * pixel size changes -- see getViewportRect(). m_lastWindowW/H start at
     * -1 so the first present() always computes it. */
    int             m_lastWindowW;
    int             m_lastWindowH;
    int             m_vpX, m_vpY, m_vpW, m_vpH;
};

}

#endif
