//
//  ListViewDataSourceItem.h
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 10.04.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>


@protocol ListViewDataSourceItem <NSObject>

- (NSImage*)listViewStatus:(BOOL)selected;
- (NSNumber*)listViewRating;
- (NSString*)listViewTitle;
- (NSString*)listViewLastPlayed;
- (NSString*)listViewConsoleName;

- (void)setListViewRating:(NSNumber*)number;

@end
