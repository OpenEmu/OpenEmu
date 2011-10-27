//
//  OETableView.h
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 03.04.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface OETableView : NSTableView 
{
@private
	NSColor *selectionColor;
}

@property (retain, readwrite) NSColor *selectionColor;
@end
