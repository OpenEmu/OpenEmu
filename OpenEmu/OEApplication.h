//
//  OEApplication.h
//  OpenEmu
//
//  Created by Remy Demarest on 06/08/2016.
//
//

#import <Cocoa/Cocoa.h>

NS_ASSUME_NONNULL_BEGIN

@class OEApplication;

@protocol OEApplicationDelegate <NSApplicationDelegate, NSObject>

@optional
- (void)application:(OEApplication *)application willBeginModalSessionForWindow:(NSWindow *)window;
- (void)application:(OEApplication *)application didBeginModalSessionForWindow:(NSWindow *)window;
- (void)applicationWillEndModalSession:(OEApplication *)application;
- (void)applicationDidEndModalSession:(OEApplication *)application;
- (void)spotlightStatusDidChangeForApplication:(OEApplication *)application;

@end

@interface OEApplication : NSApplication

@property (nullable, assign) id<OEApplicationDelegate> delegate;
@property (nonatomic) BOOL isSpotlightFrontmost;

@end

NS_ASSUME_NONNULL_END
