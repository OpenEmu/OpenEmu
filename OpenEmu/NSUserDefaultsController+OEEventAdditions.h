//
//  NSUserDefaultsController+OEEventAdditions.h
//  OpenEmu
//
//  Created by Remy Demarest on 17/04/2011.
//  Copyright 2011 NuLayer Inc. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface NSUserDefaultsController (OEEventAdditions)
// FIXME: Is this method that useful?
- (id)eventValueForKeyPath:(NSString *)keyPath;
@end
