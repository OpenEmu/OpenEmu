//
//  CoverFlowDataSourceItem.h
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 05.05.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Foundation/Foundation.h>


@protocol CoverFlowDataSourceItem <NSObject>

- (NSString *) imageUID;
- (NSString *) imageRepresentationType;
- (id) imageRepresentation;
- (NSString *) imageTitle;
- (NSString *) imageSubtitle;

@end
