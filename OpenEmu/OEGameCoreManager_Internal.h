//
//  OEGameCoreManager_Internal.h
//  OpenEmu
//
//  Created by Remy Demarest on 25/08/2013.
//
//

#import "OEGameCoreManager.h"

@protocol OEGameCoreHelper;

@interface OEGameCoreManager ()
@property(nonatomic, strong) id<OEGameCoreHelper> gameCoreHelper;
@end
