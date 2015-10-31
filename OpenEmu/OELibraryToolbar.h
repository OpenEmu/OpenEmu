//
//  OELibraryToolbar.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 10/19/15.
//
//

#import <Cocoa/Cocoa.h>

@interface OELibraryToolbar : NSToolbar
@property (assign) IBOutlet NSSegmentedControl *categorySelector;
@property (assign) IBOutlet NSButton *gridViewButton;
@property (assign) IBOutlet NSButton *listViewButton;
@property (assign) IBOutlet NSSlider *gridSizeSlider;
@property (assign) IBOutlet NSButton *addButton;
@property (assign) IBOutlet NSSearchField *searchField;
@end
