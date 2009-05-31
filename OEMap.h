/*
 *  OEMap.h
 *  OpenEmu
 *
 *  Created by Joshua Weinberg on 5/30/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#import <Foundation/Foundation.h>
//#import "GameCore.h"

typedef struct OEEmulatorKey {
    NSUInteger player;
    NSUInteger key;
} OEEmulatorKey;

#if !defined(OE_INLINE)
#if defined(__GNUC__)
#define OE_INLINE static __inline__ __attribute__((always_inline))
#elif defined(__cplusplus)
#define OE_INLINE static inline
#endif
#endif

OE_INLINE OEEmulatorKey OEMakeEmulatorKey(NSUInteger player, NSUInteger key)
{
    OEEmulatorKey ret;
    ret.player = player;
    ret.key = key;
    return ret;
}

typedef NSInteger     OEMapKey;
typedef OEEmulatorKey OEMapValue;

typedef struct _OEMap *OEMapRef;

OEMapRef OEMapCreate(size_t capacity);
void OEMapRelease(OEMapRef map);
void OEMapSetValue(OEMapRef map, OEMapKey key, OEMapValue value);
BOOL OEMapGetValue(OEMapRef map, OEMapKey key, OEMapValue *value);
void OEMapRemoveMaskedKeysForValue(OEMapRef map, OEMapKey mask, OEMapValue value);
void OEMapSetValueComparator(OEMapRef map, BOOL (*comparator)(OEMapValue, OEMapValue));
void OEMapShowOffContent(OEMapRef map);
