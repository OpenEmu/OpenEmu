//
//  NSDocument+INAppStoreWindowFixes.m
//  SampleApp
//
//  Created by Indragie Karunaratne on 1/5/2014.
//  Copyright (c) 2014 Indragie Karunaratne. All rights reserved.
//
//  Licensed under the BSD 2-clause License. See LICENSE file distributed in the source
//  code of this project.
//

#import "NSDocument+INAppStoreWindowFixes.h"
#import "INAppStoreWindow.h"
#import "INAppStoreWindowSwizzling.h"

@interface INAppStoreWindow (Private)
- (void)_layoutTrafficLightsAndContent;
@end

@implementation NSDocument (INAppStoreWindowFixes)

+ (void)load
{
    INAppStoreWindowSwizzle(self, @selector(updateChangeCount:), @selector(ind_updateChangeCount:));
}

- (void)ind_updateChangeCount:(NSDocumentChangeType)changeType
{
    [self ind_updateChangeCount:changeType]; // Original implementation
    NSArray *windowControllers = self.windowControllers;
    for (NSWindowController *wc in windowControllers) {
        if ([wc.window isKindOfClass:INAppStoreWindow.class]) {
            [(INAppStoreWindow *)wc.window _layoutTrafficLightsAndContent];
        }
    }
}

@end
