//
//  OEFeaturedGamesViewController.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 09/07/14.
//
//

#import <Cocoa/Cocoa.h>
#import <WebKit/WebKit.h>

#import "OELibrarySubviewController.h"
#import "OEFeaturedGamesView.h"

@interface OEFeaturedGamesViewController : NSViewController <OELibrarySubviewController>
@property (nonatomic) OELibraryController *libraryController;
@property (assign) IBOutlet WebView *webView;
@end
