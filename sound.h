#pragma once

#include "include/SDL2/SDL.h"
#include "include/SDL2/SDL_mixer.h"

#define N_SFX 6
typedef enum { SFX_BASKET, SFX_CLUCK, SFX_FALL, SFX_PLOP, SFX_SHIT, SFX_WIND } SFXType;
Mix_Chunk* SFX[N_SFX];
Mix_Music* BGMusic;

void loadSounds()
{
    SDL_Init(SDL_INIT_AUDIO);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    BGMusic         = Mix_LoadMUS("assets/sounds/happy.mp3");
    SFX[SFX_BASKET] = Mix_LoadWAV("assets/sounds/basketthud.wav");
    SFX[SFX_CLUCK]  = Mix_LoadWAV("assets/sounds/cluck.wav");
    SFX[SFX_FALL]   = Mix_LoadWAV("assets/sounds/fall.wav");
    SFX[SFX_PLOP]   = Mix_LoadWAV("assets/sounds/plop.wav");
    SFX[SFX_SHIT]   = Mix_LoadWAV("assets/sounds/shit.wav");
    SFX[SFX_WIND]   = Mix_LoadWAV("assets/sounds/wind.wav");
    Mix_Volume(-1, 25);
    Mix_VolumeMusic(10);
    Mix_PlayMusic(BGMusic, -1);
}

void playSFX(SFXType type) { Mix_PlayChannel(-1, SFX[type], 0); }

void freeSounds()
{
    Mix_FreeMusic(BGMusic);
    Mix_CloseAudio();
    SDL_Quit();
}
