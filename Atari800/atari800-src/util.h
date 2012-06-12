#ifndef UTIL_H_
#define UTIL_H_

#include "config.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

/* String functions ------------------------------------------------------ */

/* Returns TRUE if the characters are equal or represent the same letter
   in different case. */
int Util_chrieq(char c1, char c2);

#ifdef __STRICT_ANSI__
/* Returns a positive integer if str1>str2, negative if str1<str2
 * 0 if str1 == str2, case insensitive */
int Util_stricmp(const char *str1, const char *str2);
#elif defined(HAVE_WINDOWS_H)
#define Util_stricmp _stricmp
#elif defined(HAVE_STRCASECMP)
#define Util_stricmp strcasecmp
#else
#define Util_stricmp stricmp
#endif

/* Same as stpcpy() in some C libraries: copies src to dest
   and returns a pointer to the trailing NUL in dest. */
char *Util_stpcpy(char *dest, const char *src);

/* NestedVM strncpy from newlib has a bug */
#ifdef HAVE_STRNCPY
#define Util_strncpy strncpy
#else
char *Util_strncpy(char *dest, const char *src, size_t size);
#endif

/* Same as strlcpy() in some C libraries: copies src to dest
   and terminates the string. Never writes more than size characters
   to dest (the result may be truncated). Returns dest. */
char *Util_strlcpy(char *dest, const char *src, size_t size);

/* Modifies the string to uppercase and returns it. */
char *Util_strupper(char *s);

/* Modifies the string to lowercase and returns it. */
char *Util_strlower(char *s);

/* Similar to Perl's chomp(): removes trailing LF, CR or CR/LF. */
void Util_chomp(char *s);

/* Similar to Basic's trim(): removes leading and trailing whitespace. */
void Util_trim(char *s);

/* Converts the string to a non-negative integer and returns it.
   The string must represent the number and nothing else.
   -1 indicates an error. */
int Util_sscandec(const char *s);

/* Likewise, but parses hexadecimal numbers. */
int Util_sscanhex(const char *s);

/* Likewise, but allows only 0 and 1. */
int Util_sscanbool(const char *s);

/* Converts the string S to a signed integer *DEST and returns a success flag.
   The string must be a decimal number optionally preceded by a + or - sign. */
int Util_sscansdec(char const *s, int *dest);

/* Converts the string S to a (signed) floating point number *DEST and returns
   a success flag. The string must be a floating-point number. */
int Util_sscandouble(char const *s, double *dest);

/* safe_strncpy guarantees that the dest. buffer ends with '\0' */
char *safe_strncpy(char *, const char *, int);

/* Math functions -------------------------------------------------------- */

/* Rounds the floating-point number to the nearest integer. */
#if HAVE_ROUND
#define Util_round round
#else
double Util_round(double x);
#endif

/* Function for comparing double floating-point numbers. */
#define Util_almostequal(x, y, epsilon) (fabs((x)-(y)) <= (epsilon))

/* Memory management ----------------------------------------------------- */

/* malloc() with out-of-memory checking. Never returns NULL. */
void *Util_malloc(size_t size);

/* realloc() with out-of-memory checking. Never returns NULL. */
void *Util_realloc(void *ptr, size_t size);

/* strdup() with out-of-memory checking. Never returns NULL. */
char *Util_strdup(const char *s);


/* Filenames ------------------------------------------------------------- */

/* I assume here that '\n' is not valid in filenames,
   at least not as their first character. */
#define Util_FILENAME_NOT_SET               "\n"
#define Util_filenamenotset(filename)  ((filename)[0] == '\n')

#ifdef DIR_SEP_BACKSLASH
#define Util_DIR_SEP_CHAR '\\'
#define Util_DIR_SEP_STR  "\\"
#else
#define Util_DIR_SEP_CHAR '/'
#define Util_DIR_SEP_STR  "/"
#endif

/* Splits a filename into directory part and file part. */
/* dir_part or file_part may be NULL. */
void Util_splitpath(const char *path, char *dir_part, char *file_part);

/* Concatenates file paths.
   Places directory separator char between paths, unless path1 is empty
   or ends with the separator char, or path2 starts with the separator char. */
void Util_catpath(char *result, const char *path1, const char *path2);


/* File I/O -------------------------------------------------------------- */

/* Returns TRUE if the specified file exists. */
int Util_fileexists(const char *filename);

/* Returns TRUE if the specified directory exists. */
int Util_direxists(const char *filename);

/* Rewinds the stream to its beginning. */
#ifdef HAVE_REWIND
#define Util_rewind(fp) rewind(fp)
#else
#define Util_rewind(fp) fseek(fp, 0, SEEK_SET)
#endif

/* Returns the length of an open stream.
   May change the current position. */
int Util_flen(FILE *fp);

/* Deletes a file, returns 0 on success, -1 on failure. */
#ifdef HAVE_WINDOWS_H
int Util_unlink(const char *filename);
#define HAVE_UTIL_UNLINK
#elif defined(HAVE_UNLINK)
#define Util_unlink  unlink
#define HAVE_UTIL_UNLINK
#endif /* defined(HAVE_UNLINK) */

/* Creates a file that does not exist and fills in filename with its name. */
FILE *Util_uniqopen(char *filename, const char *mode);

/* Support for temporary files.

   Util_tmpbufdef() defines storage for names of temporary files, if necessary.
     Example use:
     Util_tmpbufdef(static, mytmpbuf[4]) // four buffers with "static" storage
   Util_fopen() opens a *non-temporary* file and marks tmpbuf
   as *not containing* name of a temporary file.
   Util_tmpopen() creates a temporary file with "wb+" mode.
   Util_fclose() closes a file. If it was temporary, it gets deleted.

   There are 3 implementations of the above:
   - one that uses tmpfile() (preferred)
   - one that stores names of temporary files and deletes them when they
     are closed
   - one that creates unique files but doesn't delete them
     because Util_unlink is not available
*/
#ifdef HAVE_TMPFILE
#define Util_tmpbufdef(modifier, def)
#define Util_fopen(filename, mode, tmpbuf)  fopen(filename, mode)
#define Util_tmpopen(tmpbuf)                tmpfile()
#define Util_fclose(fp, tmpbuf)             fclose(fp)
#elif defined(HAVE_UTIL_UNLINK)
#define Util_tmpbufdef(modifier, def)       modifier char def [FILENAME_MAX];
#define Util_fopen(filename, mode, tmpbuf)  (tmpbuf[0] = '\0', fopen(filename, mode))
#define Util_tmpopen(tmpbuf)                Util_uniqopen(tmpbuf, "wb+")
#define Util_fclose(fp, tmpbuf)             (fclose(fp), tmpbuf[0] != '\0' && Util_unlink(tmpbuf))
#else
/* if we can't delete the created file, leave it to the user */
#define Util_tmpbufdef(modifier, def)
#define Util_fopen(filename, mode, tmpbuf)  fopen(filename, mode)
#define Util_tmpopen(tmpbuf)                Util_uniqopen(NULL, "wb+")
#define Util_fclose(fp, tmpbuf)             fclose(fp)
#endif

#endif /* UTIL_H_ */
