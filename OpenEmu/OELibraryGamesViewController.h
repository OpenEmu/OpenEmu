//
//  OELibraryGamesViewController.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 10/14/15.
//
//

#import <Cocoa/Cocoa.h>
#import "OELibrarySubviewController.h"

@class OESidebarController, OEGameCollectionViewController;
@interface OELibraryGamesViewController : NSViewController <OELibrarySubviewController>
@property (weak, nonatomic) OELibraryController* libraryController;
@property (weak) IBOutlet OESidebarController *sidebarController;
@property (weak) IBOutlet OEGameCollectionViewController *gameCollectionController;
@end
