/*
 *  OEMap.c
 *  OpenEmu
 *
 *  Created by Joshua Weinberg on 5/30/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "OEMap.h"

void OEMapEntry_InitializeEntry( OEMapEntry *entry, KEY_TYPE key, VALUE_TYPE value )
{
	entry->key = key;
	entry->value = value;
	entry->allocated = 1;
}

void OEMap_InitializeMap( OEMap *map, int(*value_compare)(VALUE_TYPE, VALUE_TYPE))
{
	map->count = 0;
	map->entries = 0;
	map->value_compare = value_compare;
}

void OEMap_SetKey( OEMap *map, KEY_TYPE key, VALUE_TYPE value)
{
	if( map->count )
	{
		for ( int i = 0; i < map->count; i++ )
		{
			OEMapEntry *entry = &map->entries[i];
			if( entry->key == key )
			{
				entry->allocated = 1;
				entry->value = value;
				return;
			}
		}
		
		//find the next unallocated spot
		for ( int i = 0; i < map->count; i++ )
		{
			OEMapEntry *entry = &map->entries[i];
			if( entry->allocated == 0 )
			{
				entry->allocated = 1;
				entry->value = value;
				entry->key = key;
				return;
			}
		}
	}
	
	//Key wasn't found
	map->count = map->count+1;
	map->entries = realloc(map->entries, sizeof(OEMapEntry) * map->count);
	
	OEMapEntry newEntry;
	OEMapEntry_InitializeEntry(&newEntry, key, value);
	
	map->entries[map->count - 1] = newEntry;
}

int OEMap_ValueForKey( const OEMap *map, KEY_TYPE key, VALUE_TYPE* value)
{

	if( map->count )
	{
		for ( int i = 0; i < map->count; i++ )
		{
			OEMapEntry *entry = &map->entries[i];
			if( entry->key == key  && entry->allocated)
			{
				*value = entry->value;
				return 1;
			}
		}
	}
	return 0;
}

void OEMap_ClearMaskedKeysForValue( OEMap *map, VALUE_TYPE value, KEY_TYPE mask )
{
	if( map->count )
	{
		for( int i = 0; i < map->count; i++ )
		{
			OEMapEntry *entry = &map->entries[i];
			if(entry->allocated && map->value_compare( value, entry->value ) && entry->key & mask)
			{
				entry->allocated = 0;
			}
		}
	}
}

void OEMap_DestroyMap( OEMap *map )
{
	if(map->count)
		free(map->entries);
	map->count = 0;
}