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

NSString *const OEMaxSaveGameNameLengthKey = @"MaxSaveGameNameLength";

NSString *const OERemoveGameFromCollectionAlertSuppressionKey = @"removeGamesFromCollectionWithoutConfirmation";
NSString *const OELoadAutoSaveAlertSuppressionKey   = @"loadAutosaveDialogChoice";
NSString *const OESaveGameWhenQuitAlertSuppressionKey = @"autosaveOnStopDialogChoice";
NSString *const OEDeleteGameAlertSuppressionKey = @"removeStatesWithoutConfirmation";
NSString *const OESaveGameAlertSuppressionKey = @"saveGameWithoutConfirmation";
NSString *const OEChangeCoreAlertSuppressionKey = @"changeCoreWithoutConfirmation";
NSString *const OEResetSystemAlertSuppressionKey = @"resetSystemWithoutConfirmation";
NSString *const OEStopEmulationAlertSuppressionKey = @"stopEmulationWithoutConfirmation";
NSString *const OERemoveGameFilesFromLibraryAlertSuppressionKey = @"trashFilesDialogChoice";
NSString *const OEGameCoreGlitchesSuppressionKey = @"OEGameCoreGlitches";
NSString *const OERenameSpecialSaveStateAlertSuppressionKey = @"OERenameSpecialSaveStateAlertSuppressionKey";
NSString *const OEDeleteScreenshotAlertSuppressionKey = @"OEDeleteScreenshotAlertSuppressionKey";
NSString *const OEDeleteSaveStateAlertSuppressionKey  = @"OEDeleteSaveStateAlertSuppressionKey";

@implementation OEHUDAlert (DefaultAlertsAdditions)

+ (id)quitApplicationAlert
{
    OEHUDAlert *alert = [[OEHUDAlert alloc] init];

    alert.headlineText = OELocalizedString(@"Are you sure you want to quit the application?", @"");
    alert.messageText = OELocalizedString(@"OpenEmu will save and quit all games that are currently running.", @"");
    alert.defaultButtonTitle = OELocalizedString(@"Quit", @"");
    alert.alternateButtonTitle = OELocalizedString(@"Cancel", @"");

    return alert;
}

+ (id)saveAutoSaveGameAlert
{
    [[NSUserDefaults standardUserDefaults] registerDefaults:@{ OESaveGameWhenQuitAlertSuppressionKey: @(1)}];
    
    OEHUDAlert *alert = [[OEHUDAlert alloc] init];

    alert.headlineText = OELocalizedString(@"Would you like to save your game before you quit?", @"");
    alert.messageText = OELocalizedString(@"OpenEmu includes a save game feature that allows you to continue playing exactly where you left off.", @"");
    alert.defaultButtonTitle = OELocalizedString(@"Save Game", @"");
    alert.alternateButtonTitle = OELocalizedString(@"Do Not Save", @"");
    
    [alert setSuppressOnDefaultReturnOnly:NO];
    [alert showSuppressionButtonForUDKey:OESaveGameWhenQuitAlertSuppressionKey];
    
    return alert;
}

+ (id)loadAutoSaveGameAlert
{
    OEHUDAlert *alert = [[OEHUDAlert alloc] init];
    
    alert.headlineText = OELocalizedString(@"Would you like to continue your last game?", @"");
    alert.messageText = OELocalizedString(@"Do you want to continue playing where you left off?", @"");
    alert.defaultButtonTitle = OELocalizedString(@"Yes", @"");
    alert.alternateButtonTitle = OELocalizedString(@"No", @"");
    
    [alert setSuppressOnDefaultReturnOnly:NO];
    [alert showSuppressionButtonForUDKey:OELoadAutoSaveAlertSuppressionKey];
    
    return alert;
}


