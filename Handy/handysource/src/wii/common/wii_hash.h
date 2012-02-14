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
#ifndef WII_HASH_H
#define WII_HASH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gctypes.h>

/*
 * Computes the hash of the specified source
 *
 * source	The source to calculate the hash for
 * length	The length of the source
 * result	The string to receive the result of the hash computation
 */
void wii_hash_compute( const u8* source, u32 length, char result[33] );

#ifdef __cplusplus
}
#endif

#endif
