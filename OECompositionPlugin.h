//
//  OECompositionPlugin.h
//  OpenEmu
//
//  Created by Remy Demarest on 01/07/2009.
//  Copyright 2009 Psycho Inc.. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <Quartz/Quartz.h>

@interface OECompositionPlugin : NSObject
{
    QCComposition *composition;
    NSString      *name;
    NSString      *path;
}

+ (NSArray *)allPluginNames;
+ (NSString *)pluginFolder;
+ (NSString *)pluginExtension;

+ (id)compositionPluginWithName:(NSString *)aName;

@property(readonly) QCComposition *composition;
@property(readonly) NSString      *name;
@property(readonly) NSString      *path;
@property(readonly) NSString      *description;
@property(readonly) NSString      *copyright;
@property(readonly) BOOL           isBuiltIn;
@property(readonly) BOOL           isTimeDependent;
@property(readonly) BOOL           hasConsumers;
@property(readonly) NSString      *category;

@end
