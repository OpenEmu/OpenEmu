/*---------------------------------------------------------------------------

  unzip.h

  This header file is used by all of the unzip source files.  Its contents
  are divided into seven more-or-less separate sections:  predefined macros,
  OS-dependent includes, (mostly) OS-independent defines, typedefs, function
  prototypes (or "prototypes," in the case of non-ANSI compilers), macros,
  and global-variable declarations.

  ---------------------------------------------------------------------------*/

#include <zlib.h>

/*****************************************/
/*  Predefined, Machine-specific Macros  */
/*****************************************/

#if (defined(__GO32__) && defined(unix))   /* DOS extender */
#  undef unix
#endif

#if defined(unix) || defined(__convexc__) || defined(M_XENIX)
#  ifndef UNIX
#    define UNIX
#  endif /* !UNIX */
#endif /* unix || __convexc__ || M_XENIX */

/* Much of the following is swiped from zip's tailor.h: */

/* define MSDOS for Turbo C (unless OS/2) and Power C as well as Microsoft C */
#ifdef __POWERC
#  define __TURBOC__
#  define MSDOS
#endif /* __POWERC */
#if (defined(__TURBOC__) && defined(__MSDOS__) && !defined(MSDOS))
#  define MSDOS
#endif

/* use prototypes and ANSI libraries if __STDC__, or Microsoft or Borland C,
 * or Silicon Graphics, or Convex, or IBM C Set/2, or GNU gcc under emx, or
 * or Watcom C, or Macintosh, or Windows NT.
 */
#if (__STDC__ || defined(MSDOS) || defined(sgi) || defined(CONVEX) || defined(__sun))
#  ifndef PROTO
#    define PROTO
#  endif
#  define MODERN
#endif
#if (defined(__IBMC__) || defined(__EMX__) || defined(__WATCOMC__))
#  ifndef PROTO
#    define PROTO
#  endif
#  define MODERN
#endif
#if (defined(THINK_C) || defined(MPW) || defined(WIN32))
#  ifndef PROTO
#    define PROTO
#  endif
#  define MODERN
#endif

/* turn off prototypes if requested */
#if (defined(NOPROTO) && defined(PROTO))
#  undef PROTO
#endif

#if (defined(ultrix) || defined(bsd4_2) || defined(sun) || defined(pyr))
#  if (!defined(BSD) && !defined(__SYSTEM_FIVE) && !defined(SYSV))
#    define BSD
#  endif /* !BSD && !__SYSTEM_FIVE && !SYSV */
#endif /* ultrix || bsd4_2 || sun || pyr */

#if (defined(CONVEX) || defined(CRAY) || defined(__SYSTEM_FIVE))
#  ifndef TERMIO
#    define TERMIO
#  endif /* !TERMIO */
#endif /* CONVEX || CRAY || __SYSTEM_FIVE */

#ifdef pyr  /* Pyramid */
#  ifndef ZMEM
#    define ZMEM
#  endif /* !ZMEM */
#endif /* pyr */

#ifdef CRAY
#  ifdef ZMEM
#    undef ZMEM
#  endif /* ZMEM */
#endif /* CRAY */

/* the i386 test below is to catch SCO Unix (which has redefinition
 * warnings if param.h is included), but it probably doesn't hurt if
 * other 386 Unixes get nailed, too...except now that 386BSD and BSDI
 * exist.  Sigh.  <sys/param.h> is mostly included for "BSD", I think.
 * [An alternate fix for SCO Unix is below.]
 */
#if (defined(MINIX) || (defined(i386) && defined(unix)))
#  define NO_PARAM_H
#endif /* MINIX || (i386 && unix) */





/***************************/
/*  OS-Dependent Includes  */
/***************************/

#ifndef MINIX            /* Minix needs it after all the other includes (?) */
#  include <stdio.h>
#endif
#include <ctype.h>       /* skip for VMS, to use tolower() function? */
#include <errno.h>       /* used in mapname() */
#ifndef NO_ERRNO
#  define DECLARE_ERRNO  /* everybody except MSC 6.0, SCO cc, Watcom C/386 */
#endif /* !NO_ERRNO */
#ifdef VMS
#  include <types.h>     /* (placed up here instead of in VMS section below */
#  include <stat.h>      /* because types.h is used in some other headers) */
#else /* !VMS */
#  if !defined(THINK_C) && !defined(MPW)
#    include <sys/types.h>                 /* off_t, time_t, dev_t, ... */
#    include <sys/stat.h>
#  endif /* !THINK_C && !MPW */
#endif /* ?VMS */

