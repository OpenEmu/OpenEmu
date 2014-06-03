//
//  NSDocument+INAppStoreWindowFixes.h
//  SampleApp
//
//  Created by Indragie Karunaratne on 1/5/2014.
//  Copyright (c) 2014 Indragie Karunaratne. All rights reserved.
//
//  Licensed under the BSD 2-clause License. See LICENSE file distributed in the source
//  code of this project.
//

#import <Cocoa/Cocoa.h>

// Fix for the issue described here: https://github.com/indragiek/INAppStoreWindow/issues/91
//
// Including this category in your project will swizzle NSDocument -updateChangeCount: such
// that any INAppStoreWindow instances belonging to the document's window controllers will
// automatically relayout their title bars when the change count is updated.
@interface NSDocument (INAppStoreWindowFixes)
@end
