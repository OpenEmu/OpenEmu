//
//  CoreView.m
//  OEPreferencesMockup
//
//  Created by Christoph Leimbrock on 31.05.11.
//  Copyright 2011 none. All rights reserved.
//

#import "CoreView.h"
#import "NSImage+OEDrawingAdditions.h"
@interface CoreView (Priavate)
- (void)_updateContent;
@end

@implementation CoreView
#define rando(x0, x1)  ((float)(x0 + (x1 - x0) * rand() / ((float) RAND_MAX)))

#define collapsedHeight 57
#define seperatorHeight 2

#define seperatorUpperColor [NSColor colorWithDeviceWhite:0.0 alpha:1.0]
#define seperatorLowerColor [NSColor colorWithDeviceWhite:0.22 alpha:1.0]

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {}
    
    return self;
}

- (void)awakeFromNib{
 // Fake some cores
	CoreItem* bsnes = [[[CoreItem alloc] init] autorelease];
	bsnes.title = @"bsnes";
	bsnes.version = @"v078";
	
	CoreItem* crabemu = [[[CoreItem alloc] init] autorelease];
	crabemu.title = @"CrabEmu";
	crabemu.version = @"v0.1.9";
	
	CoreItem* mupen64plus = [[[CoreItem alloc] init] autorelease];
	mupen64plus.title = @"Mupen64Plus";
	mupen64plus.version = @"v1.99.4";
	
	CoreItem* neopop = [[[CoreItem alloc] init] autorelease];
	neopop.title = @"NeoPop";
	neopop.version = @"v0.71";
	
	CoreItem* visualboyadvance = [[[CoreItem alloc] init] autorelease];
	visualboyadvance.title = @"Visual Boy Advance";
	visualboyadvance.version = @"v1.7.2";
	
/*	NSImageView* imgView = [[NSImageView alloc] initWithFrame:NSMakeRect(0, 0, 0, 0)];
	[imgView setImage:[NSImage imageNamed:@"subviewtest"]];
	[imgView setImageFrameStyle:NSImageFrameNone];
*/
	
	visualboyadvance.detailView = sampleCoreSettings;
	visualboyadvance.detailViewSize = NSMakeSize(324, 264);
	
	CoreItem* gamebatte = [[[CoreItem alloc] init] autorelease];
	gamebatte.title = @"Gamebatte";
	gamebatte.version = @"v0.4.3";
	
	CoreItem* genesisPlus = [[[CoreItem alloc] init] autorelease];
	genesisPlus.title = @"GenesisPlus";
	genesisPlus.version = @"v1.1.1";
	
	CoreItem* nestopia = [[[CoreItem alloc] init] autorelease];
	nestopia.title = @"Nestopia";
	nestopia.version = @"v1.4.2";
	
	CoreItem* snes9x = [[[CoreItem alloc] init] autorelease];
	snes9x.title = @"SNES9x";
	snes9x.version = @"v1.52.1";
	
	CoreItem* mednafen = [[[CoreItem alloc] init] autorelease];
	mednafen.title = @"Mednafen";
	mednafen.version = @"v0.9.16-WIP";
	
	cores = [[NSArray arrayWithObjects:bsnes, crabemu, mupen64plus, neopop, visualboyadvance, gamebatte, genesisPlus, nestopia, snes9x, mednafen, nil] retain];
	[self _updateContent];
	[self scrollPoint:NSMakePoint(0, self.frame.size.height)];
}

- (void)dealloc{
	[cores release];
	cores = nil;
	
    [super dealloc];
}

+ (void)initialize{
	NSImage* coreTri = [NSImage imageNamed:@"core_triangle"];
	[coreTri setName:@"core_triangle_closed" forSubimageInRect:NSMakeRect(0, 0, 9, 10)];
	[coreTri setName:@"core_triangle_open" forSubimageInRect:NSMakeRect(9, 0, 9, 10)];
}

