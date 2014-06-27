//
//  Screenshot.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 27/06/14.
//
//

#import <Foundation/Foundation.h>
#import <CoreData/CoreData.h>
#import "OEDBItem.h"

@class OEDBRom;

extern NSString * const OEDBScreenshotImportRequired;

@interface OEDBScreenshot : OEDBItem

@property (nonatomic, retain) NSString * location;
@property (nonatomic, retain) NSString * name;
@property (nonatomic, retain) NSDate * timestamp;
@property (nonatomic, retain) NSString * userDescription;
@property (nonatomic, retain) OEDBRom *rom;

@property (nonatomic, assign) NSURL *URL;
@end
