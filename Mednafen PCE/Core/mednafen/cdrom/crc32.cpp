/*  dvdisaster: Additional error correction for optical media.
 *  Copyright (C) 2004-2007 Carsten Gnoerlich.
 *  Project home page: http://www.dvdisaster.com
 *  Email: carsten@dvdisaster.com  -or-  cgnoerlich@fsfe.org
 *
 *  CRC32 code based upon public domain code by Ross Williams (see notes below)  
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA,
 *  or direct your browser at http://www.gnu.org.
 */

#include "dvdisaster.h"

/***
 *** EDC checksum used in CDROM sectors
 ***/

/*****************************************************************/
/*                                                               */
/* CRC LOOKUP TABLE                                              */
/* ================                                              */
/* The following CRC lookup table was generated automagically    */
/* by the Rocksoft^tm Model CRC Algorithm Table Generation       */
/* Program V1.0 using the following model parameters:            */
/*                                                               */
/*    Width   : 4 bytes.                                         */
/*    Poly    : 0x8001801BL                                      */
/*    Reverse : TRUE.                                            */
/*                                                               */
/* For more information on the Rocksoft^tm Model CRC Algorithm,  */
/* see the document titled "A Painless Guide to CRC Error        */
/* Detection Algorithms" by Ross Williams                        */
/* (ross@guest.adelaide.edu.au.). This document is likely to be  */
/* in the FTP archive "ftp.adelaide.edu.au/pub/rocksoft".        */
/*                                                               */
/*****************************************************************/