#ifdef MODERN
#  if (!defined(M_XENIX) && !(defined(__GNUC__) && defined(sun)))
#    include <stddef.h>
#  endif
#  if (!defined(__GNUC__) && !defined(apollo))   /* both define __STDC__ */
#    include <stdlib.h>    /* standard library prototypes, malloc(), etc. */
#  else
#    ifdef __EMX__
#      include <stdlib.h>  /* emx IS gcc but has stdlib.h */
#    endif
#  endif
#  include <string.h>      /* defines strcpy, strcmp, memcpy, etc. */
   typedef size_t extent;
#else /* !MODERN */
   char *malloc();
   char *strchr(), *strrchr();
   long lseek();
   typedef unsigned int extent;
#  define void int
#endif /* ?MODERN */

/* this include must be down here for SysV.4, for some reason... */
#include <signal.h>      /* used in unzip.c, file_io.c */



/*---------------------------------------------------------------------------
    Next, a word from our Unix (mostly) sponsors:
  ---------------------------------------------------------------------------*/

#ifdef UNIX
#  ifdef AMIGA
#    include <libraries/dos.h>
#  else /* !AMIGA */
#    ifndef NO_PARAM_H
#if 0  /* [GRR: this is an alternate fix for SCO's redefinition bug] */
#      ifdef NGROUPS_MAX
#        undef NGROUPS_MAX     /* SCO bug:  defined again in <param.h> */
#      endif /* NGROUPS_MAX */
#endif /* 0 */
#      include <sys/param.h>   /* conflict with <sys/types.h>, some systems? */
#    endif /* !NO_PARAM_H */
#  endif /* ?AMIGA */

#  ifndef BSIZE
#    ifdef MINIX
#      define BSIZE   1024
#    else /* !MINIX */
#      define BSIZE   DEV_BSIZE  /* assume common for all Unix systems */
#    endif /* ?MINIX */
#  endif

#  ifndef BSD
#    if (!defined(AMIGA) && !defined(MINIX))
#      define NO_MKDIR           /* for mapname() */
#    endif /* !AMIGA && !MINIX */
#    include <time.h>
     struct tm *gmtime(), *localtime();
#  else   /* BSD */
#    include <sys/time.h>
#    include <sys/timeb.h>
#    ifdef _AIX
#      include <time.h>
#    endif
#  endif

#else   /* !UNIX */
#  define BSIZE   512               /* disk block size */
#endif /* ?UNIX */

#if (defined(V7) || defined(BSD))
#  define strchr    index
#  define strrchr   rindex
#endif

/*---------------------------------------------------------------------------
    And now, our MS-DOS and OS/2 corner:
  ---------------------------------------------------------------------------*/

#ifdef __TURBOC__
#  define DOS_OS2
#  include <sys/timeb.h>      /* for structure ftime                        */
#  ifndef __BORLANDC__        /* there appears to be a bug (?) in Borland's */
#    include <mem.h>          /*   MEM.H related to __STDC__ and far poin-  */
#  endif                      /*   ters. (dpk)  [mem.h included for memcpy] */
#  include <dos.h>            /* for REGS macro (at least for Turbo C 2.0)  */
#else                         /* NOT Turbo C (or Power C)...                */
#  ifdef MSDOS                /*   but still MS-DOS, so we'll assume it's   */
#    ifndef MSC               /*   Microsoft's compiler and fake the ID, if */
#      define MSC             /*   necessary (it is in 5.0; apparently not  */
#    endif                    /*   in 5.1 and 6.0)                          */
#    include <dos.h>          /* for _dos_setftime()                        */
#  endif
#endif

#if (defined(__IBMC__) && defined(__OS2__))
#  define DOS_OS2
#  define S_IFMT 0xF000
#  define timezone _timezone
#endif

#ifdef __WATCOMC__
#  define DOS_OS2
#  define __32BIT__
#  ifdef DECLARE_ERRNO
#    undef DECLARE_ERRNO
#  endif
#  undef far
#  define far
#endif

#ifdef __EMX__
#  define DOS_OS2
#  define __32BIT__
#  define far
#endif /* __EMX__ */

#ifdef MSC                    /* defined for all versions of MSC now         */
#  define DOS_OS2             /* Turbo C under DOS, MSC under DOS or OS/2    */
#  if (defined(_MSC_VER) && (_MSC_VER >= 600))    /* new with 5.1 or 6.0 ... */
#    undef DECLARE_ERRNO      /* errno is now a function in a dynamic link   */
#  endif                      /*   library (or something)--incompatible with */
#endif                        /*   the usual "extern int errno" declaration  */

#ifdef DOS_OS2                /* defined for all MS-DOS and OS/2 compilers   */
#  include <io.h>             /* lseek(), open(), setftime(), dup(), creat() */
#  include <time.h>           /* localtime() */
#endif