- (void)drawRect:(NSRect)dirtyRect{
	NSRect cellRect = NSMakeRect(0, self.frame.size.height, self.frame.size.width, 0);
	
	for(CoreItem* aCore in cores){
		float height = 0;
		height += collapsedHeight;
		
		if(!aCore.collapsed){
			height += 23;
			height += aCore.detailViewSize.height;
		}
		
		if(aCore != [cores lastObject]){
			height += seperatorHeight;
		}
		cellRect.size.height = height;
		cellRect.origin.y -= height;
		
		// Calculate rect for text
		NSRect textRect = cellRect;
		textRect.origin.y -= (collapsedHeight-cellRect.size.height);
		textRect.size.height = collapsedHeight;
		textRect = NSInsetRect(textRect, 43, 10);
		
		textRect.origin.y -= 10;

		// Draw title
		NSColor* textColor = [NSColor colorWithDeviceWhite:0.96 alpha:1.0];
		NSFont* font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:NSBoldFontMask weight:5.0 size:11.0];
		NSShadow* shadow = [[[NSShadow alloc] init] autorelease];
		[shadow setShadowColor:[NSColor colorWithDeviceWhite:0.0 alpha:0.4]];
		[shadow setShadowBlurRadius:1.0];
		[shadow setShadowOffset:NSMakeSize(0, -1)];
		
		NSMutableDictionary* textAttributes = [NSMutableDictionary dictionary];
		[textAttributes setObject:textColor forKey:NSForegroundColorAttributeName];
		[textAttributes setObject:font forKey:NSFontAttributeName];
		[textAttributes setObject:shadow forKey:NSShadowAttributeName];
		
		NSAttributedString* attributedTitle = [[NSAttributedString alloc] initWithString:aCore.title attributes:textAttributes];
		[attributedTitle drawInRect:textRect];
		
		// Draw version
		font = [[NSFontManager sharedFontManager] fontWithFamily:@"Helvetica" traits:NSBoldFontMask weight:4.0 size:11.0];
		textColor = [NSColor colorWithDeviceWhite:0.62 alpha:1.0];
		[textAttributes setObject:textColor forKey:NSForegroundColorAttributeName];
		[textAttributes setObject:font forKey:NSFontAttributeName];
		[textAttributes setObject:[NSNumber numberWithFloat:0.3] forKey:NSObliquenessAttributeName];
		
		textRect.origin.x += attributedTitle.size.width + 10;
		[aCore.version drawInRect:textRect withAttributes:textAttributes];	
		[attributedTitle release];	
		
		// Draw collapse / expand button
	//	if(aCore.detailView!=nil){
			NSRect btnRect = NSMakeRect(27, textRect.origin.y+25, 9, 10);
			NSImage* btnImage = aCore.collapsed ? [NSImage imageNamed:@"core_triangle_closed"] : [NSImage imageNamed:@"core_triangle_open"];
			[btnImage drawInRect:btnRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0];
		
			aCore.btnRect = btnRect;
	//	}
		
		// adjust frame of subview if necessary
		if(!aCore.collapsed){
			NSRect detailViewFrame = cellRect;
			detailViewFrame.size = aCore.detailViewSize;
			detailViewFrame.origin.x += 42;
			detailViewFrame.origin.y += 28;
			
			[aCore.detailView setFrame:detailViewFrame];
			[aCore.detailView setHidden:NO];		
		}
		
		// Draw seperator line if appropriate
		if(aCore != [cores lastObject]){
			NSRect lineRect = NSMakeRect(0, cellRect.origin.y+2, self.frame.size.width, 1);
			[seperatorUpperColor setFill];
			NSRectFill(lineRect);
			
			lineRect.origin.y -= 1;
			[seperatorLowerColor setFill];
			NSRectFill(lineRect);
		}
		
		
	}
}

- (void)_updateContent{	
	// calculate new height;
	float height = 0;
	
	for(CoreItem* core in cores){
		height += collapsedHeight;
		
		if(!core.collapsed){
			height += 24;
			height += core.detailViewSize.height;
			
			// check if we are already superview
			if([core.detailView superview]!=self){
				// check if view somehow has another superview
				if([core.detailView superview]){
					// remove from super view
					[core.detailView removeFromSuperview];
				}
				
				[self addSubview:core.detailView];
				[core.detailView setHidden:YES];				
			}
		} else if([core.detailView superview]==self){
			[core.detailView removeFromSuperview];
		}
		
		if(core != [cores lastObject]){
			height += seperatorHeight;
		}
		
	}
	
	height = height < [self visibleRect].size.height ? [self visibleRect].size.height : height;
	
	NSPoint clipOrigin = self.enclosingScrollView.contentView.bounds.origin;
	clipOrigin.y += (height-self.frame.size.height);
	clipOrigin.y = clipOrigin.y < 0 ? 0 : clipOrigin.y;
	clipOrigin.y = clipOrigin.y > height ? height : clipOrigin.y;
	
	BOOL needsSpecialDisplay = height==[self visibleRect].size.height;
	[self setFrameSize:NSMakeSize([self visibleRect].size.width, height)];
	
	[self.enclosingScrollView.contentView scrollToPoint:clipOrigin];
	[self.enclosingScrollView reflectScrolledClipView:self.enclosingScrollView.contentView];
	
	[self setNeedsDisplay:needsSpecialDisplay];
}


- (void)mouseDown:(NSEvent *)theEvent{
	coreToBeExpanded = nil;

	NSPoint loc = [self convertPointFromBase:[theEvent locationInWindow]];
	for(CoreItem* aCore in cores){
		if(NSPointInRect(loc, aCore.btnRect)){
			coreToBeExpanded = aCore;
			return;
		}
	}
}

- (void)mouseUp:(NSEvent *)theEvent{
	NSPoint loc = [self convertPointFromBase:[theEvent locationInWindow]];
	if(coreToBeExpanded && NSPointInRect(loc, coreToBeExpanded.btnRect)){
		coreToBeExpanded.collapsed = !coreToBeExpanded.collapsed;
		[self _updateContent];
	}

}
@end

@implementation CoreItem
@synthesize collapsed, title, version, detailViewSize, detailView, btnRect;

- (id)init {
    self = [super init];
    if (self) {
        self.collapsed = YES;
    }
    return self;
}

@end