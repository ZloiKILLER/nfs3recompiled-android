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

void AudioDevice::play(AudioBuffer* buffer)
{
    if (m_playingBuffers.empty() && m_device)
        SDL_ResumeAudioDevice(m_device);
    m_playingBuffers.push_back(buffer);
}

void AudioDevice::stop(AudioBuffer* buffer)
{
    m_playingBuffers.erase(std::remove(m_playingBuffers.begin(), m_playingBuffers.end(), buffer), m_playingBuffers.end());
    if (m_playingBuffers.empty() && m_device)
        SDL_PauseAudioDevice(m_device);
}


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

void AudioBuffer::audioCallback22050(x86::reg8* stream, int len)
{
    x86::sreg16* destData = reinterpret_cast<x86::sreg16*>(stream);
    x86::reg32 start = m_playStart;
    x86::reg32 stop = start + len;
    if (stop > m_playStop)
    {
        stop = m_playStop;
        len = stop - start;
    }
    if (!len) return;
    x86::reg32 bufferStart = (start % m_bufferSize) / sizeof(x86::sreg16);
    x86::reg32 bufferStop = (stop % m_bufferSize) / sizeof(x86::sreg16);
    memset(destData, 0, len);
    if (bufferStop <= bufferStart)
    {
        for (x86::sreg16* data = m_buffer + bufferStart;
             data < m_buffer + m_bufferSize/2;
             ++data, ++destData)
            *destData += *data;
        for (x86::sreg16* data = m_buffer;
             data < m_buffer + bufferStop;
             ++data, ++destData)
            *destData += *data;
    }
    else
    {
        for (x86::sreg16* data = m_buffer + bufferStart;
             data < m_buffer + bufferStop;
             ++data, ++destData)
            *destData += *data;
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

void AudioBuffer::unlock(x86::reg32 bufferWritten)
{
    SDL_LockAudioStream(m_device->stream());
    m_playStop += bufferWritten;
    SDL_UnlockAudioStream(m_device->stream());
}


}