#ifdef OS2                    /* defined for all OS/2 compilers */
#  ifdef isupper
#    undef isupper
#  endif
#  ifdef tolower
#    undef tolower
#  endif
#  define isupper(x)   IsUpperNLS((unsigned char)(x))
#  define tolower(x)   ToLowerNLS((unsigned char)(x))
#endif

#ifdef WIN32
#  include <io.h>             /* read(), open(), etc. */
#  include <time.h>
#  include <memory.h>
#  include <direct.h>         /* mkdir() */
#  ifdef FILE_IO_C
#    include <fcntl.h>
#    include <conio.h>
#    include <sys/types.h>
#    include <sys/utime.h>
#    include <windows.h>
#    define DOS_OS2
#    define getch() getchar()
#  endif
#endif

/*---------------------------------------------------------------------------
    Followed by some VMS (mostly) stuff:
  ---------------------------------------------------------------------------*/

#ifdef VMS
#  include <time.h>               /* the usual non-BSD time functions */
#  include <file.h>               /* same things as fcntl.h has */
#  include <rms.h>
#  define _MAX_PATH NAM$C_MAXRSS  /* to define FILNAMSIZ below */
#  define UNIX                    /* can share most of same code from now on */
#  define RETURN    return_VMS    /* VMS interprets return codes incorrectly */
#else /* !VMS */
#  ifndef THINK_C
#    define RETURN  return        /* only used in main() */
#  else
#    define RETURN(v) { int n;\
                        n = (v);\
                        fprintf(stderr, "\npress <return> to continue ");\
                        while (getc(stdin) != '\n');\
                        putc('\n', stderr);\
                        InitCursor();\
                        goto start;\
                      }
#  endif
#  ifdef V7
#    define O_RDONLY  0
#    define O_WRONLY  1
#    define O_RDWR    2
#  else /* !V7 */
#    ifdef MTS
#      include <sys/file.h>     /* MTS uses this instead of fcntl.h */
#      include <timeb.h>
#      include <time.h>
#    else /* !MTS */
#      ifdef COHERENT           /* Coherent 3.10/Mark Williams C */
#        include <sys/fcntl.h>
#        define SHORT_NAMES
#        define tzset  settz
#      else /* !COHERENT */
#        include <fcntl.h>      /* O_BINARY for open() w/o CR/LF translation */
#      endif /* ?COHERENT */
#    endif /* ?MTS */
#  endif /* ?V7 */
#endif /* ?VMS */

#if (defined(MSDOS) || defined(VMS))
#  define DOS_VMS
#endif

/*---------------------------------------------------------------------------
    And some Mac stuff for good measure:
  ---------------------------------------------------------------------------*/

#ifdef THINK_C
#  define MACOS
#  ifndef __STDC__            /* if Think C hasn't defined __STDC__ ... */
#    define __STDC__ 1        /*   make sure it's defined: it needs it */
#  else /* __STDC__ defined */
#    if !__STDC__             /* sometimes __STDC__ is defined as 0; */
#      undef __STDC__         /*   it needs to be 1 or required header */
#      define __STDC__ 1      /*   files are not properly included. */
#    endif /* !__STDC__ */
#  endif /* ?defined(__STDC__) */
#endif /* THINK_C */

#ifdef MPW
#  define MACOS
#  include <Errors.h>
#  include <Files.h>
#  include <Memory.h>
#  include <Quickdraw.h>
#  include <ToolUtils.h>
#  define CtoPstr c2pstr
#  define PtoCstr p2cstr
#  ifdef CR
#    undef  CR
#  endif
#endif /* MPW */

#ifdef MACOS
#  define open(x,y) macopen(x,y, gnVRefNum, glDirID)
#  define close macclose
#  define read macread
#  define write macwrite
#  define lseek maclseek
#  define creat(x,y) maccreat(x, gnVRefNum, glDirID, gostCreator, gostType)
#  define stat(x,y) macstat(x,y,gnVRefNum, glDirID)

#  ifndef isascii
#    define isascii(c) ((unsigned char)(c) <= 0x3F)
#  endif

#  include "macstat.h"

typedef struct _ZipExtraHdr {
    unsigned short header;    /*    2 bytes */
    unsigned short data;      /*    2 bytes */
} ZIP_EXTRA_HEADER;

typedef struct _MacInfoMin {
    unsigned short header;    /*    2 bytes */
    unsigned short data;      /*    2 bytes */
    unsigned long signature;  /*    4 bytes */
    FInfo finfo;              /*   16 bytes */
    unsigned long lCrDat;     /*    4 bytes */
    unsigned long lMdDat;     /*    4 bytes */
    unsigned long flags ;     /*    4 bytes */
    unsigned long lDirID;     /*    4 bytes */
                              /*------------*/
} MACINFOMIN;                 /* = 40 bytes for size of data */

