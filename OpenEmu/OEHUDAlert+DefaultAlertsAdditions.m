//
//  OEHUDAlert+DefaultAlertsAdditions.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 18.03.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "OEHUDAlert+DefaultAlertsAdditions.h"

@implementation OEHUDAlert (DefaultAlertsAdditions)

+ (id)saveAutoSaveGameAlert
{
    OEHUDAlert *alert = [[OEHUDAlert alloc] init];
    
    alert.messageText = NSLocalizedString(@"OpenEmu includes a save game feature that allows you to continue playing exactly where you left off.", @"");
    alert.defaultButtonTitle = NSLocalizedString(@"Save Game", @"");
    alert.alternateButtonTitle = NSLocalizedString(@"Do Not Save", @"");
    alert.headlineLabelText = NSLocalizedString(@"Would you like to save your game before you quit?", @"");
    
    [alert setSuppressOnDefaultReturnOnly:NO];
    [alert showSuppressionButtonForUDKey:UDSaveGameWhenQuitAlertSuppressionKey];
    
    return alert;
}

+ (id)loadAutoSaveGameAlert
{
    OEHUDAlert *alert = [[OEHUDAlert alloc] init];
    
    alert.messageText = NSLocalizedString(@"Do you want to continue playing where you left off?", @"");
    alert.defaultButtonTitle = NSLocalizedString(@"Yes", @"");
    alert.alternateButtonTitle = NSLocalizedString(@"No", @"");
    alert.headlineLabelText = NSLocalizedString(@"Would you like to continue your last game?", @"");
    
    [alert setSuppressOnDefaultReturnOnly:NO];
    [alert showSuppressionButtonForUDKey:UDLoadAutoSaveAlertSuppressionKey];
    
    return alert;
}


+ (id)saveGameAlertWithProposedName:(NSString*)name
{
    OEHUDAlert *alert = [[OEHUDAlert alloc] init];
    
    [alert setInputLabelText:@"Save As:"];
    [alert setDefaultButtonTitle:NSLocalizedString(@"Save Game", @"")];
    [alert setAlternateButtonTitle:NSLocalizedString(@"Cancel", @"")];
    [alert setShowsInputField:YES];
    
    NSInteger maxiumumSaveGameLength = [[NSUserDefaults standardUserDefaults] integerForKey:UDMaxSaveGameNameLengthKey];
    if([name length]>maxiumumSaveGameLength)
    {
        name = [name substringToIndex:maxiumumSaveGameLength];
    }
    [alert setStringValue:name];
    [alert setHeight:112.0];    
    [alert setWidth:372.0];
    [alert setInputLimit:40];
    
    [alert showSuppressionButtonForUDKey:UDSaveGameAlertSuppressionKey];
    [alert setSuppressionLabelText:NSLocalizedString(@"Don't ask again", @"")];
    
    return alert;
}

+ (id)deleteStateAlertWithStateName:(NSString*)stateName
{
    OEHUDAlert *alert = [[OEHUDAlert alloc] init];
    NSString *messageText = [NSString stringWithFormat:NSLocalizedString(@"Are you sure you want to delete the save game called '%@' from your OpenEmu library?", @""), stateName];
    [alert setMessageText:messageText];
    [alert setDefaultButtonTitle:NSLocalizedString(@"Delete Game", @"")];
    [alert setAlternateButtonTitle:NSLocalizedString(@"Cancel", @"")];
    [alert setHeadlineLabelText:nil];
    [alert showSuppressionButtonForUDKey:UDDelteGameAlertSuppressionKey];
    
    return alert;
}

+ (id)removeGamesFromCollectionAlert
{
    OEHUDAlert *alert = [[OEHUDAlert alloc] init];
    NSString *messageText = [NSString stringWithFormat:NSLocalizedString(@"Are you sure you want to remove the selected games from the collection?", @"")];
    [alert setMessageText:messageText];
    [alert setDefaultButtonTitle:NSLocalizedString(@"Remove", @"")];
    [alert setAlternateButtonTitle:NSLocalizedString(@"Cancel", @"")];
    [alert setHeadlineLabelText:nil];
    [alert showSuppressionButtonForUDKey:UDRemoveGameFromCollectionAlertSuppressionKey];
    
    return alert;
}


+ (id)removeGamesFromLibraryAlert:(BOOL)multipleGames
{
    OEHUDAlert *alert = [[OEHUDAlert alloc] init];
    NSString *messageText = multipleGames ? [NSString stringWithFormat:NSLocalizedString(@"Are you sure you want to delete the selected games from your OpenEmu library?", @"")]
                                          : [NSString stringWithFormat:NSLocalizedString(@"Are you sure you want to delete the selected game from your OpenEmu library?", @"")];
    [alert setMessageText:messageText];
    [alert setDefaultButtonTitle:multipleGames?NSLocalizedString(@"Delete Games", @""):NSLocalizedString(@"Delete Game", @"")];
    [alert setAlternateButtonTitle:NSLocalizedString(@"Cancel", @"")];
    [alert setHeadlineLabelText:nil];
    [alert showSuppressionButtonForUDKey:UDRemoveGameFromCollectionAlertSuppressionKey];
    
    return alert;
}

+ (id)removeGameFilesFromLibraryAlert:(BOOL)multipleGames
{
    OEHUDAlert *alert = [[OEHUDAlert alloc] init];
    NSString *headlineText = multipleGames ? [NSString stringWithFormat:NSLocalizedString(@"Do you want to move the selected games to the Trash, or keep them in the OpenEmu Library folder?", @"")] : [NSString stringWithFormat:NSLocalizedString(@"Do you want to move the selected game to the Trash, or keep it in the OpenEmu Library folder?", @"")];
    [alert setHeadlineLabelText:headlineText];
    NSString *messageText = NSLocalizedString(@"Only files in the OpenEmu Library folder will be moved to the Trash.", @"");
    [alert setMessageText:messageText];
    [alert setDefaultButtonTitle:NSLocalizedString(@"Move to Trash", @"")];
    [alert setAlternateButtonTitle:NSLocalizedString(@"Cancel", @"")];
    
    return alert;
}

@end
