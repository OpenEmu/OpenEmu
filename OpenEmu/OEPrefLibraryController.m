//
//  OEPrefLibraryController.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 07.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OEPrefLibraryController.h"


@implementation OEPrefLibraryController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
    }
    
    return self;
}

- (void)dealloc{
    [super dealloc];
}
#pragma mark ViewController Overrides
- (NSString*)nibName{
	return @"OEPrefLibraryController";
}

#pragma mark OEPreferencePane Protocol
- (NSImage*)icon{
	return [NSImage imageNamed:@"tb_pref_library"];
}

- (NSString*)title{
	return @"Library";
}

- (NSSize)viewSize{
	return NSMakeSize(423, 480);
}

@end