typedef struct _MacInfo {
    unsigned short header;    /*    2 bytes */
    unsigned short data;      /*    2 bytes */
    unsigned long signature;  /*    4 bytes */
    FInfo finfo;              /*   16 bytes */
    unsigned long lCrDat;     /*    4 bytes */
    unsigned long lMdDat;     /*    4 bytes */
    unsigned long flags ;     /*    4 bytes */
    unsigned long lDirID;     /*    4 bytes */
    char rguchVolName[28];    /*   28 bytes */
                              /*------------*/
} MACINFO;                    /* = 68 bytes for size of data */
#endif /* MACOS */

/*---------------------------------------------------------------------------
    And finally, some random extra stuff:
  ---------------------------------------------------------------------------*/

#ifdef MINIX
#  include <stdio.h>
#endif

#ifdef SHORT_NAMES         /* Mark Williams C, ...? */
#  define extract_or_test_files    xtr_or_tst_files
#  define extract_or_test_member   xtr_or_tst_member
#endif

#ifdef MTS
#  include <unix.h>          /* Some important non-ANSI routines */
#  define mkdir(s,n) (-1)    /* No "make directory" capability */
#  define EBCDIC             /* Set EBCDIC conversion on */
#endif





/*************/
/*  Defines  */
/*************/

#ifndef WSIZE
#  define WSIZE 0x8000       /* window size--must be a power of two, and */
#endif /* !WSIZE */          /*  at least 32K for zip's deflate method */

#define DIR_BLKSIZ    64     /* number of directory entries per block
                              *  (should fit in 4096 bytes, usually) */
#ifndef INBUFSIZ
#  define INBUFSIZ    2048   /* works for MS-DOS small model */
#endif /* !INBUFSIZ */

/*
 * If <limits.h> exists on most systems, should include that, since it may
 * define some or all of the following:  NAME_MAX, PATH_MAX, _POSIX_NAME_MAX,
 * _POSIX_PATH_MAX.
 */
#ifdef DOS_OS2
#  include <limits.h>
#endif /* DOS_OS2 */

#ifdef _MAX_PATH
#  define FILNAMSIZ       (_MAX_PATH)
#else /* !_MAX_PATH */
#  define FILNAMSIZ       1025
#endif /* ?_MAX_PATH */

#ifndef PATH_MAX
#  ifdef MAXPATHLEN                /* defined in <sys/param.h> some systems */
#    define PATH_MAX      MAXPATHLEN
#  else
#    if FILENAME_MAX > 255         /* used like PATH_MAX on some systems */
#      define PATH_MAX    FILENAME_MAX
#    else
#      define PATH_MAX    (FILNAMSIZ - 1)
#    endif
#  endif /* ?MAXPATHLEN */
#endif /* !PATH_MAX */

#define OUTBUFSIZ         INBUFSIZ

#define ZSUFX             ".zip"
#define CENTRAL_HDR_SIG   "\113\001\002"   /* the infamous "PK" signature */
#define LOCAL_HDR_SIG     "\113\003\004"   /*  bytes, sans "P" (so unzip */
#define END_CENTRAL_SIG   "\113\005\006"   /*  executable not mistaken for */
#define EXTD_LOCAL_SIG    "\113\007\010"   /*  zipfile itself) */

#define SKIP              0    /* choice of activities for do_string() */
#define DISPLAY           1
#define FILENAME          2
#define EXTRA_FIELD       3

#define DOES_NOT_EXIST    -1   /* return values for check_for_newer() */
#define EXISTS_AND_OLDER  0
#define EXISTS_AND_NEWER  1

#define DOS_OS2_FAT_      0    /* version_made_by codes (central dir) */
#define AMIGA_            1
#define VMS_              2    /* make sure these are not defined on */
#define UNIX_             3    /*  the respective systems!!  (like, for */
#define VM_CMS_           4    /*  instance, "VMS", or "UNIX":  CFLAGS = */
#define ATARI_            5    /*  -O -DUNIX) */
#define OS2_HPFS_         6
#define MAC_              7
#define Z_SYSTEM_         8
#define CPM_              9
/* #define TOPS20_   10?  (TOPS20_ is to be defined in PKZIP 2.0...)  */
#define NUM_HOSTS         10   /* index of last system + 1 */

#define STORED            0    /* compression methods */
#define SHRUNK            1
#define REDUCED1          2
#define REDUCED2          3
#define REDUCED3          4
#define REDUCED4          5
#define IMPLODED          6
#define TOKENIZED         7
#define DEFLATED          8
#define NUM_METHODS       9    /* index of last method + 1 */
/* don't forget to update list_files() appropriately if NUM_METHODS changes */

