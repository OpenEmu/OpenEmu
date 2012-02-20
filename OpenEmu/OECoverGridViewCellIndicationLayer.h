//
//  OECoverGridViewCellIndicationLayer.h
//  OpenEmu
//
//  Created by Faustino Osuna on 2/19/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "OEGridLayer.h"

typedef enum
{
    OECoverGridViewCellIndicationTypeNone           = 0,
    OECoverGridViewCellIndicationTypeFileMissing    = 2,
    OECoverGridViewCellIndicationTypeProcessing     = 1,
    OECoverGridViewCellIndicationTypeDropOn         = 3,
} OECoverGridViewCellIndicationType;

@interface OECoverGridViewCellIndicationLayer : OEGridLayer
{
@private
    OECoverGridViewCellIndicationType _type;
}

#pragma mark - Properties
@property (nonatomic, readwrite) OECoverGridViewCellIndicationType type;

@end
