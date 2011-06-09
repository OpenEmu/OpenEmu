//
//  OEPrefCoresController.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 07.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OEPrefCoresController.h"


@implementation OEPrefCoresController

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
- (void)awakeFromNib{
}

- (NSString*)nibName{
	return @"OEPrefCoresController";
}

#pragma mark OEPreferencePane Protocol
- (NSImage*)icon{
	return [NSImage imageNamed:@"tb_pref_cores"];
}

- (NSString*)title{
	return @"Cores";
}

- (NSSize)viewSize{
	return NSMakeSize(423, 480);
}

@end