#define DF_MDY            0    /* date format 10/26/91 (USA only) */
#define DF_DMY            1    /* date format 26/10/91 (most of the world) */
#define DF_YMD            2    /* date format 91/10/26 (a few countries) */

#define UNZIP_VERSION     20   /* compatible with PKUNZIP 2.0 */
#define VMS_VERSION       42   /* if OS-needed-to-extract is VMS:  can do */

/*---------------------------------------------------------------------------
    True sizes of the various headers, as defined by PKWare--so it is not
    likely that these will ever change.  But if they do, make sure both these
    defines AND the typedefs below get updated accordingly.
  ---------------------------------------------------------------------------*/
#define LREC_SIZE     26    /* lengths of local file headers, central */
#define CREC_SIZE     42    /*  directory headers, and the end-of-    */
#define ECREC_SIZE    18    /*  central-dir record, respectively      */

#define MAX_BITS      13                 /* used in unShrink() */
#define HSIZE         (1 << MAX_BITS)    /* size of global work area */

#define LF      10    /* '\n' on ASCII machines.  Must be 10 due to EBCDIC */
#define CR      13    /* '\r' on ASCII machines.  Must be 13 due to EBCDIC */
#define CTRLZ   26    /* DOS & OS/2 EOF marker (used in file_io.c, vms.c) */

#ifdef EBCDIC
#  define ascii_to_native(c)   ebcdic[(c)]
#  define NATIVE    "EBCDIC"
#endif

#if MPW
#  define FFLUSH    putc('\n',stderr);
#else /* !MPW */
#  define FFLUSH    fflush(stderr);
#endif /* ?MPW */

#ifdef VMS
#  define ENV_UNZIP     "UNZIP_OPTS"      /* name of environment variable */
#  define ENV_ZIPINFO   "ZIPINFO_OPTS"
#else /* !VMS */
#  define ENV_UNZIP     "UNZIP"
#  define ENV_ZIPINFO   "ZIPINFO"
#endif /* ?VMS */

#ifdef CRYPT
#  define PWLEN         80
#  define DECRYPT(b)    (update_keys(t=((b)&0xff)^decrypt_byte()),t)
#endif /* CRYPT */

#ifdef QQ  /* Newtware version */
#  define QCOND   (!quietflg)   /* for no file comments with -vq or -vqq */
#else      /* (original) Bill Davidsen version  */
#  define QCOND   (which_hdr)   /* no way to kill file comments with -v, -l */
#endif

#ifndef TRUE
#  define TRUE      1   /* sort of obvious */
#endif
#ifndef FALSE
#  define FALSE     0
#endif

#ifndef SEEK_SET        /* These should all be declared in stdio.h!  But   */
#  define SEEK_SET  0   /*  since they're not (in many cases), do so here. */
#  define SEEK_CUR  1
#  define SEEK_END  2
#endif

#ifndef S_IRUSR
#  define S_IRWXU       00700       /* read, write, execute: owner */
#  define S_IRUSR       00400       /* read permission: owner */
#  define S_IWUSR       00200       /* write permission: owner */
#  define S_IXUSR       00100       /* execute permission: owner */
#  define S_IRWXG       00070       /* read, write, execute: group */
#  define S_IRGRP       00040       /* read permission: group */
#  define S_IWGRP       00020       /* write permission: group */
#  define S_IXGRP       00010       /* execute permission: group */
#  define S_IRWXO       00007       /* read, write, execute: other */
#  define S_IROTH       00004       /* read permission: other */
#  define S_IWOTH       00002       /* write permission: other */
#  define S_IXOTH       00001       /* execute permission: other */
#endif /* !S_IRUSR */

#ifdef ZIPINFO      /* these are individually checked because SysV doesn't */
#  ifndef S_IFBLK   /*  have some of them, Microsoft C others, etc. */
#    define   S_IFBLK     0060000     /* block special */
#  endif
#  ifndef S_IFIFO  /* in Borland C, not MSC */
#    define   S_IFIFO     0010000     /* fifo */
#  endif
#  ifndef S_IFLNK  /* in BSD, not SysV */
#    define   S_IFLNK     0120000     /* symbolic link */
#  endif
#  ifndef S_IFSOCK  /* in BSD, not SysV */
#    define   S_IFSOCK    0140000     /* socket */
#  endif
#  ifndef S_ISUID
#    define S_ISUID       04000       /* set user id on execution */
#  endif
#  ifndef S_ISGID
#    define S_ISGID       02000       /* set group id on execution */
#  endif
#  ifndef S_ISVTX
#    define S_ISVTX       01000       /* directory permissions control */
#  endif
#  ifndef S_ENFMT
#    define S_ENFMT       S_ISGID     /* record locking enforcement flag */
#  endif
#endif /* ZIPINFO */





