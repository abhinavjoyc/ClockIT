// audio_system.cpp
#include "audio.h"
#include <iostream>

bool Audio_Init(int frequency,
    Uint16 format,
    int channels,
    int chunkSize,
    int numSfxChannels)
{
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0) {
        std::cout << "SDL_Init error: " << SDL_GetError() << "\n";
        return false;
    }

    // Optional: Mix_Init if you use MP3/OGG/FLAC; omitted for brevity. [web:24][web:40]

    if (Mix_OpenAudio(frequency, format, channels, chunkSize) < 0) {
        std::cout << "Mix_OpenAudio error: " << Mix_GetError() << "\n";
        return false;
    }

    Mix_AllocateChannels(numSfxChannels); // allow multiple simultaneous SFX [web:25][web:41]
    return true;
}

void Audio_Shutdown()
{
    Mix_CloseAudio(); // closes audio device [web:17][web:19]
    SDL_QuitSubSystem(SDL_INIT_AUDIO | SDL_INIT_TIMER);
    // If you used SDL_Init only for audio, you can call SDL_Quit() instead. [web:17][web:19]
}

Mix_Chunk* Audio_LoadSfx(const std::string& path)
{
    Mix_Chunk* c = Mix_LoadWAV(path.c_str()); // loads WAV/OGG/etc. [web:17][web:19]
    if (!c) {
        std::cout << "Mix_LoadWAV error (" << path << "): "
            << Mix_GetError() << "\n";
    }
    return c;
}

void Audio_FreeSfx(Mix_Chunk* sfx)
{
    if (sfx) {
        Mix_FreeChunk(sfx); // free sample data [web:17]
    }
}

int Audio_PlayOnce(Mix_Chunk* sfx)
{
    if (!sfx) return -1;
    // -1 = pick first free channel; 0 loops = play once. [web:2][web:21][web:25]
    int ch = Mix_PlayChannel(-1, sfx, 0);
    return ch;
}

int Audio_PlayLoop(Mix_Chunk* sfx)
{
    if (!sfx) return -1;
    // loops = -1 means infinite loop. [web:2][web:21][web:25]
    int ch = Mix_PlayChannel(-1, sfx, -1);
    return ch;
}

void Audio_PauseChannel(int ch)
{
    if (ch >= 0) {
        Mix_Pause(ch); // pause specific channel [web:27]
    }
}

void Audio_ResumeChannel(int ch)
{
    if (ch >= 0) {
        Mix_Resume(ch); // resume specific channel [web:27]
    }
}

void Audio_StopChannel(int ch)
{
    if (ch >= 0) {
        Mix_HaltChannel(ch); // stop specific channel immediately [web:21][web:25]
    }
}

void Audio_PauseAll()
{
    Mix_Pause(-1); // -1 = all channels [web:27]
}

void Audio_ResumeAll()
{
    Mix_Resume(-1); // -1 = all channels [web:27]
}

void Audio_StopAll()
{
    Mix_HaltChannel(-1); // -1 = all channels [web:21][web:25]
}

// Simple blocking timed playback; for real engine, integrate with your own timing.
int Audio_PlayTimedMs(Mix_Chunk* sfx, Uint32 ms)
{
    if (!sfx) return -1;

    int ch = Mix_PlayChannel(-1, sfx, -1); // loop; we will stop it ourselves [web:25]
    if (ch == -1) return -1;

    Uint32 start = SDL_GetTicks();
    while (SDL_GetTicks() - start < ms) {
        SDL_Delay(1);
    }

    Mix_HaltChannel(ch); // stop after ms [web:21]
    return ch;
}
// Mute all chunks (not music)
void Audio_MuteAll()
{
    Mix_MasterVolume(0);                // 0 = silence [web:82]
}

// Restore all chunks to full volume
void Audio_UnmuteAll()
{
    Mix_MasterVolume(MIX_MAX_VOLUME);   // 128 = full volume [web:82][web:64]
}
void Audio_SetChannelVolume(int ch, int vol) // vol 0..MIX_MAX_VOLUME
{
    if (ch >= 0) Mix_Volume(ch, vol); //[web:89] 
}
void Audio_SetAllChannelVolume(int vol) // vol: 0 .. MIX_MAX_VOLUME
{
    Mix_Volume(-1, vol); // -1 = all channels
}
