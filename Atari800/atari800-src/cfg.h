#ifndef CFG_H_
#define CFG_H_
/* Checks for "popular" filenames of ROM images in the specified directory
   and sets CFG_*_filename to the ones found.
   If only_if_not_set is TRUE, then CFG_*_filename is modified only when
   Util_filenamenotset() is TRUE for it. */
void CFG_FindROMImages(const char *directory, int only_if_not_set);

/* Load Atari800 text configuration file. */
int CFG_LoadConfig(const char *alternate_config_filename);

/* Writes Atari800 text configuration file. */
int CFG_WriteConfig(void);

/* Paths to ROM images. */
extern char CFG_osa_filename[FILENAME_MAX];
extern char CFG_osb_filename[FILENAME_MAX];
extern char CFG_xlxe_filename[FILENAME_MAX];
extern char CFG_5200_filename[FILENAME_MAX];
extern char CFG_basic_filename[FILENAME_MAX];

/* Compares the string PARAM with each entry in the CFG_STRINGS array
   (of size CFG_STRINGS_SIZE), and returns index under which PARAM is found.
   If PARAM does not exist in CFG_STRINGS, returns value lower than 0.
   String comparison is case-insensitive. */
int CFG_MatchTextParameter(char const *param, char const * const cfg_strings[], int cfg_strings_size);

#endif /* CFG_H_ */
