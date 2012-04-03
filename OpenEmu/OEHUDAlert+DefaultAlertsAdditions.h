//
//  OEHUDAlert+DefaultAlertsAdditions.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 18.03.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "OEHUDAlert.h"

@interface OEHUDAlert (DefaultAlertsAdditions)

+ (id)saveGameAlertWithProposedName:(NSString*)name;
+ (id)saveAutoSaveGameAlert;
+ (id)loadAutoSaveGameAlert;
+ (id)deleteStateAlertWithStateName:(NSString*)stateName;

+ (id)removeGamesFromCollectionAlert;
@end
