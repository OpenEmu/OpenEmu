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
#  include <config.h>
#endif /* HAVE_CONFIG_H */

#define MALLOC_DEBUG

#include <sciresource.h>
#include <engine.h>
#include <console.h>
#include <versions.h>

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif /* HAVE_GETOPT_H */

static int hexdump = 0;
static int opcode_size = 0;
static int verbose = 0;

static resource_mgr_t *resmgr;

#ifdef HAVE_GETOPT_LONG
static struct option options[] = {
	{"version", no_argument, 0, 256},
	{"help", no_argument, 0, 'h'},
	{"hexdump", no_argument, &hexdump, 1},
	{"opcode-size", no_argument, &opcode_size, 1},
	{"verbose", no_argument, &verbose, 1},
	{"gamedir", required_argument, 0, 'd'},
	{0, 0, 0, 0}
};
#endif /* HAVE_GETOPT_LONG */

#define SCI_ASSUME_VERSION SCI_VERSION_FTU_NEW_SCRIPT_HEADER

typedef struct name_s {
	int offset;
	char *name;
	int class_no;
	struct name_s *next;
} name_t;

typedef struct area_s {
	int start_offset;
	int end_offset;
	void *data;
	struct area_s *next;
} area_t;

enum area_type { area_said, area_string, area_object, area_last };

typedef struct script_state_s {
	int script_no;
	name_t *names;
	area_t *areas [area_last];

	struct script_state_s *next;
} script_state_t;

typedef struct disasm_state_s {
	char **snames;
	int selector_count;
	opcode *opcodes;
	int kernel_names_nr;
	char **kernel_names;
	word_t **words;
	int word_count;

	char **class_names;
	int *class_selector_count;
	short **class_selectors;
	int class_count;
	int old_header;

	script_state_t *scripts;
} disasm_state_t;

void
disassemble_script(disasm_state_t *d, int res_no, int pass_no);

script_state_t *
find_script_state(disasm_state_t *d, int script_no);

void
script_free_names(script_state_t *s);

void
script_add_name(script_state_t *s, int aoffset, char *aname, int aclass_no);

char *
script_find_name(script_state_t *s, int offset, int *class_no);

void
script_add_area(script_state_t *s, int start_offset, int end_offset, int type, void *data);

void
script_free_areas(script_state_t *s);

int
script_get_area_type(script_state_t *s, int offset, void **pdata);

void
disasm_init(disasm_state_t *d);

void
disasm_free_state(disasm_state_t *d);

