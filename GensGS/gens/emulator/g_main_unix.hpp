/**
 * Gens: Main loop. (Linux-specific code)
 */

#ifndef G_MAIN_LINUX_HPP
#define G_MAIN_LINUX_HPP

#ifdef __cplusplus
extern "C" {
#endif

#define GENS_DIR_SEPARATOR_STR "/"
#define GENS_DIR_SEPARATOR_CHR '/'

#include <sys/stat.h>
#include <stdlib.h>

void Get_Save_Path(char *buf, size_t n);
void Create_Save_Directory(const char *dir);

// Main loop.
int main(int argc, char *argv[]);

#ifdef __cplusplus
}
#endif

#endif
