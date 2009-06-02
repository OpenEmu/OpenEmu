/*
 *  OEMap.c
 *  OpenEmu
 *
 *  Created by Joshua Weinberg on 5/30/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "OEMap.h"

#ifndef BOOL_STR
#define BOOL_STR(var) ((var) ? "YES" : "NO")
#endif

typedef struct {
	OEMapKey    key;
	OEMapValue  value;
	BOOL        allocated;
} OEMapEntry;

typedef BOOL (*_OECompare)(OEMapValue, OEMapValue);

typedef struct _OEMap {
    size_t      capacity;
	size_t      count;
	OEMapEntry *entries;
    NSLock     *lock;
	_OECompare  valueIsEqual;
} OEMap;

BOOL defaultIsEqual(OEMapValue v1, OEMapValue v2)
{
    return v1.key == v2.key && v1.player == v2.player;
}

OEMapRef OEMapCreate(size_t capacity)
{
    OEMapRef ret      = malloc(sizeof(OEMap));
    ret->count        = 0;
    ret->capacity     = capacity;
    ret->entries      = malloc(sizeof(OEMapEntry) * capacity);
    ret->valueIsEqual = defaultIsEqual;
    ret->lock         = [[NSLock alloc] init];
    return ret;
}

void OEMapRelease(OEMapRef map)
{
    if(map == NULL) return;
    
    [map->lock release];
    if(map->capacity > 0)
        free(map->entries);
    free(map);
}

// The lock must be acquired before using this function
static void _OEMapSetValue(OEMapRef map, OEMapKey key, OEMapValue value)
{
    if(map->count != 0)
    {
        for(size_t i = 0, max = map->count; i < max; i++)
        {
            OEMapEntry *entry = &map->entries[i];
			if(entry->key == key)
			{
                entry->value = value;
                entry->allocated = YES;
				return;
			}
        }
        
        //find the next unallocated spot
		for (int i = 0; i < map->count; i++)
		{
            OEMapEntry *entry = &map->entries[i];
			if(!entry->allocated)
			{
                entry->key = key;
                entry->value = value;
                entry->allocated = YES;
				return;
			}
		}
    }
    
    if(map->count + 1 > map->capacity)
    {
        map->capacity = map->capacity * 2;
        map->entries = realloc(map->entries, sizeof(OEMapEntry) * map->capacity);
    }
    
    OEMapEntry *entry = &map->entries[map->count++];
    entry->value = value;
    entry->key   = key;
    entry->allocated = YES;
}

void OEMapSetValue(OEMapRef map, OEMapKey key, OEMapValue value)
{
    [map->lock lock]; 
    _OEMapSetValue(map, key, value);
    [map->lock unlock];
}

BOOL OEMapGetValue(OEMapRef map, OEMapKey key, OEMapValue *value)
{
    BOOL ret = NO;
    [map->lock lock];
    for(size_t i = 0, max = map->count; i < max; i++)
    {
        OEMapEntry *entry = &map->entries[i];
        if(entry->allocated && entry->key == key)
        {
            *value = entry->value;
            ret = YES;
            break;
        }
    }
    [map->lock unlock];
    return ret;
}

void OEMapSetValueComparator(OEMapRef map, BOOL (*comparator)(OEMapValue, OEMapValue))
{
    [map->lock lock];
    map->valueIsEqual = comparator;
    [map->lock unlock];
}

void OEMapRemoveMaskedKeysForValue(OEMapRef map, OEMapKey mask, OEMapValue value)
{
    [map->lock lock];
    for(size_t i = 0, max = map->count; i < max; i++)
    {
        OEMapEntry *entry = &map->entries[i];
        if(entry->allocated && map->valueIsEqual(value, entry->value) && entry->key & mask)
            entry->allocated = NO;
    }
    [map->lock unlock];
}

void OEMapShowOffContent(OEMapRef map)
{
    NSLog(@"Count = %d", map->count);
    for(size_t i = 0, max = map->count; i < max; i++)
    {
        OEMapEntry *entry = &map->entries[i];
        NSLog(@"entry[%d] = { .allocated = %s, .key = %d, .value = { .key = %d, .player = %d } }", i, BOOL_STR(entry->allocated), entry->key, entry->value.key, entry->value.player);
    }
}
