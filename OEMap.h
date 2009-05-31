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

OE_EXTERN OEMapRef OEMapCreate(size_t capacity);
OE_EXTERN void     OEMapRelease(OEMapRef map);
OE_EXTERN void     OEMapSetValue(OEMapRef map, OEMapKey key, OEMapValue value);
OE_EXTERN BOOL     OEMapGetValue(OEMapRef map, OEMapKey key, OEMapValue *value);
OE_EXTERN void     OEMapRemoveMaskedKeysForValue(OEMapRef map, OEMapKey mask, OEMapValue value);
OE_EXTERN void     OEMapSetValueComparator(OEMapRef map, BOOL (*comparator)(OEMapValue, OEMapValue));
