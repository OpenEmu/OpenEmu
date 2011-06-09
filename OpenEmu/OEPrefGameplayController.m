//
//  OEPrefGameplayController.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 07.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OEPrefGameplayController.h"


@implementation OEPrefGameplayController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Initialization code here.
    }
    
    return self;
}

- (void)dealloc
{
    [super dealloc];
}
#pragma mark ViewController Overrides
- (NSString*)nibName{
	return @"OEPrefGameplayController";
}

#pragma mark OEPreferencePane Protocol
- (NSImage*)icon{
	return [NSImage imageNamed:@"tb_pref_gameplay"];
}

- (NSString*)title{
	return @"Gameplay";
}

- (NSSize)viewSize{
	return NSMakeSize(423, 347);
}

@end
