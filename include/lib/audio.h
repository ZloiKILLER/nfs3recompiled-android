#ifndef LIB_AUDIO_H_
#define LIB_AUDIO_H_
#include <lib/app.h>
#include <lib/memmap.h>
#include <SDL3/SDL.h>


namespace nfs
{

class AudioBuffer;

class AudioDevice : public GenericResource
{
public:
    AudioDevice();
    ~AudioDevice();

    void play(AudioBuffer* buffer);
    void stop(AudioBuffer* buffer);
    SDL_AudioStream* stream() const { return m_stream; }

private:
    static void audioCallback22050(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount);

private:
    SDL_AudioDeviceID           m_device;
    SDL_AudioStream*            m_stream;
    std::vector<AudioBuffer*>   m_playingBuffers;
};

class AudioBuffer : public GenericResource
{
    friend class AudioDevice;
public:
    AudioBuffer(AudioDevice* device, App* app, nfs::u32 bufferSize);
    ~AudioBuffer();

    nfs::u32 bufferSize() const;
    void play();
    void stop();
    nfs::u32 lock();
    void unlock(nfs::u32 bufferWritten);
    nfs::u32 playStart() const        { return m_playStart; }
    nfs::u32 playStop() const         { return m_playStop; }
    nfs::u32 getBackBuffer() const    { return m_bufferMap->getBlockStart(); }
    bool primary() const                { return m_primary; }

private:
    void audioCallback22050(nfs::u8* stream, int len);

private:
    AudioDevice*    m_device;
    nfs::u32      m_bufferSize;
    MemMap*         m_memmap;
    MemMap*         m_bufferMap;
    nfs::s16*    m_buffer;
    nfs::u32      m_playStart;
    nfs::u32      m_playStop;
    bool            m_primary;
};

}

#endif /*!LIB_AUDIO_H_*/
