//
//  OEMainWindowToolbarViewController.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 06.12.12.
//
//

#import <Cocoa/Cocoa.h>

@interface OEMainWindowToolbarViewController : NSViewController

- (void)disableAllItems;

@property (strong) IBOutlet NSButton      *toolbarSidebarButton;
@property (strong) IBOutlet NSButton      *toolbarGridViewButton;
@property (strong) IBOutlet NSButton      *toolbarFlowViewButton;
@property (strong) IBOutlet NSButton      *toolbarListViewButton;

@property (strong) IBOutlet NSButton      *toolbarAddToSidebarButton;
@property (strong) IBOutlet NSSearchField *toolbarSearchField;
@property (strong) IBOutlet NSSlider      *toolbarSlider;
@end
