/*
 * CRC.h
 *
 * Copyright (c) 2017-present, MacPaw Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301  USA
 */
#import <Foundation/Foundation.h>

uint32_t XADCRC(uint32_t prevcrc,uint8_t byte,const uint32_t *table);
uint32_t XADCalculateCRC(uint32_t prevcrc,const uint8_t *buffer,int length,const uint32_t *table);

// Version approximately 2x (up to 6x times) faster
// This one expects 16x256 tables passed in at the moment only XADCRCTable_sliced16_edb88320 is pregenerated
uint32_t XADCalculateCRCFast(uint32_t prevcrc,const uint8_t *buffer,int length, const uint32_t (*table)[256]);

uint64_t XADCRC64(uint64_t prevcrc,uint8_t byte,const uint64_t *table);
uint64_t XADCalculateCRC64(uint64_t prevcrc,const uint8_t *buffer,int length,const uint64_t *table);

int XADUnReverseCRC16(int val);

extern const uint32_t XADCRCTable_a001[256];
extern const uint32_t XADCRCTable_sliced16_edb88320[16][256];
extern const uint32_t XADCRCReverseTable_1021[256];
extern const uint32_t XADCRCTable_edb88320[256];
extern const uint64_t XADCRCTable_c96c5795d7870f42[256];