/**************/
/*  Typedefs  */
/**************/

#ifndef _BULL_SOURCE                /* Bull has it defined somewhere already */
   typedef unsigned char  byte;     /* code assumes UNSIGNED bytes */
#endif /* !_BULL_SOURCE */

typedef char              boolean;
typedef long              longint;
typedef unsigned short    UWORD;
typedef unsigned long     ULONG;
typedef unsigned char     uch;
typedef unsigned short    ush;
typedef unsigned long     ulg;

typedef struct min_info {
    unsigned unix_attr;
    unsigned dos_attr;
    int hostnum;
    longint offset;
    ULONG compr_size;        /* compressed size (needed if extended header) */
    ULONG crc;               /* crc (needed if extended header) */
    unsigned encrypted : 1;  /* file encrypted: decrypt before uncompressing */
    unsigned ExtLocHdr : 1;  /* use time instead of CRC for decrypt check */
    unsigned text : 1;       /* file is text or binary */
    unsigned lcflag : 1;     /* convert filename to lowercase */
} min_info;

typedef struct VMStimbuf {
    char *revdate;           /* (both correspond to Unix modtime/st_mtime) */
    char *credate;
} VMStimbuf;

/*---------------------------------------------------------------------------
    Zipfile layout declarations.  If these headers ever change, make sure the
    xxREC_SIZE defines (above) change with them!
  ---------------------------------------------------------------------------*/

   typedef byte   local_byte_hdr[ LREC_SIZE ];
#      define L_VERSION_NEEDED_TO_EXTRACT_0     0
#      define L_VERSION_NEEDED_TO_EXTRACT_1     1
#      define L_GENERAL_PURPOSE_BIT_FLAG        2
#      define L_COMPRESSION_METHOD              4
#      define L_LAST_MOD_FILE_TIME              6
#      define L_LAST_MOD_FILE_DATE              8
#      define L_CRC32                           10
#      define L_COMPRESSED_SIZE                 14
#      define L_UNCOMPRESSED_SIZE               18
#      define L_FILENAME_LENGTH                 22
#      define L_EXTRA_FIELD_LENGTH              24

   typedef byte   cdir_byte_hdr[ CREC_SIZE ];
#      define C_VERSION_MADE_BY_0               0
#      define C_VERSION_MADE_BY_1               1
#      define C_VERSION_NEEDED_TO_EXTRACT_0     2
#      define C_VERSION_NEEDED_TO_EXTRACT_1     3
#      define C_GENERAL_PURPOSE_BIT_FLAG        4
#      define C_COMPRESSION_METHOD              6
#      define C_LAST_MOD_FILE_TIME              8
#      define C_LAST_MOD_FILE_DATE              10
#      define C_CRC32                           12
#      define C_COMPRESSED_SIZE                 16
#      define C_UNCOMPRESSED_SIZE               20
#      define C_FILENAME_LENGTH                 24
#      define C_EXTRA_FIELD_LENGTH              26
#      define C_FILE_COMMENT_LENGTH             28
#      define C_DISK_NUMBER_START               30
#      define C_INTERNAL_FILE_ATTRIBUTES        32
#      define C_EXTERNAL_FILE_ATTRIBUTES        34
#      define C_RELATIVE_OFFSET_LOCAL_HEADER    38

   typedef byte   ec_byte_rec[ ECREC_SIZE+4 ];
/*     define SIGNATURE                         0   space-holder only */
#      define NUMBER_THIS_DISK                  4
#      define NUM_DISK_WITH_START_CENTRAL_DIR   6
#      define NUM_ENTRIES_CENTRL_DIR_THS_DISK   8
#      define TOTAL_ENTRIES_CENTRAL_DIR         10
#      define SIZE_CENTRAL_DIRECTORY            12
#      define OFFSET_START_CENTRAL_DIRECTORY    16
#      define ZIPFILE_COMMENT_LENGTH            20


   typedef struct local_file_header {                 /* LOCAL */
       byte version_needed_to_extract[2];
       UWORD general_purpose_bit_flag;
       UWORD compression_method;
       UWORD last_mod_file_time;
       UWORD last_mod_file_date;
       ULONG crc32;
       ULONG compressed_size;
       ULONG uncompressed_size;
       UWORD filename_length;
       UWORD extra_field_length;
   } local_file_hdr;

   typedef struct central_directory_file_header {     /* CENTRAL */
       byte version_made_by[2];
       byte version_needed_to_extract[2];
       UWORD general_purpose_bit_flag;
       UWORD compression_method;
       UWORD last_mod_file_time;
       UWORD last_mod_file_date;
       ULONG crc32;
       ULONG compressed_size;
       ULONG uncompressed_size;
       UWORD filename_length;
       UWORD extra_field_length;
       UWORD file_comment_length;
       UWORD disk_number_start;
       UWORD internal_file_attributes;
       ULONG external_file_attributes;
       ULONG relative_offset_local_header;
   } cdir_file_hdr;

   typedef struct end_central_dir_record {            /* END CENTRAL */
       UWORD number_this_disk;
       UWORD num_disk_with_start_central_dir;
       UWORD num_entries_centrl_dir_ths_disk;
       UWORD total_entries_central_dir;
       ULONG size_central_directory;
       ULONG offset_start_central_directory;
       UWORD zipfile_comment_length;
   } ecdir_rec;





