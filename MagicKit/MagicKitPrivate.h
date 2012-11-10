/*
 *  MagicKitPrivate.h
 *  MagicKit
 *
 *  Created by Aidan Steele on 22/10/10.
 *  Copyright 2010 Glass Echidna. All rights reserved.
 *
 */
#import <Foundation/Foundation.h>
#import <MagicKit/GEMagicKit.h>

@interface GEMagicKit ()

@end

@interface GEMagicResult ()

- (id)initWithMimeType:(NSString *)aMimeType description:(NSString *)aDescription typeHierarchy:(NSArray *)typeHierarchy;
@property (nonatomic, readwrite, copy) NSString *mimeType;
@property (nonatomic, readwrite, copy) NSString *description;
@property (nonatomic, readwrite, copy) NSString *uniformType;
@property (nonatomic, readwrite, copy) NSArray *uniformTypeHierarchy;

@end