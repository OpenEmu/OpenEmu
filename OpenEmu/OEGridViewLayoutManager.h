//
//  OEGridViewLayoutManager.h
//  OEGridViewDemo
//
//  Created by Faustino Osuna on 2/9/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>

@protocol OEGridViewLayoutManagerProtocol <NSObject>

- (void)layoutSublayers;

@end


@interface OEGridViewLayoutManager : NSObject

+ (OEGridViewLayoutManager *)layoutManager;

@end
