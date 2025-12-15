// audio_system.h
#pragma once
#include <SDL.h>
#include <SDL_mixer.h>
#include <string>

// Call once at program start
bool Audio_Init(int frequency = 44100,
    Uint16 format = MIX_DEFAULT_FORMAT,
    int channels = 2,
    int chunkSize = 2048,
    int numSfxChannels = 16);

// Call once at program exit
void Audio_Shutdown();

// Load / free sound effect
Mix_Chunk* Audio_LoadSfx(const std::string& path);
void Audio_FreeSfx(Mix_Chunk* sfx);

// Basic playback
int Audio_PlayOnce(Mix_Chunk* sfx);          // returns channel
int Audio_PlayLoop(Mix_Chunk* sfx);          // returns channel, loops forever

// Control specific channel
void Audio_PauseChannel(int ch);
void Audio_ResumeChannel(int ch);
void Audio_StopChannel(int ch);

// Global control
void Audio_PauseAll();
void Audio_ResumeAll();
void Audio_StopAll();
void Audio_MuteAll();
void Audio_UnmuteAll();
void Audio_SetChannelVolume(int ch, int vol);


// Timed playback (simple, blocking helper)
int Audio_PlayTimedMs(Mix_Chunk* sfx, Uint32 ms); // plays, waits, stops
