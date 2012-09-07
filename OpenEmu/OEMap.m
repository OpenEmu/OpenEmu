/*
 Copyright (c) 2009, OpenEmu Team
 
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the OpenEmu Team nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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

typedef struct __OEMap {
    OSSpinLock  lock;
    size_t      capacity;
    size_t      count;
    OEMapEntry *entries;
    _OECompare  valueIsEqual;
} OEMap;

static BOOL defaultIsEqual(OEMapValue v1, OEMapValue v2)
{
    return v1.key == v2.key && v1.player == v2.player;
}

OEMapRef OEMapCreate(size_t capacity)
{
    OEMapRef ret      = malloc(sizeof(OEMap));
    ret->lock         = OS_SPINLOCK_INIT;
    ret->count        = 0;
    ret->capacity     = capacity;
    ret->entries      = malloc(sizeof(OEMapEntry) * capacity);
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

// The lock must be acquired before using this function
static void _OEMapSetValue(OEMapRef map, OEMapKey key, OEMapValue value)
{
    if(map == NULL) return;
    
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
    if(map == NULL) return;
    
    OSSpinLockLock(&map->lock);
    _OEMapSetValue(map, key, value);
    OSSpinLockUnlock(&map->lock);
}

BOOL OEMapGetValue(OEMapRef map, OEMapKey key, OEMapValue *value)
{
    if(map == NULL) return NO;
    
    BOOL ret = NO;
    OSSpinLockLock(&map->lock);
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
    OSSpinLockUnlock(&map->lock);
    return ret;
}

void OEMapRemoveValue(OEMapRef map, OEMapKey key)
{
    if(map == NULL) return;
    
    OSSpinLockLock(&map->lock);
    for(size_t i = 0, max = map->count; i < max; i++)
    {
        OEMapEntry *entry = &map->entries[i];
        if(entry->allocated && entry->key == key)
        {
            entry->allocated = NO;
            break;
        }
    }
    OSSpinLockUnlock(&map->lock);
}

void OEMapSetValueComparator(OEMapRef map, BOOL (*comparator)(OEMapValue, OEMapValue))
{
    if(map == NULL) return;
    
    OSSpinLockLock(&map->lock);
    map->valueIsEqual = comparator;
    OSSpinLockUnlock(&map->lock);
}

void OEMapRemoveMaskedKeysForValue(OEMapRef map, OEMapKey mask, OEMapValue value)
{
    if(map == NULL) return;
    
    OSSpinLockLock(&map->lock);
    for(size_t i = 0, max = map->count; i < max; i++)
    {
        OEMapEntry *entry = &map->entries[i];
        if(entry->allocated && map->valueIsEqual(value, entry->value) && entry->key & mask)
            entry->allocated = NO;
    }
    OSSpinLockUnlock(&map->lock);
}

#if 0
void OEMapShowOffContent(OEMapRef map)
{
    if(map == NULL) return;
    
    NSLog(@"Count = %zu", map->count);
    for(size_t i = 0, max = map->count; i < max; i++)
    {
        OEMapEntry *entry = &map->entries[i];
        NSLog(@"entry[%zu] = { .allocated = %s, .key = %d, .value = { .key = %d, .player = %d } }", i, BOOL_STR(entry->allocated), entry->key, entry->value.key, entry->value.player);
    }
}
#endif
