//
//  INAppStoreWindowSwizzling.h
//
//  Created by Jake Petroules on 1/13/2014.
//  Copyright (c) 2011-2014 Indragie Karunaratne. All rights reserved.
//
//  Licensed under the BSD 2-clause License. See LICENSE file distributed in the source
//  code of this project.
//

#import <objc/runtime.h>

void INAppStoreWindowSwizzle(Class c, SEL orig, SEL new);
