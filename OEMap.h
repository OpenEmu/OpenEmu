/*
 *  OEMap.h
 *  OpenEmu
 *
 *  Created by Joshua Weinberg on 5/30/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#import <Foundation/Foundation.h>
#import "GameCore.h"

#define KEY_TYPE NSInteger
#define VALUE_TYPE OEEmulatorKey

typedef struct {
	KEY_TYPE key;
	VALUE_TYPE value;
	int allocated;
} OEMapEntry;

typedef struct {
	int (*value_compare)(VALUE_TYPE, VALUE_TYPE);
	OEMapEntry *entries;
	int count;
} OEMap;

void OEMap_InitializeMap( OEMap *map, int(*value_compare)(VALUE_TYPE, VALUE_TYPE));
void OEMap_DestroyMap( OEMap *map );
void OEMap_SetKey( OEMap *map, KEY_TYPE key, VALUE_TYPE value);
int OEMap_ValueForKey( const OEMap *map, KEY_TYPE key, VALUE_TYPE* value);
void OEMap_ClearMaskedKeysForValue( OEMap *map, VALUE_TYPE value, KEY_TYPE mask );
