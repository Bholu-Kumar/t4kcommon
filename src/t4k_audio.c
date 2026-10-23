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
static float mus_volume = 1.0f;   /* independent music gain (0.0 – 1.0) */
static float sfx_volume = 1.0f;   /* independent sfx gain   (0.0 – 1.0) */

#define NUM_SFX_TRACKS 16
static MIX_Track *sfx_tracks[NUM_SFX_TRACKS] = {NULL};
static int next_sfx_track = 0;
static MIX_Track *music_track = NULL;

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
    if (sound && audio_enabled)
    {
        MIX_Mixer *mixer = T4K_GetAudioMixer();
        if (!mixer) return;

        MIX_Track *trk = sfx_tracks[next_sfx_track];
        if (!trk)
        {
            trk = MIX_CreateTrack(mixer);
            sfx_tracks[next_sfx_track] = trk;
        }

        if (trk)
        {
            MIX_SetTrackAudio(trk, sound);
            MIX_SetTrackGain(trk, sfx_volume);
            if (loops != 0)
            {
                SDL_PropertiesID props = SDL_CreateProperties();
                SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, (loops == -1) ? -1 : loops);
                MIX_PlayTrack(trk, props);
                SDL_DestroyProperties(props);
            }
            else
            {
                MIX_PlayTrack(trk, 0);
            }
            next_sfx_track = (next_sfx_track + 1) % NUM_SFX_TRACKS;
        }
        else
        {
            MIX_PlayAudio(mixer, sound);
        }
    }
}

void T4K_AudioHaltChannel( int channel )
{
    (void)channel;
    if (main_mixer)
        MIX_StopAllTracks(main_mixer, 0);
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
void T4K_AudioMusicUnload(void)
{
    if (music_track)
    {
        MIX_StopTrack(music_track, 0);
        MIX_SetTrackAudio(music_track, NULL);
    }
    if (default_music)
	MIX_DestroyAudio(default_music);
    default_music = NULL;
}

bool T4K_IsPlayingMusic(void)
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
    {
        MIX_Mixer *mixer = T4K_GetAudioMixer();
        if (!mixer) return;

        if (!music_track)
        {
            music_track = MIX_CreateTrack(mixer);
        }

        if (music_track)
        {
            MIX_SetTrackAudio(music_track, musicData);
            MIX_SetTrackGain(music_track, mus_volume);
            if (loops != 0)
            {
                SDL_PropertiesID props = SDL_CreateProperties();
                SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, (loops == -1) ? -1 : loops);
                MIX_PlayTrack(music_track, props);
                SDL_DestroyProperties(props);
            }
            else
            {
                MIX_PlayTrack(music_track, 0);
            }
        }
        else
        {
            MIX_PlayAudio(mixer, musicData);
        }
    }
}

void T4K_AudioMusicPause(void)
{
    if (music_track)
        MIX_PauseTrack(music_track);
}

void T4K_AudioMusicResume(void)
{
    if (music_track)
        MIX_ResumeTrack(music_track);
}

void T4K_AudioEnable(bool enabled)
{
    audio_enabled = enabled;
}

void T4K_AudioToggle(void)
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

void T4K_AudioSetMusicVolume(float gain)
{
    if (gain < 0.0f) gain = 0.0f;
    if (gain > 1.0f) gain = 1.0f;
    mus_volume = gain;
    if (music_track)
        MIX_SetTrackGain(music_track, mus_volume);
}

float T4K_AudioGetMusicVolume(void)
{
    return mus_volume;
}

void T4K_AudioSetSfxVolume(float gain)
{
    if (gain < 0.0f) gain = 0.0f;
    if (gain > 1.0f) gain = 1.0f;
    sfx_volume = gain;
    for (int i = 0; i < NUM_SFX_TRACKS; i++)
    {
        if (sfx_tracks[i])
            MIX_SetTrackGain(sfx_tracks[i], sfx_volume);
    }
}

float T4K_AudioGetSfxVolume(void)
{
    return sfx_volume;
}

void T4K_AudioCleanup(void)
{
    T4K_AudioMusicUnload();
    if (main_mixer)
    {
        MIX_StopAllTracks(main_mixer, 0);
        for (int i = 0; i < NUM_SFX_TRACKS; i++)
        {
            if (sfx_tracks[i])
            {
                MIX_SetTrackAudio(sfx_tracks[i], NULL);
                MIX_DestroyTrack(sfx_tracks[i]);
                sfx_tracks[i] = NULL;
            }
        }
        if (music_track)
        {
            MIX_SetTrackAudio(music_track, NULL);
            MIX_DestroyTrack(music_track);
            music_track = NULL;
        }
        MIX_DestroyMixer(main_mixer);
        main_mixer = NULL;
    }
}