/*************************/
/*  Function Prototypes  */
/*************************/

#ifndef __
#  define __   OF
#endif

/*---------------------------------------------------------------------------
    Decompression functions:
  ---------------------------------------------------------------------------*/

int    explode ();
void   unReduce ();
void   unShrink ();

/*---------------------------------------------------------------------------
    Functions in file_io.c and crypt.c:
  ---------------------------------------------------------------------------*/

int    FillBitBuffer             ();
int    ReadByte                  __((UWORD *x));                /* file_io.c */

/************/
/*  Macros  */
/************/

#ifndef MAX
#  define MAX(a,b)   ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#  define MIN(a,b)   ((a) < (b) ? (a) : (b))
#endif


#define LSEEK(abs_offset) {longint request=(abs_offset)+extra_bytes,\
   inbuf_offset=request%INBUFSIZ, bufstart=request-inbuf_offset;\
   if(request<0) {fprintf(stderr, SeekMsg, ReportMsg); return(3);}\
   else if(bufstart!=cur_zipfile_bufstart)\
   {cur_zipfile_bufstart=lseek(zipfd,bufstart,SEEK_SET);\
   if((incnt=read(zipfd,(char *)inbuf,INBUFSIZ))<=0) return(51);\
   inptr=inbuf+(int)inbuf_offset; incnt-=(int)inbuf_offset;} else\
   {incnt+=(inptr-inbuf)-(int)inbuf_offset; inptr=inbuf+(int)inbuf_offset;}}

/*
 *  Seek to the block boundary of the block which includes abs_offset,
 *  then read block into input buffer and set pointers appropriately.
 *  If block is already in the buffer, just set the pointers.  This macro
 *  is used by process_end_central_dir (unzip.c) and do_string (misc.c).
 *  A slightly modified version is embedded within extract_or_test_files
 *  (unzip.c).  ReadByte and readbuf (file_io.c) are compatible.
 *
 *  macro LSEEK(abs_offset)
 *      ULONG   abs_offset;
 *  {
 *      longint   request = abs_offset + extra_bytes;
 *      longint   inbuf_offset = request % INBUFSIZ;
 *      longint   bufstart = request - inbuf_offset;
 *
 *      if (request < 0) {
 *          fprintf(stderr, SeekMsg, ReportMsg);
 *          return(3);             /-* 3:  severe error in zipfile *-/
 *      } else if (bufstart != cur_zipfile_bufstart) {
 *          cur_zipfile_bufstart = lseek(zipfd, bufstart, SEEK_SET);
 *          if ((incnt = read(zipfd,inbuf,INBUFSIZ)) <= 0)
 *              return(51);        /-* 51:  unexpected EOF *-/
 *          inptr = inbuf + (int)inbuf_offset;
 *          incnt -= (int)inbuf_offset;
 *      } else {
 *          incnt += (inptr-inbuf) - (int)inbuf_offset;
 *          inptr = inbuf + (int)inbuf_offset;
 *      }
 *  }
 *
 */


#define SKIP_(length) if(length&&((error=do_string(length,SKIP))!=0))\
  {error_in_archive=error; if(error>1) return error;}

/*
 *  Skip a variable-length field, and report any errors.  Used in zipinfo.c
 *  and unzip.c in several functions.
 *
 *  macro SKIP_(length)
 *      UWORD   length;
 *  {
 *      if (length && ((error = do_string(length, SKIP)) != 0)) {
 *          error_in_archive = error;   /-* might be warning *-/
 *          if (error > 1)              /-* fatal *-/
 *              return (error);
 *      }
 *  }
 *
 */

#define READBIT(nbits,zdest) {if(nbits>bits_left) FillBitBuffer();\
  zdest=(int)((UWORD)bitbuf&mask_bits[nbits]);bitbuf>>=nbits;bits_left-=nbits;}

/*
 * macro READBIT(nbits,zdest)
 *  {
 *      if (nbits > bits_left)
 *          FillBitBuffer();
 *      zdest = (int)((UWORD)bitbuf & mask_bits[nbits]);
 *      bitbuf >>= nbits;
 *      bits_left -= nbits;
 *  }
 *
 */


