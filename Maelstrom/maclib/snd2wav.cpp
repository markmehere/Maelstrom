/*
    SND2WAV:  A program to convert Macintosh (tm) sound resources
    Copyright (C) 1997-2021 Sam Lantinga <slouken@libsdl.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/* A Macintosh sound resource converter */

#include <stdlib.h>
#include <string.h>

#include "Mac_Wave.h"

static Wave wave;

int snd2wav(const char *path)
{
	Mac_Resource *macx;
	Mac_ResData  *snd;
	char wavname[128];
	Uint16 *ids, rate;
	int i;

	macx = new Mac_Resource(path);
	if ( macx->Error() ) {
		fprintf(stderr, "%s\n", macx->Error());
		delete macx;
		exit(255);
	}
	if ( macx->NumResources("snd ") == 0 ) {
		fprintf(stderr, "No sound resources in '%s'\n", path);
		delete macx;
		exit(1);
	}

	ids = macx->ResourceIDs("snd ");

	for ( i=0; ids[i] != 0xFFFF; ++i ) {
		snd = macx->Resource("snd ", ids[i]);
		if ( snd == NULL ) {
			fprintf(stderr, "Warning: %s\n", macx->Error());
			continue;
		}
		wave.Load(snd, rate);
		sprintf(wavname, "/tmp/Maelstrom_Snd#%d.wav", ids[i]);
		wave.Save(wavname);
	}
	delete macx;
    return 0;
}
