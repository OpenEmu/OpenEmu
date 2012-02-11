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

@property (nonatomic, strong) IBOutlet UIWindow *window;
@property (nonatomic, strong) IBOutlet UINavigationController *navController;
@property (nonatomic, strong) IBOutlet BrowserViewController *bonjourBrowser;

@end

