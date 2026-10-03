#include <lib/audio.h>
#include <lib/mutex.h>
#include <SDL3/SDL.h>
#include <vector>
#include <algorithm>

namespace win32
{

static const x86::reg32 s_channelCount = 2;
static const x86::reg32 s_sampleCount = 4096;
static const x86::reg32 s_sampleSize = s_sampleCount * s_channelCount;

/* The game mixes at 22050 Hz and the stream takes it at that rate; the device
 * runs at the phone's own rate (NFS_AUDIO_RATE, from AudioManager), and SDL
 * converts between the two.  Opened at the game's rate instead -- raised to
 * SDL's 44100 floor -- the device ran at a rate the phone does not, so Android
 * resampled it inside its mixer, which keeps a stream off the low-latency path:
 * the sound lagged the picture by a quarter of a second or more on some phones,
 * and not on others.  Without the variable -- the desktop -- the device opens as
 * it always did. */
AudioDevice::AudioDevice()
    :   m_device(0)
    ,   m_stream(nullptr)
{
    SDL_AudioSpec game;
    game.format   = SDL_AUDIO_S16;
    game.channels = s_channelCount;
    game.freq     = 22050;
    SDL_AudioSpec output = game;
    const char* nativeRate = SDL_getenv("NFS_AUDIO_RATE");
    const int rate = nativeRate ? SDL_atoi(nativeRate) : 0;
    if (rate >= 8000 && rate <= 192000)
        output.freq = rate;
    m_device = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &output);
    if (!m_device)
    {
        SDL_Log("[AUDIO] no output: %s", SDL_GetError());
        return;
    }
    // Paused until a buffer plays, as SDL_OpenAudioDeviceStream left it.
    SDL_PauseAudioDevice(m_device);
    m_stream = SDL_CreateAudioStream(&game, &output);
    if (!m_stream || !SDL_SetAudioStreamGetCallback(m_stream, &audioCallback22050, this)
        || !SDL_BindAudioStream(m_device, m_stream))
    {
        SDL_Log("[AUDIO] no stream: %s", SDL_GetError());
        return;
    }
    SDL_AudioSpec opened;
    int frames = 0;
    if (SDL_GetAudioDeviceFormat(m_device, &opened, &frames))
        SDL_Log("[AUDIO] output at %d Hz, %d channels, %d frames a buffer; the game's %d Hz converted to it",
                opened.freq, opened.channels, frames, game.freq);
}

AudioDevice::~AudioDevice()
{
    SDL_DestroyAudioStream(m_stream);
    if (m_device)
        SDL_CloseAudioDevice(m_device);
}

/* The list of playing buffers is changed here, on a guest thread, and walked by
 * audioCallback22050 on SDL's audio thread, which SDL calls with the stream
 * locked; so the list changes under that lock.  Pausing and resuming the device
 * stay outside it: the audio thread takes the device's lock before the
 * stream's, and the other order here could deadlock with it. */
void AudioDevice::play(AudioBuffer* buffer)
{
    if (m_stream)
        SDL_LockAudioStream(m_stream);
    const bool first = m_playingBuffers.empty();
    if (std::find(m_playingBuffers.begin(), m_playingBuffers.end(), buffer) == m_playingBuffers.end())
        m_playingBuffers.push_back(buffer);
    if (m_stream)
        SDL_UnlockAudioStream(m_stream);
    if (first && m_device)
        SDL_ResumeAudioDevice(m_device);
}

void AudioDevice::stop(AudioBuffer* buffer)
{
    if (m_stream)
        SDL_LockAudioStream(m_stream);
    m_playingBuffers.erase(std::remove(m_playingBuffers.begin(), m_playingBuffers.end(), buffer), m_playingBuffers.end());
    const bool none = m_playingBuffers.empty();
    if (m_stream)
        SDL_UnlockAudioStream(m_stream);
    if (none && m_device)
        SDL_PauseAudioDevice(m_device);
}


/* Every playing buffer mixed into one silence-filled block, each adding to what
 * the ones before it left, clamped to 16 bits. */
void AudioDevice::audioCallback22050(void* userdata, SDL_AudioStream* stream, int additional_amount, int /*total_amount*/)
{
    AudioDevice* audio = reinterpret_cast<AudioDevice*>(userdata);
    std::vector<x86::reg8> buffer(additional_amount, 0);
    for (std::vector<AudioBuffer*>::iterator it = audio->m_playingBuffers.begin();
        it != audio->m_playingBuffers.end();
        ++it)
    {
        (*it)->audioCallback22050(buffer.data(), additional_amount);
    }
    SDL_PutAudioStreamData(stream, buffer.data(), additional_amount);
    return;
}


