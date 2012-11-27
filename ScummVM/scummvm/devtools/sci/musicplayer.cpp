/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <sfx_iterator_internal.h>
#include <sfx_engine.h>
#include <sfx_core.h>
#include <resource.h>

#define DUMMY_SOUND_HANDLE 0xdeadbeef

static song_iterator_t *
build_iterator(resource_mgr_t *resmgr, int song_nr, int type, songit_id_t id) {
	resource_t *song = scir_find_resource(resmgr, sci_sound, song_nr, 0);

	if (!song)
		return NULL;

	return songit_new(song->data, song->size, type, id);
}

int main(int argc, char** argv) {
	resource_mgr_t *resmgr;
	sfx_state_t sound;
	int res_version = SCI_VERSION_AUTODETECT;
	int sound_nr;
	song_handle_t dummy1;
	int dummy2;
	int arg;
	int it_type = SCI_SONG_ITERATOR_TYPE_SCI0;
	song_iterator_t *base, *ff;

	printf("FreeSCI %s music player Copyright (C) 1999-2007\n", VERSION);
	printf(" Dmitry Jemerov, Christopher T. Lansdown, Sergey Lapin, Rickard Lind,\n"
	       " Carl Muckenhoupt, Christoph Reichenbach, Magnus Reftel, Lars Skovlund,\n"
	       " Rink Springer, Petr Vyhnak, Solomon Peachy, Matt Hargett, Alex Angas\n"
	       " Walter van Niftrik, Rainer Canavan, Ruediger Hanke, Hugues Valois\n"
	       "This program is free software. You can copy and/or modify it freely\n"
	       "according to the terms of the GNU general public license, v2.0\n"
	       "or any later version, at your option.\n"
	       "It comes with ABSOLUTELY NO WARRANTY.\n");
	if (argc < 3) {
		fprintf(stderr, "Syntax: %s <resource dir> <sound number> [<sound number> ...]\n", argv[0]);
		return 1;
	}

	if (!(resmgr = scir_new_resource_manager(argv[1], res_version,
	               0, 1024 * 128))) {
		fprintf(stderr, "Could not find any resources; quitting.\n");
		return 2;
	}

	if (resmgr->sci_version >= SCI_VERSION_01)
		it_type = SCI_SONG_ITERATOR_TYPE_SCI1;

	sfx_init(&sound, resmgr, 0);
	sfx_set_volume(&sound, 127);

	arg = 2 - 1;
	while (++arg < argc) {
		sound_nr = atoi(argv[arg]);
		base = ff = build_iterator(resmgr, sound_nr, it_type,
		                           DUMMY_SOUND_HANDLE);
		printf("Playing resource %d...\n", sound_nr);
		if (sfx_add_song(&sound, ff,
		                 0, DUMMY_SOUND_HANDLE, sound_nr)) {
			fprintf(stderr, "Could not start sound resource. Does it exist?\n");
			return 2;
		}
		sfx_song_set_status(&sound, DUMMY_SOUND_HANDLE, SOUND_STATUS_PLAYING);
		while (sfx_poll(&sound, &dummy1, &dummy2) != SI_FINISHED) {}
	}
	sfx_exit(&sound);
	scir_free_resource_manager(resmgr);
	return 0;
}
