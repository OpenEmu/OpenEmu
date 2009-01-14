//
//  main.m
//  OpenEmu
//
//  Created by Josh Weinberg on 9/1/08.
//  Copyright __MyCompanyName__ 2008 . All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "GameApp.h"

//Need a custom app override for the custom input loop
int GameMain(int argc, const char *argv[]) {
	
    [GameApp sharedApplication];
    [NSBundle loadNibNamed:@"MainMenu" owner:NSApp];
    [NSApp run];
	return 0;
}

int main(int argc, char *argv[])
{
    return GameMain(argc, (const char **) argv);
}

