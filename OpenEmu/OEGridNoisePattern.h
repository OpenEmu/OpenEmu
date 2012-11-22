//
//  OEGridNoisePattern.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 22.11.12.
//
//
#import <Cocoa/Cocoa.h>

extern void OEBackgroundNoisePatternCreate(void);
extern void OEBackgroundNoisePatternDrawInContext(void *info, CGContextRef ctx);
extern void OEBackgroundNoisePatternRelease(void *info);

extern CGColorRef OEBackgroundNoiseColorRef;
