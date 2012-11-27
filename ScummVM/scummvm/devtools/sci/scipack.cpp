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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sciresource.h>
#include <resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

#define COPY_BLOCK_SIZE 512

unsigned short *resource_ids = NULL;

void
help() {
	printf("Usage:\n\tscipack <file_0> ... <file_n>\n"
	       "\nBuilds an uncompressed SCI0 resource.000 and a resource.map\n");
}

int /* Returns resource ID on success, -1 on error */
test_file(char *filename) {
	char *dot = strchr(filename, '.');
	char *endptr;
	FILE *f;
	int res_type, res_index;

	if (!dot) {
		fprintf(stderr, "Must contain a period");
		return -1;
	}

	*dot = 0;

	for (res_type = 0; res_type < sci_invalid_resource
	        && scumm_stricmp(filename, sci_resource_types[res_type]); res_type++);

	*dot = '.';

	if (res_type == sci_invalid_resource) {
		fprintf(stderr, "Invalid resource type");
		return -1;
	}

	++dot;
	res_index = strtol(dot, &endptr, 10);

	if (!*dot || *endptr) {
		fprintf(stderr, "Invalid resource index");
		return -1;
	}

	if (res_index < 0) {
		fprintf(stderr, "Negative resource index");
		return -1;
	}

	if (res_index >= 1000) {
		fprintf(stderr, "Resource index too large");
		return -1;
	}

	f = fopen(filename, "r");
	if (!f) {
		perror("While asserting file");
		return -1;
	}
	fclose(f);

	return (res_type << 11) | res_index;
}

int
build_file_ids(int count, char **names) {
	int i;
	int error = 0;

	resource_ids = (unsigned short*) malloc(sizeof(unsigned short) * count);

	for (i = 0; i < count; i++) {
		int id = test_file(names[i]);
		if (id < 0) {
			error = -1;
			fprintf(stderr, ": %s\n", names[i]);
		} else resource_ids[i] = id;
	}

	return error;
}


static inline void
write_uint16(int fd, unsigned int uint) {
	unsigned char upper = (uint >> 8) & 0xff;
	unsigned char lower = (uint) & 0xff;

	if ((write(fd, &upper, 1) < 1)
	        || (write(fd, &lower, 1) < 1)) {
		perror("While writing");
		exit(1);
	}
}

int
write_files(int count, char **names) {
	int resource_000, resource_map;
	int i;

	resource_000 = creat("resource.000", 0644);
	if (resource_000 < 0) {
		perror("While creating 'resource.000'");
		return -1;
	}

	resource_map = creat("resource.map", 0644);
	if (resource_map < 0) {
		perror("While creating 'resource.map'");
		return -1;
	}

	for (i = 0; i < count; i++) {
		int fd = open(names[i], O_RDONLY);
		struct stat fdstat;
		int fdsize;
		unsigned char buf[512];
		int j;
		long offset = lseek(resource_000, SEEK_CUR, 0);
		int top_offset = (offset >> 16) & 0xffff;
		int bot_offset = offset & 0xffff;

		if (fd < 0) {
			perror(names[i]);
			return -1;
		}
		fstat(fd, &fdstat);
		fdsize = fdstat.st_size;

		write_uint16(resource_000, resource_ids[i]);
		write_uint16(resource_000, fdsize);
		write_uint16(resource_000, fdsize);
		write_uint16(resource_000, 0);

		do {
			j = read(fd, buf, COPY_BLOCK_SIZE);
			write(resource_000, buf, j);
		} while (j == COPY_BLOCK_SIZE);
		close(fd);

		write_uint16(resource_map, resource_ids[i]);
		write_uint16(resource_map, bot_offset);
		write_uint16(resource_map, top_offset);
	}

	/* Terminate resource 000 */
	write_uint16(resource_000, 0);

	/* Terminate resource map */
	write_uint16(resource_map, 0xffff);
	write_uint16(resource_map, 0xffff);

	close(resource_000);
	close(resource_map);
}


int
main(int argc, char **argv) {
	printf("scipack.c Copyright (C) 2002 Christoph Reichenbach\n"
	       "This program is FREE SOFTWARE. You may copy it and/or re-distribute it\n"
	       "according to the terms of the GNU General Public License. See LICENSING\n"
	       "for details.\n");

	if (argc < 1)
		help();

	if (build_file_ids(argc - 1, argv + 1))
		return -1;

	if (write_files(argc - 1, argv + 1))
		return -1;
	free(resource_ids);
}
