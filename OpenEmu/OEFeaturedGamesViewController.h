//
//  OEFeaturedGamesViewController.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 09/07/14.
//
//

#import <Cocoa/Cocoa.h>
#import "OELibrarySubviewController.h"

@interface OEFeaturedGamesViewController : NSViewController <OELibrarySubviewController>
@property (nonatomic) OELibraryController *libraryController;
@end
