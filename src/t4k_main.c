/*
   t4k_main.c
   Functions used to initialize the t4k_common library

   Copyright 2009, 2010.
Authors: Boleslaw Kulbabinski, Brendan Luchen
Project email: <tuxmath-devel@lists.sourceforge.net>
Project website: http://tux4kids.alioth.debian.org

t4k_main.c is part of the t4k_common library.

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



#include <stdio.h>
#include <stdlib.h>
#include "t4k_common.h"
#include "t4k_globals.h"

#if defined(HAVE_LIBSDL_NET) && HAVE_LIBSDL_NET
#include <SDL3_net/SDL_net.h>
#endif

int debug_status;


/* set global variables */
/* TODO look into support for locale switching at runtime
 ** http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=490115 
 */
int InitT4KCommon(int debug_flags)
{
    fprintf(stderr, "Initializing " PACKAGE_STRING "\n");

    /* Video: */
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
	fprintf(stderr,
		"\nError: I could not initialize video!\n"
		"The Simple DirectMedia error that occured was:\n"
		"%s\n\n", SDL_GetError());
	return 0;
    }

    /* Audio: */
    if (SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        fprintf(stderr,
	    "\nWarning: I could not initialize audio!\n"
	    "The Simple DirectMedia error that occured was:\n"
	    "%s\n\n", SDL_GetError());
    }

    /* TTS */
    if(T4K_Tts_init())
    {
        fprintf(stderr,"\nWarning: I could not initialize Tts!\n");
    }
    
    /* Text (either SDL_ttf or SDL_Pango): */
    if (!T4K_Setup_SDL_Text())
    {
	fprintf( stderr, "Couldn't initialize text (SDL_ttf or SDL_Pango)\n");
	return 0;
    }

#if defined(HAVE_LIBSDL_NET) && HAVE_LIBSDL_NET
    /* Networking: */
    if (!SDLNet_Init())
    {
        fprintf(stderr, "SDLNet_Init: %s\n", SDL_GetError());
	return 0;
    }
#endif

    /* Seed random-number generator: */
    srand(SDL_GetTicks());

    debug_status = debug_flags;
    T4K_InitBlitQueue();
    return 1;
}

void CleanupT4KCommon(void)
{
    // Close the audio mixer.
    Mix_CloseAudio();
    
    T4K_UnloadMenus();
    // Unload SDL_Pango or SDL_ttf:
    T4K_Cleanup_SDL_Text();
    
#if defined(HAVE_LIBSDL_NET) && HAVE_LIBSDL_NET
    /* Quit networking if appropriate: */
    SDLNet_Quit();
#endif

    // Finally, quit SDL
    SDL_Quit();
}


int T4K_HandleStdEvents (const SDL_Event* event)
{
    int ret = 0;

    if (event->type != SDL_EVENT_KEY_DOWN)
	return 0;

    SDL_Keycode key = event->key.key;

    /* Toggle screen mode: */
    if (key == SDLK_F10)
    {
	//    Opts_SetGlobalOpt(FULLSCREEN, !Opts_GetGlobalOpt(FULLSCREEN) );
	T4K_SwitchScreenMode();
	//    game_recalc_positions();
	ret = 1;
    }

    /* Toggle music: */
#ifndef NOSOUND
    else if (key == SDLK_F11)
    {
	T4K_AudioToggle();
    }
#endif

    return ret;
}