int main(int argc, char** argv) {
	int i;
	char outfilename [256];
	int optindex = 0;
	int c;
	disasm_state_t disasm_state;
	char *gamedir = NULL;
	int res_version = SCI_VERSION_AUTODETECT;

#ifdef HAVE_GETOPT_LONG
	while ((c = getopt_long(argc, argv, "vhxr:d:", options, &optindex)) > -1) {
#else /* !HAVE_GETOPT_H */
	while ((c = getopt(argc, argv, "vhxr:d:")) > -1) {
#endif /* !HAVE_GETOPT_H */

		switch (c) {
		case 256:
			printf("scidisasm ("PACKAGE") "VERSION"\n");
			printf("This program is copyright (C) 1999 Christoph Reichenbach.\n"
			       "It comes WITHOUT WARRANTY of any kind.\n"
			       "This is free software, released under the GNU General Public License.\n");
			exit(0);

		case 'h':
			printf("Usage: scidisasm\n"
			       "\nAvailable options:\n"
			       " --version               Prints the version number\n"
			       " --help        -h        Displays this help message\n"
			       " --gamedir <dir> -d<dir> Read game resources from dir\n"
			       " --hexdump     -x        Hex dump all script resources\n"
			       " --verbose               Print additional disassembly information\n"
			       " --opcode-size           Print opcode size postfixes\n");
			exit(0);

		case 'd':
			if (gamedir) sci_free(gamedir);
			gamedir = sci_strdup(optarg);
			break;

		case 'r':
			res_version = atoi(optarg);
			break;

		case 0: /* getopt_long already did this for us */
		case '?':
			/* getopt_long already printed an error message. */
			break;

		default:
			return -1;
		}
	}

	if (gamedir)
		if (chdir(gamedir)) {
			printf("Error changing to game directory '%s'\n", gamedir);
			exit(1);
		}

	printf("Loading resources...\n");
	if (!(resmgr = scir_new_resource_manager(sci_getcwd(), res_version,
	               1, 1024 * 128))) {
		fprintf(stderr, "Could not find any resources; quitting.\n");
		exit(1);
	}

	disasm_init(&disasm_state);

	script_adjust_opcode_formats(resmgr->sci_version);

	printf("Performing first pass...\n");
	for (i = 0; i < resmgr->resources_nr; i++)
		if (resmgr->resources[i].type == sci_script)
			disassemble_script(&disasm_state,
			                   resmgr->resources[i].number, 1);

	printf("Performing second pass...\n");
	for (i = 0; i < resmgr->resources_nr; i++)
		if (resmgr->resources[i].type == sci_script) {
			sprintf(outfilename, "%03d.script",
			        resmgr->resources[i].number);
			open_console_file(outfilename);
			disassemble_script(&disasm_state,
			                   resmgr->resources[i].number, 2);
		}

	close_console_file();
	disasm_free_state(&disasm_state);

	free(resmgr->resource_path);
	scir_free_resource_manager(resmgr);
	return 0;
}

/* -- General operations on disasm_state_t -------------------------------  */

void
disasm_init(disasm_state_t *d) {
	d->snames = vocabulary_get_snames(resmgr, &d->selector_count, SCI_ASSUME_VERSION);
	d->opcodes = vocabulary_get_opcodes(resmgr);
	d->kernel_names = vocabulary_get_knames(resmgr, &d->kernel_names_nr);
	d->words = vocab_get_words(resmgr, &d->word_count);
	d->scripts = NULL;
	d->old_header = 0;

	d->class_count = vocabulary_get_class_count(resmgr);
	d->class_names = (char **) sci_malloc(d->class_count * sizeof(char *));
	memset(d->class_names, 0, d->class_count * sizeof(char *));
	d->class_selector_count = (int *) sci_malloc(d->class_count * sizeof(int));
	memset(d->class_selector_count, 0, d->class_count * sizeof(int));
	d->class_selectors = (short **) sci_malloc(d->class_count * sizeof(short *));
	memset(d->class_selectors, 0, d->class_count * sizeof(short *));
}

void
disasm_free_state(disasm_state_t *d) {
	script_state_t *s, *next_script;
	int i;

	s = d->scripts;
	while (s) {
		next_script = s->next;
		script_free_names(s);
		script_free_areas(s);
		s = next_script;
	}

	for (i = 0; i < d->class_count; i++) {
		if (d->class_names [i]) sci_free(d->class_names [i]);
		if (d->class_selectors [i]) sci_free(d->class_selectors [i]);
	}

	free(d->class_names);
	free(d->class_selectors);
	free(d->class_selector_count);

	vocabulary_free_snames(d->snames);
	vocabulary_free_opcodes(d->opcodes);
	vocabulary_free_knames(d->kernel_names);
	vocab_free_words(d->words, d->word_count);
}

script_state_t *
find_script_state(disasm_state_t *d, int script_no) {
	script_state_t *s;

	for (s = d->scripts; s; s = s->next)
		if (s->script_no == script_no) return s;

	s = (script_state_t *) sci_malloc(sizeof(script_state_t));
	memset(s, 0, sizeof(script_state_t));
	s->script_no = script_no;
	s->next = d->scripts;

	d->scripts = s;
	return s;
}

/* -- Name table operations ----------------------------------------------  */

void
script_free_names(script_state_t *s) {
	name_t *p = s->names, *next_name;

	while (p) {
		next_name = p->next;
		free(p->name);
		free(p);
		p = next_name;
	}

	s->names = NULL;
}

void
script_add_name(script_state_t *s, int aoffset, char *aname, int aclass_no) {
	name_t *p;
	char *name = script_find_name(s, aoffset, NULL);
	if (name) return;

	p = (name_t *) sci_malloc(sizeof(name_t));
	p->offset = aoffset;
	p->name = sci_strdup(aname);
	p->class_no = aclass_no;
	p->next = s->names;
	s->names = p;
}

char *
script_find_name(script_state_t *s, int offset, int *aclass_no) {
	name_t *p;

	for (p = s->names; p; p = p->next)
		if (p->offset == offset) {
			if (aclass_no && p->class_no != -2) *aclass_no = p->class_no;
			return p->name;
		}

	return NULL;
}

/* -- Area table operations ----------------------------------------------  */

void
script_add_area(script_state_t *s, int start_offset, int end_offset, int type, void *data) {
	area_t *area;

	area = (area_t *) sci_malloc(sizeof(area_t));
	area->start_offset = start_offset;
	area->end_offset = end_offset;
	area->data = data;
	area->next = s->areas [type];

	s->areas [type] = area;
}

void
script_free_areas(script_state_t *s) {
	int i;

	for (i = 0; i < area_last; i++) {
		area_t *area = s->areas [i], *next_area;
		while (area) {
			next_area = area->next;
			free(area);
			area = next_area;
		}
	}
}

int
script_get_area_type(script_state_t *s, int offset, void **pdata) {
	int i;

	for (i = 0; i < area_last; i++) {
		area_t *area = s->areas [i];
		while (area) {
			if (area->start_offset <= offset && area->end_offset >= offset)     {
				if (pdata != NULL) *pdata = area->data;
				return i;
			}
			area = area->next;
		}
	}

	return -1;
}

char *
get_selector_name(disasm_state_t *d, int selector) {
	static char selector_name [256];

	if (d->snames && selector >= 0 && selector < d->selector_count)
		return d->snames [selector];
	else {
		sprintf(selector_name, "unknown_sel_%X", selector);
		return selector_name;
	}
}

const char *
get_class_name(disasm_state_t *d, int class_no) {
	static char class_name [256];

	if (class_no == -1)
		return "<none>";
	else if (class_no >= 0 && class_no < d->class_count && d->class_names [class_no])
		return d->class_names [class_no];
	else {
		sprintf(class_name, "class_%d", class_no);
		return class_name;
	}
}

/* -- Code to dump individual script block types -------------------------  */

static void
script_dump_object(disasm_state_t *d, script_state_t *s,
                   unsigned char *data, int seeker, int objsize, int pass_no) {
	int selectors, overloads, selectorsize;
	int species = getInt16(data + 8 + seeker);
	int superclass = getInt16(data + 10 + seeker);
	int namepos = getInt16(data + 14 + seeker);
	int i = 0;
	short sel;
	const char *name;
	char buf [256];
	short *sels;

	selectors = (selectorsize = getInt16(data + seeker + 6));
	name = namepos ? ((const char *)data + namepos) : "<unknown>";

	if (pass_no == 1)
		script_add_area(s, seeker, seeker + objsize - 1, area_object, strdup(name));

	if (pass_no == 2) {
		sciprintf(".object\n");
		sciprintf("Name: %s\n", name);
		sciprintf("Superclass: %s  [%x]\n", get_class_name(d, superclass), superclass);
		sciprintf("Species: %s  [%x]\n", get_class_name(d, species), species);

		sciprintf("-info-:%x\n", getInt16(data + 12 + seeker) & 0xffff);

		sciprintf("Function area offset: %x\n", getInt16(data + seeker + 4));
		sciprintf("Selectors [%x]:\n", selectors);
	}

	seeker += 8;

	if (species < d->class_count)
		sels = d->class_selectors [species];
	else
		sels = NULL;

	while (selectors--) {
		if (pass_no == 2) {
			sel = getInt16(data + seeker) & 0xffff;
			if (sels && (sels [i] >= 0) && (sels[i] < d->selector_count)) {
				sciprintf("  [#%03x] %s = 0x%x\n", i, d->snames [sels [i]], sel);
				i++;
			} else
				sciprintf("  [#%03x] <unknown> = 0x%x\n", i++, sel);
		}

		seeker += 2;
	}

	selectors = overloads = getInt16(data + seeker);

	if (pass_no == 2)
		sciprintf("Overloaded functions: %x\n", overloads);

	seeker += 2;

	while (overloads--) {
		word selector = getInt16(data + (seeker)) & 0xffff;
		if (d->old_header) selector >>= 1;

		if (pass_no == 1) {
			sprintf(buf, "%s::%s", name, get_selector_name(d, selector));
			script_add_name(s, getInt16(data + seeker + selectors*2 + 2), buf, species);
		} else {
			sciprintf("  [%03x] %s: @", selector, get_selector_name(d, selector));
			sciprintf("%04x\n", getInt16(data + seeker + selectors*2 + 2));
		}

		seeker += 2;
	}
}

static void
script_dump_class(disasm_state_t *d, script_state_t *s,
                  unsigned char *data, int seeker, int objsize, int pass_no) {
	word selectors, overloads, selectorsize;
	int species = getInt16(data + 8 + seeker);
	int superclass = getInt16(data + 10 + seeker);
	int namepos = getInt16(data + 14 + seeker);
	const char *name;
	char buf [256];
	int i;

	name = namepos ? ((const char *)data + namepos) : "<unknown>";
	selectors = (selectorsize = getInt16(data + seeker + 6));

	if (pass_no == 1) {
		if (species >= 0 && species < d->class_count) {
			if (!namepos) {
				sprintf(buf, "class_%d", species);
				d->class_names [species] = sci_strdup(buf);
			} else
				d->class_names [species] = sci_strdup(name);

			d->class_selector_count [species] = selectors;
			d->class_selectors [species] = (short *) sci_malloc(sizeof(short) * selectors);
		}
	}

	if (pass_no == 2) {
		sciprintf(".class\n");
		sciprintf("Name: %s\n", name);
		sciprintf("Superclass: %s  [%x]\n", get_class_name(d, superclass), superclass);
		sciprintf("Species: %x\n", species);
		sciprintf("-info-:%x\n", getInt16(data + 12 + seeker) & 0xffff);

		sciprintf("Function area offset: %x\n", getInt16(data + seeker + 4));
		sciprintf("Selectors [%x]:\n", selectors);
	}

	seeker += 8;
	selectorsize <<= 1;

	for (i = 0; i < selectors; i++) {
		word selector = 0xffff & getInt16(data + (seeker) + selectorsize);
		if (d->old_header) selector >>= 1;

		if (pass_no == 1) {
			if (species >= 0 && species < d->class_count)
				d->class_selectors [species][i] = selector;
		} else
			sciprintf("  [%03x] %s = 0x%x\n", selector, get_selector_name(d, selector),
			          getInt16(data + seeker) & 0xffff);

		seeker += 2;
	}

	seeker += selectorsize;

	selectors = overloads = getInt16(data + seeker);

	sciprintf("Overloaded functions: %x\n", overloads);

	seeker += 2;

	while (overloads--) {
		word selector = getInt16(data + (seeker)) & 0xffff;
		if (d->old_header) selector >>= 1;

		if (pass_no == 1) {
			sprintf(buf, "%s::%s", name, get_selector_name(d, selector));
			script_add_name(s, getInt16(data + seeker + selectors*2 + 2) & 0xffff, buf, species);
		} else {
			sciprintf("  [%03x] %s: @", selector & 0xffff, get_selector_name(d, selector));
			sciprintf("%04x\n", getInt16(data + seeker + selectors*2 + 2) & 0xffff);
		}

		seeker += 2;
	}
}

static int
script_dump_said_string(disasm_state_t *d, unsigned char *data, int seeker) {
	while (1) {
		unsigned short nextitem = (unsigned char) data [seeker++];
		if (nextitem == 0xFF) return seeker;

		if (nextitem >= 0xF0) {
			switch (nextitem) {
			case 0xf0:
				sciprintf(", ");
				break;
			case 0xf1:
				sciprintf("& ");
				break;
			case 0xf2:
				sciprintf("/ ");
				break;
			case 0xf3:
				sciprintf("( ");
				break;
			case 0xf4:
				sciprintf(") ");
				break;
			case 0xf5:
				sciprintf("[ ");
				break;
			case 0xf6:
				sciprintf("] ");
				break;
			case 0xf7:
				sciprintf("# ");
				break;
			case 0xf8:
				sciprintf("< ");
				break;
			case 0xf9:
				sciprintf("> ");
				break;
			}
		} else {
			nextitem = nextitem << 8 | (unsigned char) data [seeker++];
			sciprintf("%s ", vocab_get_any_group_word(nextitem, d->words, d->word_count));
			if (verbose)
				sciprintf("[%03x] ", nextitem);
		}
	}
}

static void
script_dump_said(disasm_state_t *d, script_state_t *s,
                 unsigned char *data, int seeker, int objsize, int pass_no) {
	int _seeker = seeker + objsize - 4;

	if (pass_no == 1) {
		script_add_area(s, seeker, seeker + objsize - 1, area_said, NULL);
		return;
	}

	sciprintf(".said\n");

	while (seeker < _seeker - 1) {
		sciprintf("%04x: ", seeker);
		seeker = script_dump_said_string(d, data, seeker);
		sciprintf("\n");
	}
}

static void
script_dump_synonyms(disasm_state_t *d, script_state_t *s,
                     unsigned char *data, int seeker, int objsize, int pass_no) {
	int _seeker = seeker + objsize - 4;

	sciprintf("Synonyms:\n");
	while (seeker < _seeker) {
		int search = getInt16(data + seeker);
		int replace = getInt16(data + seeker + 2);
		seeker += 4;
		if (search < 0) break;
		sciprintf("%s[%03x] ==> %s[%03x]\n",
		          vocab_get_any_group_word(search, d->words, d->word_count), search,
		          vocab_get_any_group_word(replace, d->words, d->word_count), replace);
	}
}

static void
script_dump_strings(disasm_state_t *d, script_state_t *s,
                    unsigned char *data, int seeker, int objsize, int pass_no) {
	int endptr = seeker + objsize - 4;

	if (pass_no == 1) {
		script_add_area(s, seeker, seeker + objsize - 1, area_string, NULL);
		return;
	}

	sciprintf(".strings\n");
	while (data [seeker] && seeker < endptr) {
		sciprintf("%04x: %s\n", seeker, data + seeker);
		seeker += strlen((char *) data + seeker) + 1;
	}
}

static void
script_dump_exports(disasm_state_t *d, script_state_t *s,
                    unsigned char *data, int seeker, int objsize, int pass_no) {
	byte *pexport = (byte *)(data + seeker);
	word export_count = getUInt16(pexport);
	int i;
	char buf [256];

	pexport += 2;

	if (pass_no == 2) sciprintf(".exports\n");

	for (i = 0; i < export_count; i++) {
		if (pass_no == 1) {
			guint16 offset = getUInt16(pexport);
			sprintf(buf, "exp_%02X", i);
			script_add_name(s, offset, buf, -1);
		} else
			sciprintf("%02X: %04X\n", i, *pexport);
		pexport += 2;
	}
}

/* -- The disassembly code -----------------------------------------------  */

static void
script_disassemble_code(disasm_state_t *d, script_state_t *s,
                        unsigned char *data, int seeker, int objsize, int pass_no) {
	int endptr = seeker + objsize - 4;
	int i = 0;
	int cur_class = -1;
	word dest;
	void *area_data;
	char buf [256];
	char *dest_name;

	if (pass_no == 2) sciprintf(".code\n");

	while (seeker < endptr - 1) {
		unsigned char opsize = data [seeker];
		unsigned char opcode = opsize >> 1;
		word param_value;
		char *name;

		opsize &= 1; /* byte if true, word if false */

		if (pass_no == 2) {
			name = script_find_name(s, seeker, &cur_class);
			if (name) sciprintf("      %s:\n", name);
			sciprintf("%04X: ", seeker);
			sciprintf("%s", d->opcodes[opcode].name);
			if (opcode_size && formats[opcode][0])
				sciprintf(".%c", opsize ? 'b' : 'w');
			sciprintf("\t");
		}

		seeker++;

		for (i = 0; formats[opcode][i]; i++)

			switch (formats[opcode][i]) {

			case Script_Invalid:
				if (pass_no == 2) sciprintf("-Invalid operation-");
				break;

			case Script_SByte:
			case Script_Byte:
				if (pass_no == 2) sciprintf(" %02x", data[seeker]);
				seeker++;
				break;

			case Script_Word:
			case Script_SWord:
				if (pass_no == 2)
					sciprintf(" %04x", 0xffff & (data[seeker] | (data[seeker+1] << 8)));
				seeker += 2;
				break;

			case Script_SVariable:
			case Script_Variable:
			case Script_Global:
			case Script_Local:
			case Script_Temp:
			case Script_Param:
			case Script_SRelative:
			case Script_Property:
			case Script_Offset:
				if (opsize)
					param_value = data [seeker++];
				else {
					param_value = 0xffff & (data[seeker] | (data[seeker+1] << 8));
					seeker += 2;
				}

				if (pass_no == 1) {
					if (opcode == op_jmp || opcode == op_bt || opcode == op_bnt)					{
						dest = seeker + (short) param_value;
						sprintf(buf, "lbl_%04X", dest);
						script_add_name(s, dest, buf, -2);
					}
				} else if (pass_no == 2)
					switch (formats[opcode][i]) {

					case Script_SVariable:
					case Script_Variable:
						if (opcode == op_callk) {
							sciprintf(" #%s", (param_value < d->kernel_names_nr)
							          ? d->kernel_names[param_value] : "<invalid>");
							if (verbose) sciprintf("[%x]", param_value);
						} else if (opcode == op_class || (opcode == op_super && i == 0))           {
							sciprintf(" %s", (d->class_names && param_value < d->class_count)
							          ? d->class_names[param_value] : "<invalid>");
							if (verbose) sciprintf("[%x]", param_value);
						} else sciprintf(opsize ? " %02x" : " %04x", param_value);

						if (opcode == op_pushi && param_value > 0 && param_value < d->selector_count)
							sciprintf("\t\t; selector <%s>", d->snames [param_value]);

						break;

					case Script_Global:
						sciprintf(" global_%d", param_value);
						break;

					case Script_Local:
						sciprintf(" local_%d", param_value);
						break;

					case Script_Temp:
						sciprintf(" temp_%d", param_value);
						break;

					case Script_Param:
						sciprintf(" param_%d", param_value);
						break;

					case Script_Offset:
						dest = (short) param_value;
						dest_name = script_find_name(s, dest, NULL);
						if (dest_name)
							sciprintf(" %s", dest_name);
						else
							sciprintf(" %04x", dest);

						if (verbose)
							sciprintf(opsize ? "   [%02x] " : "   [%04x] ", param_value);

						if (opcode == op_lofsa || opcode == op_lofss) {
							int atype = script_get_area_type(s, dest, &area_data);
							if (atype == area_string) {
								strncpy(buf, (char *) &data [dest], sizeof(buf) - 1);
								buf [sizeof(buf)-1] = 0;
								if (strlen(buf) > 40) {
									buf [40] = 0;
									strcat(buf, "...");
								}
								sciprintf("\t\t; \"%s\"", buf);
							} else if (atype == area_said) {
								sciprintf("\t\t; said \"");
								script_dump_said_string(d, data, dest);
								sciprintf("\"\n");
							} else if (atype == area_object)
								sciprintf("\t\t; object <%s>", area_data);
						}
						break;

					case Script_SRelative:
						dest = seeker + (short) param_value;
						dest_name = script_find_name(s, dest, NULL);
						if (dest_name)
							sciprintf(" %s", dest_name);
						else
							sciprintf(" %04x", dest);

						if (verbose)
							sciprintf(opsize ? "   [%02x] " : "   [%04x] ", param_value);

						if (opcode == op_lofsa || opcode == op_lofss) {
							int atype = script_get_area_type(s, dest, &area_data);
							if (atype == area_string) {
								strncpy(buf, (char *) &data [dest], sizeof(buf) - 1);
								buf [sizeof(buf)-1] = 0;
								if (strlen(buf) > 40) {
									buf [40] = 0;
									strcat(buf, "...");
								}
								sciprintf("\t\t; \"%s\"", buf);
							} else if (atype == area_said) {
								sciprintf("\t\t; said \"");
								script_dump_said_string(d, data, dest);
								sciprintf("\"\n");
							} else if (atype == area_object)
								sciprintf("\t\t; object <%s>", area_data);
						}
						break;

					case Script_Property:
						if (cur_class != -1 && param_value / 2 < d->class_selector_count [cur_class]) {
							sciprintf(" %s", get_selector_name(d, d->class_selectors [cur_class][param_value/2]));
							if (verbose) sciprintf("[%x]", param_value);
						} else
							sciprintf(opsize ? " %02x" : " %04x", param_value);

						break;

					case Script_End:
						if (pass_no == 2) sciprintf("\n");
						break;

					default:
						sciprintf("Unexpected opcode format %d\n", (formats[opcode][i]));
					}

			default:
				break;
			}
		if (pass_no == 2) sciprintf("\n");

	}

}

void
disassemble_script_pass(disasm_state_t *d, script_state_t *s,
                        resource_t *script, int pass_no) {
	int _seeker = 0;
	word id = getInt16(script->data);

	if (id > 15) {
		if (pass_no == 2) sciprintf("; Old script header detected\n");
		d->old_header = 1;
	}

	if (d->old_header) _seeker = 2;

	while (_seeker < script->size) {
		int objtype = getInt16(script->data + _seeker);
		int objsize;
		int seeker = _seeker + 4;

		if (!objtype) return;

		if (pass_no == 2)
			sciprintf("\n");

		objsize = getInt16(script->data + _seeker + 2);

		if (pass_no == 2) {
			sciprintf("; Obj type #%x, offset 0x%x, size 0x%x:\n", objtype, _seeker, objsize);
			if (hexdump) sci_hexdump(script->data + seeker, objsize - 4, seeker);
		}

		_seeker += objsize;

		switch (objtype) {
		case sci_obj_object:
			script_dump_object(d, s, script->data, seeker, objsize, pass_no);
			break;

		case sci_obj_code:
			script_disassemble_code(d, s, script->data, seeker, objsize, pass_no);
			break;

		case sci_obj_synonyms:
			script_dump_synonyms(d, s, script->data, seeker, objsize, pass_no);
			break;

		case sci_obj_said:
			script_dump_said(d, s, script->data, seeker, objsize, pass_no);
			break;

		case sci_obj_strings:
			script_dump_strings(d, s, script->data, seeker, objsize, pass_no);
			break;

		case sci_obj_class:
			script_dump_class(d, s, script->data, seeker, objsize, pass_no);
			break;

		case sci_obj_exports:
			script_dump_exports(d, s, script->data, seeker, objsize, pass_no);
			break;

		case sci_obj_pointers:
			if (pass_no == 2) {
				sciprintf("Pointers\n");
				sci_hexdump(script->data + seeker, objsize - 4, seeker);
			};
			break;

		case sci_obj_preload_text:
			if (pass_no == 2) {
				sciprintf("The script has a preloaded text resource\n");
			};
			break;

		case sci_obj_localvars:
			if (pass_no == 2) {
				sciprintf("Local vars\n");
				sci_hexdump(script->data + seeker, objsize - 4, seeker);
			};
			break;

		default:
			sciprintf("Unsupported %d!\n", objtype);
			return;
		}
	}

	sciprintf("Script ends without terminator\n");
}

void
disassemble_script(disasm_state_t *d, int res_no, int pass_no) {
	resource_t *script = scir_find_resource(resmgr, sci_script, res_no, 0);
	script_state_t *s = find_script_state(d, res_no);

	if (!script) {
		sciprintf("Script not found!\n");
		return;
	}

	disassemble_script_pass(d, s, script, pass_no);
}