+ (id)saveGameAlertWithProposedName:(NSString*)name
{
    OEHUDAlert *alert = [[OEHUDAlert alloc] init];

    [[NSUserDefaults standardUserDefaults] registerDefaults:@{OEMaxSaveGameNameLengthKey:@40}];
    
    [alert setInputLabelText:OELocalizedString(@"Save As:", @"")];
    [alert setDefaultButtonTitle:OELocalizedString(@"Save Game", @"")];
    [alert setAlternateButtonTitle:OELocalizedString(@"Cancel", @"")];
    [alert setShowsInputField:YES];
    
    NSInteger maxiumumSaveGameLength = [[NSUserDefaults standardUserDefaults] integerForKey:OEMaxSaveGameNameLengthKey];
    if([name length]>maxiumumSaveGameLength)
    {
        name = [name substringToIndex:maxiumumSaveGameLength];
    }
    [alert setStringValue:name];
    [alert setInputLimit:40];
    
    [alert showSuppressionButtonForUDKey:OESaveGameAlertSuppressionKey];

    return alert;
}

+ (id)resetSystemAlert
{
    OEHUDAlert *alert = [[OEHUDAlert alloc] init];
    
    alert.headlineText = OELocalizedString(@"Are you sure you want to reset the console?", @"");
    alert.defaultButtonTitle = OELocalizedString(@"Restart", @"");
    alert.alternateButtonTitle = OELocalizedString(@"Cancel", @"");
    
    [alert setSuppressOnDefaultReturnOnly:YES];
    [alert showSuppressionButtonForUDKey:OEResetSystemAlertSuppressionKey];
    
    return alert;
}

+ (id)stopEmulationAlert
{
    OEHUDAlert *alert = [[OEHUDAlert alloc] init];

    alert.headlineText = OELocalizedString(@"Are you sure you want to stop emulation?", @"");
    alert.defaultButtonTitle = OELocalizedString(@"Stop", @"");
    alert.alternateButtonTitle = OELocalizedString(@"Cancel", @"");
    
    [alert setSuppressOnDefaultReturnOnly:YES];
    [alert showSuppressionButtonForUDKey:OEStopEmulationAlertSuppressionKey];
    
    return alert;
}


+ (id)deleteStateAlertWithStateName:(NSString*)stateName
{
    OEHUDAlert *alert = [[OEHUDAlert alloc] init];
    NSString *messageText = [NSString stringWithFormat:OELocalizedString(@"Are you sure you want to delete the save game called '%@' from your OpenEmu library?", @""), stateName];
    [alert setMessageText:messageText];
    [alert setDefaultButtonTitle:OELocalizedString(@"Delete Save", @"")];
    [alert setAlternateButtonTitle:OELocalizedString(@"Cancel", @"")];
    [alert setHeadlineText:nil];
    [alert showSuppressionButtonForUDKey:OEDeleteSaveStateAlertSuppressionKey];
    
    return alert;
}

+ (id)deleteStateAlertWithStateCount:(NSUInteger)count
{
    OEHUDAlert *alert = [[OEHUDAlert alloc] init];
    NSString *messageText = [NSString stringWithFormat:OELocalizedString(@"Are you sure you want to delete %ld save games from your OpenEmu library?", @""), count];
    [alert setMessageText:messageText];
    [alert setDefaultButtonTitle:OELocalizedString(@"Delete Saves", @"")];
    [alert setAlternateButtonTitle:OELocalizedString(@"Cancel", @"")];
    [alert setHeadlineText:nil];
    [alert showSuppressionButtonForUDKey:OEDeleteSaveStateAlertSuppressionKey];

    return alert;
}



+ (id)deleteScreenshotAlertWithScreenshotName:(NSString*)screenshotName
{
    OEHUDAlert *alert = [[OEHUDAlert alloc] init];
    NSString *messageText = [NSString stringWithFormat:OELocalizedString(@"Are you sure you want to delete the screenshot called '%@' from your OpenEmu library?", @""), screenshotName];
    [alert setMessageText:messageText];
    [alert setDefaultButtonTitle:OELocalizedString(@"Delete Screenshot", @"")];
    [alert setAlternateButtonTitle:OELocalizedString(@"Cancel", @"")];
    [alert setHeadlineText:nil];
    [alert showSuppressionButtonForUDKey:OEDeleteScreenshotAlertSuppressionKey];

    return alert;
}

