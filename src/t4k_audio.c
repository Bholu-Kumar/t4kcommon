/*
   t4k_audio.c:

   Audio-related functions.

   Copyright 2003, 2006, 2009, 2010.
Authors: Sam Hart, Jesse Andrews, David Bruce, Brendan Luchen
Project email: <tuxmath-devel@lists.sourceforge.net>
Project website: http://tux4kids.alioth.debian.org

t4k_audio.c is part of the t4k_common library.

t4k_common is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

t4k_common is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.  */



#include "t4k_common.h"
#include "t4k_globals.h"
#include <stdio.h>

static bool audio_enabled = true;
static int music_loops = 0;
static Mix_Music *default_music = NULL;

const char* MUSIC_DIR = "sounds";

void T4K_AudioMusicPlay(Mix_Music *musicData, int loops);

static MIX_Mixer *main_mixer = NULL;

MIX_Mixer* T4K_GetAudioMixer(void)
{
    if (!main_mixer)
    {
        main_mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
    }
    return main_mixer;
}

// play sound once and exit
void T4K_PlaySound(Mix_Chunk* sound)
{
    T4K_PlaySoundLoop(sound, 0);
}

// play sound "loops" times, -1 for infinite
void T4K_PlaySoundLoop(Mix_Chunk* sound, int loops)
{
    (void)loops;
    if(sound && audio_enabled)
	MIX_PlayAudio(T4K_GetAudioMixer(), sound);
}

void T4K_AudioHaltChannel( int channel )
{
    (void)channel;
}

/* audioMusicLoad attempts to load and play the music file
 * Note: loops == -1 means forever
 */
void T4K_AudioMusicLoad(char* music_path, int loops)
{
    if (audio_enabled)
    {
	default_music = T4K_LoadMusic(music_path);
	T4K_AudioMusicPlay(default_music, loops);
    }
}

/* audioMusicUnload attempts to unload any music data that was
 * loaded using the audioMusicLoad function
 */
void T4K_AudioMusicUnload()
{
    if(default_music)
	MIX_DestroyAudio(default_music);
    default_music = NULL;
}

bool T4K_IsPlayingMusic()
{
    return (default_music != NULL);
}

/* audioMusicPlay attempts to play the passed music data.
 * if a music file was loaded using the audioMusicLoad
 * it will be stopped and unloaded
 * Note: loops == -1 means forever
 */
void T4K_AudioMusicPlay(Mix_Music *musicData, int loops)
{
    if (musicData != default_music)
    {
	T4K_AudioMusicUnload(); //FIXME this feels buggy...
    }
    music_loops = loops;
    if (audio_enabled && musicData)
	MIX_PlayAudio(T4K_GetAudioMixer(), musicData);
}

void T4K_AudioEnable(bool enabled)
{
    audio_enabled = enabled;
}

void T4K_AudioToggle()
{
    T4K_AudioEnable(!audio_enabled);   
}

/* Note: SDL3_mixer master volume affects both music and sound effects together */
void T4K_AudioSetGlobalVolume(float gain)
{
    MIX_SetMixerGain(T4K_GetAudioMixer(), gain);
}

float T4K_AudioGetGlobalVolume(void)
{
    return MIX_GetMixerGain(T4K_GetAudioMixer());
}
