//
//  iPhoneRemoteAppDelegate.h
//  iPhoneRemote
//
//  Created by Josh Weinberg on 9/8/10.
//  Copyright 2010 OpenEmu. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "BrowserViewController.h"

@class iPhoneRemoteViewController;

@interface iPhoneRemoteAppDelegate : NSObject <UIApplicationDelegate, BrowserViewControllerDelegate> {
    UIWindow *window;
	UINavigationController *navController;
	BrowserViewController *bonjourBrowser;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet UINavigationController *navController;
@property (nonatomic, retain) IBOutlet BrowserViewController *bonjourBrowser;

@end