#define PEEKBIT(nbits) (nbits>bits_left? (FillBitBuffer(),\
  (UWORD)bitbuf & mask_bits[nbits]) : (UWORD)bitbuf & mask_bits[nbits])


#define NUKE_CRs(buf,len) {register int i,j; for (i=j=0; j<len;\
  (buf)[i++]=(buf)[j++]) if ((buf)[j]=='\r') ++j; len=i;}

/*
 *  Remove all the ASCII carriage returns from buffer buf (length len),
 *  shortening as necessary (note that len gets modified in the process,
 *  so it CANNOT be an expression).  This macro is intended to be used
 *  BEFORE A_TO_N(); hence the check for CR instead of '\r'.  NOTE:  The
 *  if-test gets performed one time too many, but it doesn't matter.
 *
 *  macro NUKE_CRs(buf,len)
 *  {
 *      register int   i, j;
 *
 *      for (i = j = 0;  j < len;  (buf)[i++] = (buf)[j++])
 *          if ((buf)[j] == CR)
 *              ++j;
 *      len = i;
 *  }
 *
 */


#define TOLOWER(str1,str2) {char *ps1,*ps2; ps1=(str1)-1; ps2=(str2);\
  while(*++ps1) *ps2++=(char)(isupper((int)(*ps1))?tolower((int)(*ps1)):*ps1);\
  *ps2='\0';}

/*
 *  Copy the zero-terminated string in str1 into str2, converting any
 *  uppercase letters to lowercase as we go.  str2 gets zero-terminated
 *  as well, of course.  str1 and str2 may be the same character array.
 *
 *  macro TOLOWER( str1, str2 )
 *  {
 *      char  *ps1, *ps2;
 *
 *      ps1 = (str1) - 1;
 *      ps2 = (str2);
 *      while (*++ps1)
 *          *ps2++ = (char)(isupper((int)(*ps1))? tolower((int)(*ps1)) : *ps1);
 *      *ps2='\0';
 *  }
 *
 *  NOTES:  This macro makes no assumptions about the characteristics of
 *    the tolower() function or macro (beyond its existence), nor does it
 *    make assumptions about the structure of the character set (i.e., it
 *    should work on EBCDIC machines, too).  The fact that either or both
 *    of isupper() and tolower() may be macros has been taken into account;
 *    watch out for "side effects" (in the C sense) when modifying this
 *    macro.
 */


#ifndef ascii_to_native
#  define ascii_to_native(c)   (c)
#  define A_TO_N(str1)
#else
#  ifndef NATIVE
#    define NATIVE     "native chars"
#  endif
#  define A_TO_N(str1) {register unsigned char *ps1;\
     for (ps1=str1; *ps1; ps1++) *ps1=ascii_to_native(*ps1);}
#endif

/*
 *  Translate the zero-terminated string in str1 from ASCII to the native
 *  character set. The translation is performed in-place and uses the
 *  ascii_to_native macro to translate each character.
 *
 *  macro A_TO_N( str1 )
 *  {
 *      register unsigned char *ps1;
 *
 *      for (ps1 = str1;  *ps1;  ++ps1)
 *          *ps1 = ascii_to_native(*ps1);
 *  }
 *
 *  NOTE:  Using the ascii_to_native macro means that is it the only part of
 *    unzip which knows which translation table (if any) is actually in use
 *    to produce the native character set.  This makes adding new character
 *    set translation tables easy, insofar as all that is needed is an
 *    appropriate ascii_to_native macro definition and the translation
 *    table itself.  Currently, the only non-ASCII native character set
 *    implemented is EBCDIC, but this may not always be so.
 */

/*************/
/*  Globals  */
/*************/

#ifdef MACOS
   union work {
     struct {
       short *Prefix_of;        /* (8193 * sizeof(short)) */
       byte *Suffix_of;
       byte *Stack;
     } shrink;
     byte *Slide;
   };
#else
   union work {
     struct {
       short Prefix_of[HSIZE + 2];      /* (8194 * sizeof(short)) */
       byte Suffix_of[HSIZE + 2];       /* also s-f length_nodes (smaller) */
       byte Stack[HSIZE + 2];           /* also s-f distance_nodes (smaller) */
     } shrink;
     byte Slide[WSIZE];
   };
#endif
   extern union work area;

#  define prefix_of area.shrink.Prefix_of
#  define suffix_of area.shrink.Suffix_of
#  define stack area.shrink.Stack
#  define slide area.Slide

   extern ULONG     crc32val;
   extern UWORD     mask_bits[];
   extern int       bits_left;
   extern ULONG     bitbuf;
   extern boolean   zipeof;