+ (id)deleteScreenshotAlertWithScreenshotCount:(NSUInteger)count
{
    OEHUDAlert *alert = [[OEHUDAlert alloc] init];
    NSString *messageText = [NSString stringWithFormat:OELocalizedString(@"Are you sure you want to delete %ld screenshots from your OpenEmu library?", @""), count];
    [alert setMessageText:messageText];
    [alert setDefaultButtonTitle:OELocalizedString(@"Delete Screenshots", @"")];
    [alert setAlternateButtonTitle:OELocalizedString(@"Cancel", @"")];
    [alert setHeadlineText:nil];
    [alert showSuppressionButtonForUDKey:OEDeleteScreenshotAlertSuppressionKey];

    return alert;
}

+ (id)removeGamesFromCollectionAlert
{
    OEHUDAlert *alert = [[OEHUDAlert alloc] init];
    NSString *messageText = [NSString stringWithFormat:OELocalizedString(@"Are you sure you want to remove the selected games from the collection?", @"")];
    [alert setMessageText:messageText];
    [alert setDefaultButtonTitle:OELocalizedString(@"Remove", @"")];
    [alert setAlternateButtonTitle:OELocalizedString(@"Cancel", @"")];
    [alert setHeadlineText:nil];
    [alert showSuppressionButtonForUDKey:OERemoveGameFromCollectionAlertSuppressionKey];
    
    return alert;
}


+ (id)removeGamesFromLibraryAlert:(BOOL)multipleGames
{
    OEHUDAlert *alert = [[OEHUDAlert alloc] init];
    NSString *messageText = multipleGames ? [NSString stringWithFormat:OELocalizedString(@"Are you sure you want to delete the selected games from your OpenEmu library?", @"")]
                                          : [NSString stringWithFormat:OELocalizedString(@"Are you sure you want to delete the selected game from your OpenEmu library?", @"")];
    [alert setMessageText:messageText];
    [alert setDefaultButtonTitle:multipleGames?OELocalizedString(@"Delete Games", @""):OELocalizedString(@"Delete Game", @"")];
    [alert setAlternateButtonTitle:OELocalizedString(@"Cancel", @"")];
    [alert setHeadlineText:nil];
    [alert showSuppressionButtonForUDKey:OERemoveGameFromCollectionAlertSuppressionKey];
    
    return alert;
}

+ (id)removeGameFilesFromLibraryAlert:(BOOL)multipleGames
{
    OEHUDAlert *alert = [[OEHUDAlert alloc] init];
    NSString *headlineText = multipleGames ? [NSString stringWithFormat:OELocalizedString(@"Move selected games to Trash, or keep them in the Library folder?", @"")] : [NSString stringWithFormat:OELocalizedString(@"Move selected game to Trash, or keep it in the Library folder?", @"")];
    [alert setHeadlineText:headlineText];
    NSString *messageText = OELocalizedString(@"Only files in the OpenEmu Library folder will be moved to the Trash.", @"");
    [alert setMessageText:messageText];
    [alert setDefaultButtonTitle:OELocalizedString(@"Move to Trash", @"")];
    [alert setAlternateButtonTitle:OELocalizedString(@"Keep Files", @"")];
    [alert showSuppressionButtonForUDKey:OERemoveGameFilesFromLibraryAlertSuppressionKey];
    [alert setSuppressOnDefaultReturnOnly:NO];
    
    return alert;
}


+ (id)renameSpecialStateAlert
{
    OEHUDAlert *alert = [[OEHUDAlert alloc] init];
    NSString *headlineText = [NSString stringWithFormat:OELocalizedString(@"Rename Special Save State or something?", @"")];
    [alert setHeadlineText:headlineText];
    NSString *messageText = OELocalizedString(@"Won't be able to recognize it as special save state…", @"");
    [alert setMessageText:messageText];
    [alert setDefaultButtonTitle:OELocalizedString(@"Rename", @"")];
    [alert setAlternateButtonTitle:OELocalizedString(@"Cancel", @"")];
    [alert showSuppressionButtonForUDKey:OERenameSpecialSaveStateAlertSuppressionKey];
    [alert setSuppressOnDefaultReturnOnly:YES];

    return alert;
}

@end
