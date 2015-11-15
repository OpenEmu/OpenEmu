//
//  OELibraryToolbar.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 10/19/15.
//
//

@import Cocoa;

@class OEButton;

@interface OELibraryToolbar : NSToolbar
@property (assign) IBOutlet NSSegmentedControl *categorySelector;
@property (assign) IBOutlet OEButton *gridViewButton;
@property (assign) IBOutlet OEButton *listViewButton;
@property (assign) IBOutlet NSSlider *gridSizeSlider;
@property (assign) IBOutlet NSButton *addButton;
@property (assign) IBOutlet NSSearchField *searchField;
@end
