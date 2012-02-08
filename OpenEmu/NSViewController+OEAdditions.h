//
//  NSViewController+OEAdditions.h
//  OpenEmu
//
//  Created by Remy Demarest on 07/02/2012.
//  Copyright (c) 2012 NuLayer Inc. All rights reserved.
//

#import <AppKit/AppKit.h>

@interface NSViewController (OEAdditions)

- (void)viewWillLoad;
- (void)viewDidLoad;

- (void)viewWillAppear;
- (void)viewDidAppear;

- (void)viewWillDisappear;
- (void)viewDidDisappear;

@end
