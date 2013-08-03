//
//  OEControlsSectionTitleView.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 7/29/13.
//
//

#import <Cocoa/Cocoa.h>

@interface OEControlsSectionTitleView : NSView
@property (copy) NSString *stringValue;
@property (nonatomic, readonly) NSInteger state;
@property (getter=isCollapisble) BOOL collapsible;
@property (nonatomic, getter=isPinned) BOOL pinned;

@property (assign) id target;
@property SEL action;
@end
