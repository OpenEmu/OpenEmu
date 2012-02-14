/*
Copyright (C) 2010
raz0red (www.twitchasylum.com)

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1.	The origin of this software must not be misrepresented; you
must not claim that you wrote the original software. If you use
this software in a product, an acknowledgment in the product
documentation would be appreciated but is not required.

2.	Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3.	This notice may not be removed or altered from any source
distribution.
*/

#ifndef WII_FILE_IO_H
#define WII_FILE_IO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gctypes.h>

/*
 * Unmounts the file system
 */
extern void wii_unmount();

/*
 * Mounts the file system
 *
 * return   Whether we mounted the file system successfully
 */
extern BOOL wii_mount();

/*
 * Remounts the file system
 */
extern void wii_remount();

#ifdef __cplusplus
}
#endif

#endif
