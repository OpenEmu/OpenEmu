//
//  OELibrarySubviewController.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 25.07.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "OELibraryController.h"
@protocol OELibrarySubviewController <NSObject>

- (void)setItem:(id)item;
- (id)selectedItem;

#pragma mark - State Handling
- (id)encodeCurrentState;
- (void)restoreState:(id)state;

@optional
#pragma mark - Library Handling
- (void)setLibraryController:(OELibraryController*)controller;
- (OELibraryController*)libraryController;

@end
