/*
 * Copyright 2008, 2009, Dominik Geyer
 *
 * This file is part of HoldingNuts.
 *
 * HoldingNuts is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HoldingNuts is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HoldingNuts.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *     Dominik Geyer <dominik.geyer@holdingnuts.net>
 */


/* this is the most portable way to include SDL includes (see SDL FAQ) */
#include "SDL.h"
#include "SDL_audio.h"

#include "Config.h"
#include "Debug.h"
#include "Logger.h"

#include "Audio.h"


#define FMT_FREQ	44100
#define FMT_FORMAT	AUDIO_S16
#define FMT_CHANNELS	2	/* 1=mono, 2=stereo */
#define FMT_SAMPLES	4192

#define NUM_SOUNDS	2
#define NUM_SLOTS	64

struct sample {
	Uint8 *data;
	Uint32 dpos;
	Uint32 dlen;
} sounds[NUM_SOUNDS];

struct sndslot {
	SDL_AudioCVT cvt;
} slots[NUM_SLOTS];


/* callback-function prototype */
void audio_fill(void *unused, Uint8 *stream, int len);


int audio_init()
{
	SDL_AudioSpec fmt;

	if (SDL_Init(SDL_INIT_AUDIO) < 0)
	{
		log_msg("audio", "SDL_Init() failed: %s", SDL_GetError());
		return 1;
	}
	
	fmt.freq = FMT_FREQ;
	fmt.format = FMT_FORMAT;
	fmt.channels = FMT_CHANNELS;
	fmt.samples = FMT_SAMPLES;
	fmt.callback = audio_fill;
	fmt.userdata = NULL;
	
	if (SDL_OpenAudio(&fmt, NULL) < 0)
	{
		log_msg("audio", "SDL_OpenAudio() failed: %s", SDL_GetError());
		return 1;
	}
	
	SDL_PauseAudio(0);
	
	return 0;
}

int audio_deinit()
{
	/* FIXME: free allocated sound-data */
	
	SDL_CloseAudio();
	SDL_Quit();
	
	return 0;
}

int audio_load(unsigned int id, const char *file)
{
	SDL_AudioSpec wave;
	Uint8 *data;
	Uint32 dlen;
	SDL_AudioCVT *cvt = &(slots[id].cvt);
	
	/* load the sound file */
	if (SDL_LoadWAV(file, &wave, &data, &dlen) == NULL)
	{
		log_msg("audio", "SDL_LoadWAV() failed: %s", SDL_GetError());
		return 1;
	}
	
#if 0
	log_msg("audio_load", "file '%s': format:%d channels:%d freq:%d", file, wave.format, wave.channels, wave.freq);
#endif
	
	/* convert the data */
	if (SDL_BuildAudioCVT(cvt, wave.format, wave.channels, wave.freq, FMT_FORMAT, FMT_CHANNELS, FMT_FREQ) < 0)
	{
		log_msg("audio", "SDL_BuildAudioCVT() failed: %s", SDL_GetError());
		return 2;
	}
	
	
	cvt->len = dlen;
	cvt->buf = malloc(cvt->len * cvt->len_mult);
	memcpy(cvt->buf, data, cvt->len);
	
	SDL_FreeWAV(data);
	
	
	if (SDL_ConvertAudio(cvt) == -1)
	{
		log_msg("audio", "SDL_ConvertAudio() failed: %s", SDL_GetError());
		return 3;
	}
	
	return 0;
}

void audio_play(unsigned int id)
{
	int index;
	SDL_AudioCVT *cvt = &(slots[id].cvt);
	
	/* Look for an empty (or finished) sound slot */
	for (index=0; index < NUM_SOUNDS; ++index)
	{
		if (sounds[index].dpos == sounds[index].dlen)
			break;
	}
	
	if (index == NUM_SOUNDS)
		return;
	
	/* put the sound data in the slot (it starts playing immediately) */
	SDL_LockAudio();
	sounds[index].data = cvt->buf;
	sounds[index].dlen = cvt->len_cvt;
	sounds[index].dpos = 0;
	SDL_UnlockAudio();
}

/* callback function */
void audio_fill(void *unused, Uint8 *stream, int len)
{
	int i;
	Uint32 amount;
	
	for (i=0; i < NUM_SOUNDS; ++i)
	{
		amount = (sounds[i].dlen-sounds[i].dpos);
		if ( amount > len )
			amount = len;

		SDL_MixAudio(stream, &sounds[i].data[sounds[i].dpos], amount, SDL_MIX_MAXVOLUME);
		sounds[i].dpos += amount;
	}
}
