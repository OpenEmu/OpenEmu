/*
 Copyright (c) 2012, OpenEmu Team
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 * Neither the name of the OpenEmu Team nor the
 names of its contributors may be used to endorse or promote products
 derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


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