unsigned long  edctable[256] =
{
 0x00000000L, 0x90910101L, 0x91210201L, 0x01B00300L,
 0x92410401L, 0x02D00500L, 0x03600600L, 0x93F10701L,
 0x94810801L, 0x04100900L, 0x05A00A00L, 0x95310B01L,
 0x06C00C00L, 0x96510D01L, 0x97E10E01L, 0x07700F00L,
 0x99011001L, 0x09901100L, 0x08201200L, 0x98B11301L,
 0x0B401400L, 0x9BD11501L, 0x9A611601L, 0x0AF01700L,
 0x0D801800L, 0x9D111901L, 0x9CA11A01L, 0x0C301B00L,
 0x9FC11C01L, 0x0F501D00L, 0x0EE01E00L, 0x9E711F01L,
 0x82012001L, 0x12902100L, 0x13202200L, 0x83B12301L,
 0x10402400L, 0x80D12501L, 0x81612601L, 0x11F02700L,
 0x16802800L, 0x86112901L, 0x87A12A01L, 0x17302B00L,
 0x84C12C01L, 0x14502D00L, 0x15E02E00L, 0x85712F01L,
 0x1B003000L, 0x8B913101L, 0x8A213201L, 0x1AB03300L,
 0x89413401L, 0x19D03500L, 0x18603600L, 0x88F13701L,
 0x8F813801L, 0x1F103900L, 0x1EA03A00L, 0x8E313B01L,
 0x1DC03C00L, 0x8D513D01L, 0x8CE13E01L, 0x1C703F00L,
 0xB4014001L, 0x24904100L, 0x25204200L, 0xB5B14301L,
 0x26404400L, 0xB6D14501L, 0xB7614601L, 0x27F04700L,
 0x20804800L, 0xB0114901L, 0xB1A14A01L, 0x21304B00L,
 0xB2C14C01L, 0x22504D00L, 0x23E04E00L, 0xB3714F01L,
 0x2D005000L, 0xBD915101L, 0xBC215201L, 0x2CB05300L,
 0xBF415401L, 0x2FD05500L, 0x2E605600L, 0xBEF15701L,
 0xB9815801L, 0x29105900L, 0x28A05A00L, 0xB8315B01L,
 0x2BC05C00L, 0xBB515D01L, 0xBAE15E01L, 0x2A705F00L,
 0x36006000L, 0xA6916101L, 0xA7216201L, 0x37B06300L,
 0xA4416401L, 0x34D06500L, 0x35606600L, 0xA5F16701L,
 0xA2816801L, 0x32106900L, 0x33A06A00L, 0xA3316B01L,
 0x30C06C00L, 0xA0516D01L, 0xA1E16E01L, 0x31706F00L,
 0xAF017001L, 0x3F907100L, 0x3E207200L, 0xAEB17301L,
 0x3D407400L, 0xADD17501L, 0xAC617601L, 0x3CF07700L,
 0x3B807800L, 0xAB117901L, 0xAAA17A01L, 0x3A307B00L,
 0xA9C17C01L, 0x39507D00L, 0x38E07E00L, 0xA8717F01L,
 0xD8018001L, 0x48908100L, 0x49208200L, 0xD9B18301L,
 0x4A408400L, 0xDAD18501L, 0xDB618601L, 0x4BF08700L,
 0x4C808800L, 0xDC118901L, 0xDDA18A01L, 0x4D308B00L,
 0xDEC18C01L, 0x4E508D00L, 0x4FE08E00L, 0xDF718F01L,
 0x41009000L, 0xD1919101L, 0xD0219201L, 0x40B09300L,
 0xD3419401L, 0x43D09500L, 0x42609600L, 0xD2F19701L,
 0xD5819801L, 0x45109900L, 0x44A09A00L, 0xD4319B01L,
 0x47C09C00L, 0xD7519D01L, 0xD6E19E01L, 0x46709F00L,
 0x5A00A000L, 0xCA91A101L, 0xCB21A201L, 0x5BB0A300L,
 0xC841A401L, 0x58D0A500L, 0x5960A600L, 0xC9F1A701L,
 0xCE81A801L, 0x5E10A900L, 0x5FA0AA00L, 0xCF31AB01L,
 0x5CC0AC00L, 0xCC51AD01L, 0xCDE1AE01L, 0x5D70AF00L,
 0xC301B001L, 0x5390B100L, 0x5220B200L, 0xC2B1B301L,
 0x5140B400L, 0xC1D1B501L, 0xC061B601L, 0x50F0B700L,
 0x5780B800L, 0xC711B901L, 0xC6A1BA01L, 0x5630BB00L,
 0xC5C1BC01L, 0x5550BD00L, 0x54E0BE00L, 0xC471BF01L,
 0x6C00C000L, 0xFC91C101L, 0xFD21C201L, 0x6DB0C300L,
 0xFE41C401L, 0x6ED0C500L, 0x6F60C600L, 0xFFF1C701L,
 0xF881C801L, 0x6810C900L, 0x69A0CA00L, 0xF931CB01L,
 0x6AC0CC00L, 0xFA51CD01L, 0xFBE1CE01L, 0x6B70CF00L,
 0xF501D001L, 0x6590D100L, 0x6420D200L, 0xF4B1D301L,
 0x6740D400L, 0xF7D1D501L, 0xF661D601L, 0x66F0D700L,
 0x6180D800L, 0xF111D901L, 0xF0A1DA01L, 0x6030DB00L,
 0xF3C1DC01L, 0x6350DD00L, 0x62E0DE00L, 0xF271DF01L,
 0xEE01E001L, 0x7E90E100L, 0x7F20E200L, 0xEFB1E301L,
 0x7C40E400L, 0xECD1E501L, 0xED61E601L, 0x7DF0E700L,
 0x7A80E800L, 0xEA11E901L, 0xEBA1EA01L, 0x7B30EB00L,
 0xE8C1EC01L, 0x7850ED00L, 0x79E0EE00L, 0xE971EF01L,
 0x7700F000L, 0xE791F101L, 0xE621F201L, 0x76B0F300L,
 0xE541F401L, 0x75D0F500L, 0x7460F600L, 0xE4F1F701L,
 0xE381F801L, 0x7310F900L, 0x72A0FA00L, 0xE231FB01L,
 0x71C0FC00L, 0xE151FD01L, 0xE0E1FE01L, 0x7070FF00L
};

/*
 * CDROM EDC calculation
 */

uint32 EDCCrc32(const unsigned char *data, int len)
{  
 uint32 crc = 0;

 while(len--)
  crc = edctable[(crc ^ *data++) & 0xFF] ^ (crc >> 8);

 return crc;
}