AudioBuffer::AudioBuffer(AudioDevice* device, WinApplication* app, x86::reg32 bufferSize)
    :   m_device(device)
    ,   m_bufferSize(bufferSize ? bufferSize : s_sampleSize  * sizeof(x86::sreg16))
    ,   m_memmap(new MemMap(m_bufferSize))
    ,   m_bufferMap(new MemMap(m_bufferSize))
    ,   m_buffer(&app->getMemory<x86::sreg16>(m_memmap->getBlockStart()))
    ,   m_playStart(0)
    ,   m_playStop(0)
    ,   m_primary(bufferSize == 0)
{
}

x86::reg32 AudioBuffer::bufferSize() const
{
    return m_bufferSize;
}

AudioBuffer::~AudioBuffer()
{
    delete m_bufferMap;
    delete m_memmap;
}

void AudioBuffer::play()
{
    m_device->play(this);
}

void AudioBuffer::stop()
{
    m_device->stop(this);
}

/* Adds what is written and not yet played, up to `len` bytes, to `stream`.  The
 * device's callback hands every buffer the same block: a buffer that zeroed it
 * first, as this one used to, wiped out the buffers mixed before it. */
void AudioBuffer::audioCallback22050(x86::reg8* stream, int len)
{
    x86::sreg16* destData = reinterpret_cast<x86::sreg16*>(stream);
    x86::reg32 start = m_playStart;
    x86::reg32 stop = start + len;
    if (stop > m_playStop)
    {
        stop = m_playStop;
    }
    if (stop <= start) return;
    x86::reg32 samples = (stop - start) / sizeof(x86::sreg16);
    x86::reg32 at = (start % m_bufferSize) / sizeof(x86::sreg16);
    const x86::reg32 ring = m_bufferSize / sizeof(x86::sreg16);
    for (; samples; --samples, ++destData)
    {
        const int mixed = int(*destData) + int(m_buffer[at]);
        *destData = x86::sreg16(mixed > 32767 ? 32767 : mixed < -32768 ? -32768 : mixed);
        if (++at == ring)
            at = 0;
    }
    m_playStart = stop;
    return;
}

/*void AudioBuffer::audioCallback44100(void *userdata, x86::reg8 *stream, int len)
{
    AudioBuffer* audio = reinterpret_cast<AudioBuffer*>(userdata);
    x86::sreg16* destData = reinterpret_cast<x86::sreg16*>(stream);
    NFS2_ASSERT((len/4) % s_sampleSize == 0);
    for (x86::reg32 chunk = 0; chunk < (len/4) / s_sampleSize; ++chunk)
    {
        x86::CPU cpu;
        cpu.edx = s_sampleCount;
        cpu.eax = audio->m_memmap->getBlockStart();
        cpu.esp = audio->m_memmap->getBlockStart() + audio->m_memmap->getBlockSize() - 4;
        audio->m_app->runThread(cpu, audio->m_bufferCallback);

        x86::sreg16* srcData = audio->m_buffer;
        destData[0] = srcData[0];
        destData[1] = srcData[1];
        destData[2] = (srcData[0] + srcData[2]) / 2;
        destData[3] = (srcData[1] + srcData[3]) / 2;
        for (x86::reg32 i = 1; i < s_sampleCount; i ++)
        {
            destData[i * 4 + 0] = srcData[i * 2];
            destData[i * 4 + 1] = srcData[i * 2 + 1];
            destData[i * 4 + 2] = (srcData[i * 2 - 2] + srcData[i * 2]) / 2;
            destData[i * 4 + 3] = (srcData[i * 2 - 1] + srcData[i * 2 + 1]) / 2;
        }
        destData += s_sampleSize * 2;
    }
    NFS2_ASSERT(reinterpret_cast<x86::reg8*>(destData) == stream + len);
    return;
}*/

x86::reg32 AudioBuffer::lock()
{
    return m_memmap->getBlockStart();
}

/* The game wrote `bufferWritten` more bytes after the last.  Written may run
 * more than the buffer's length ahead of played, and that is the game's normal
 * way in the menus: skipping the difference, as was tried on 2026-09-29, made
 * the menu's sound run fast and the movies, which keep time by it, run at
 * several times their speed. */
void AudioBuffer::unlock(x86::reg32 bufferWritten)
{
    SDL_AudioStream* stream = m_device->stream();
    if (stream)
        SDL_LockAudioStream(stream);
    m_playStop += bufferWritten;
    if (stream)
        SDL_UnlockAudioStream(stream);
}


}
