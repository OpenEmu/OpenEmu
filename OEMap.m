/*
 *  OEMap.c
 *  OpenEmu
 *
 *  Created by Joshua Weinberg on 5/30/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "OEMap.h"

#define CHUNK_SIZE 5

typedef struct {
	OEMapKey   key;
	OEMapValue value;
	BOOL       allocated;
} OEMapEntry;

typedef struct _OEMap {
    size_t capacity;
	size_t count;
	OEMapEntry *entries;
	BOOL (*valueIsEqual)(OEMapValue, OEMapValue);
} OEMap;

#define SET_ENTRY(entry, aKey, aValue) do { (entry)->key = (aKey), (entry)->value = (aValue), (entry)->allocated = 1; } while(0)

BOOL defaultIsEqual(OEMapValue v1, OEMapValue v2)
{
    return v1.key == v2.key && v1.player == v2.player;
}

OEMapRef OEMapCreate(size_t capacity)
{
    OEMapRef ret  = malloc(sizeof(OEMap));
    ret->count    = 0;
    ret->capacity = capacity;
    ret->entries  = malloc(sizeof(OEMapEntry) * capacity);
    ret->valueIsEqual = defaultIsEqual;
    return ret;
}

void OEMapRelease(OEMapRef map)
{
    if(map == NULL) return;
    
    if(map->capacity > 0)
        free(map->entries);
    free(map);
}

void OEMapSetValue(OEMapRef map, OEMapKey key, OEMapValue value)
{
    if(map->count != 0)
    {
        for(size_t i = 0, max = map->count; i < max; i++)
        {
            OEMapEntry *entry = &map->entries[i];
			if( entry->key == key )
			{
                entry->value = value;
                entry->allocated = YES;
				return;
			}
        }
        
        //find the next unallocated spot
		for ( int i = 0; i < map->count; i++ )
		{
            OEMapEntry *entry = &map->entries[i];
			if(entry->allocated == 0 )
			{
                SET_ENTRY(entry, key, value);
				return;
			}
		}
    }
    
    if(map->count + 1 > map->capacity)
        map->entries = realloc(map->entries, sizeof(OEMapEntry) * map->count);
    
    OEMapEntry *entry = &map->entries[map->count++];
    entry->value = value;
    entry->key   = key;
    entry->allocated = YES;
}

BOOL OEMapGetValue(OEMapRef map, OEMapKey key, OEMapValue *value)
{
    for(size_t i = 0, max = map->count; i < max; i++)
    {
        OEMapEntry *entry = &map->entries[i];
        if(entry->key == key && entry->allocated)
        {
            *value = entry->value;
            return YES;
        }
    }
    return NO;
}

void OEMapSetValueComparator(OEMapRef map, BOOL (*comparator)(OEMapValue, OEMapValue))
{
    map->valueIsEqual = comparator;
}

void OEMapRemoveMaskedKeysForValue(OEMapRef map, OEMapKey mask, OEMapValue value)
{
    for(size_t i = 0, max = map->count; i < max; i++)
    {
        OEMapEntry *entry = &map->entries[i];
        if(entry->allocated && map->valueIsEqual(value, entry->value) && entry->key & mask)
            entry->allocated = 0;
    }
}
