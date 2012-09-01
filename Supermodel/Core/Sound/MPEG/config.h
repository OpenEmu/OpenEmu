/**
 ** Supermodel
 ** A Sega Model 3 Arcade Emulator.
 ** Copyright 2011 Bart Trzynadlowski, Nik Henson
 **
 ** This file is part of Supermodel.
 **
 ** Supermodel is free software: you can redistribute it and/or modify it under
 ** the terms of the GNU General Public License as published by the Free 
 ** Software Foundation, either version 3 of the License, or (at your option)
 ** any later version.
 **
 ** Supermodel is distributed in the hope that it will be useful, but WITHOUT
 ** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 ** FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 ** more details.
 **
 ** You should have received a copy of the GNU General Public License along
 ** with Supermodel.  If not, see <http://www.gnu.org/licenses/>.
 **/
 
/*
 * config.h
 * 
 * Amp library internal header file.
 */


/* Define to empty if the keyword does not work.  */
/* #undef const */

/* Define if you don't have vprintf but do have _doprnt.  */
/* #undef HAVE_DOPRNT */

/* Define if you have <sys/wait.h> that is POSIX.1 compatible.  */
//#define HAVE_SYS_WAIT_H 1

/* Define if you have the vprintf function.  */
//#define HAVE_VPRINTF 1

/* Define as __inline if that's what the C compiler calls it.  */
/* #undef inline */

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define if you can safely include both <sys/time.h> and <time.h>.  */
//#define TIME_WITH_SYS_TIME 1

/* Define if you have the mlock function.  */
//#define HAVE_MLOCK 1

/* Define if you have the mlockall function.  */
//#define HAVE_MLOCKALL 1

/* Define if you have the sched_setscheduler function.  */
//#define HAVE_SCHED_SETSCHEDULER 1

/* Define if you have the select function.  */
//#define HAVE_SELECT 1

/* Define if you have the <dmedia/audio.h> header file.  */
/* #undef HAVE_DMEDIA_AUDIO_H */

/* Define if you have the <fcntl.h> header file.  */
//#define HAVE_FCNTL_H 1

/* Define if you have the <linux/soundcard.h> header file.  */
//#define HAVE_LINUX_SOUNDCARD_H 1

/* Define if you have the <machine/soundcard.h> header file.  */
/* #undef HAVE_MACHINE_SOUNDCARD_H */

/* Define if you have the <sys/audioio.h> header file.  */
/* #undef HAVE_SYS_AUDIOIO_H */

/* Define if you have the <sys/ioctl.h> header file.  */
//#define HAVE_SYS_IOCTL_H 1

/* Define if you have the <sys/select.h> header file.  */
//#define HAVE_SYS_SELECT_H 1

/* Define if you have the <sys/time.h> header file.  */
//#define HAVE_SYS_TIME_H 1

/* Define if you have the <unistd.h> header file.  */
#define HAVE_UNISTD_H 1

/* define if you want playing to use Linux realtime features */
/* #undef LINUX_REALTIME */
