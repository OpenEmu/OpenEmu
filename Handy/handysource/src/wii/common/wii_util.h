#ifndef WII_UTIL_H
#define WII_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

#define DIR_SEP_CHAR '/'
#define DIR_SEP_STR  "/"

typedef struct RGBA
{
  unsigned char R;
  unsigned char G;
  unsigned char B;
  unsigned char A;
} RGBA;

extern void Util_chomp(char *s);
extern void Util_trim(char *s);
extern int Util_sscandec(const char *s);
extern char *Util_strlcpy(char *dest, const char *src, size_t size);
extern int Util_fileexists( char *filename );
extern void Util_splitpath(const char *path, char *dir_part, char *file_part);
extern void Util_getextension( char *filename, char *ext );
extern int Util_hextodec( const char* hex );
extern void Util_hextorgba( const char* hex, RGBA* rgba );
extern void Util_dectohex( int dec, char *hex, int fill );
extern void Util_rgbatohex( RGBA* rgba, char *hex );

#ifdef __cplusplus
}
#endif

#endif
