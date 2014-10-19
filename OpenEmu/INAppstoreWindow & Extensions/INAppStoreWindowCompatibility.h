//
//  INAppStoreWindowCompatibility.h
//  INAppStoreWindow
//
//  Copyright (c) 2011-2014 Indragie Karunaratne. All rights reserved.
//  Copyright (c) 2014 Petroules Corporation. All rights reserved.
//
//  Licensed under the BSD 2-clause License. See LICENSE file distributed in the source
//  code of this project.
//

#import <Cocoa/Cocoa.h>

#ifndef NSAppKitVersionNumber10_7
#define NSAppKitVersionNumber10_7 1138
#endif

#ifndef NSAppKitVersionNumber10_9
#define NSAppKitVersionNumber10_9 1265
#endif

#ifndef NSAppKitVersionNumber10_10
#define NSAppKitVersionNumber10_10 1343
#endif

#if __MAC_OS_X_VERSION_MAX_ALLOWED < 1070
enum { NSWindowDocumentVersionsButton = 6, NSWindowFullScreenButton = 7 };
enum { NSFullScreenWindowMask = 1 << 14 };
extern NSString * const NSAccessibilityFullScreenButtonSubrole;
extern NSString * const NSWindowWillEnterFullScreenNotification;
extern NSString * const NSWindowDidEnterFullScreenNotification;
extern NSString * const NSWindowWillExitFullScreenNotification;
extern NSString * const NSWindowDidExitFullScreenNotification;
extern NSString * const NSWindowWillEnterVersionBrowserNotification;
extern NSString * const NSWindowDidEnterVersionBrowserNotification;
extern NSString * const NSWindowWillExitVersionBrowserNotification;
extern NSString * const NSWindowDidExitVersionBrowserNotification;
#endif

#if __MAC_OS_X_VERSION_MAX_ALLOWED < 1080
@interface NSDictionary (INNSDictionaryBackwardCompatibility)

// NS_AVAILABLE(10_8, 6_0), but this is a lie - this responds in 10.7
- (id)objectForKeyedSubscript:(id)aKey;

@end

@interface NSMutableDictionary (INNSMutableDictionaryBackwardCompatibility)

// NS_AVAILABLE(10_8, 6_0), but this is a lie - this responds in 10.7
- (void)setObject:(id)obj forKeyedSubscript:(id <NSCopying>)key;

@end
#endif

#if __MAC_OS_X_VERSION_MAX_ALLOWED < 1090
@interface NSArray (INNSArrayBackwardCompatibility)
- (id)firstObject; // not present in the 10.8 SDK but available since 10.6
@end

@interface NSData (INNSDataBackwardCompatibility)
- (id)initWithBase64Encoding:(NSString *)base64String;
@end
#endif

NS_INLINE bool INRunningLion() {
	return (NSInteger)NSAppKitVersionNumber >= NSAppKitVersionNumber10_7;
}

NS_INLINE bool INRunningMavericks() {
	return (NSInteger)NSAppKitVersionNumber >= NSAppKitVersionNumber10_9;
}

NS_INLINE bool INRunningYosemite() {
	return (NSInteger)NSAppKitVersionNumber >= NSAppKitVersionNumber10_10;
}
