/*
 Copyright (c) 2017, OpenEmu Team
 
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

NS_ASSUME_NONNULL_BEGIN

NSString *const OEMaxSaveGameNameLengthKey = @"MaxSaveGameNameLength";

NSString *const OERemoveGameFromCollectionAlertSuppressionKey = @"removeGamesFromCollectionWithoutConfirmation";
NSString *const OELoadAutoSaveAlertSuppressionKey = @"loadAutosaveDialogChoice";
NSString *const OEDeleteGameAlertSuppressionKey = @"removeStatesWithoutConfirmation";
NSString *const OESaveGameAlertSuppressionKey = @"saveGameWithoutConfirmation";
NSString *const OEChangeCoreAlertSuppressionKey = @"changeCoreWithoutConfirmation";
NSString *const OEResetSystemAlertSuppressionKey = @"resetSystemWithoutConfirmation";
NSString *const OEStopEmulationAlertSuppressionKey = @"stopEmulationWithoutConfirmation";
NSString *const OERemoveGameFilesFromLibraryAlertSuppressionKey = @"trashFilesDialogChoice";
NSString *const OEGameCoreGlitchesSuppressionKey = @"OEGameCoreGlitches";
NSString *const OERenameSpecialSaveStateAlertSuppressionKey = @"OERenameSpecialSaveStateAlertSuppressionKey";
NSString *const OEDeleteScreenshotAlertSuppressionKey = @"OEDeleteScreenshotAlertSuppressionKey";
NSString *const OEDeleteSaveStateAlertSuppressionKey = @"OEDeleteSaveStateAlertSuppressionKey";
NSString *const OEDownloadRomWarningSupperssionKey = @"OEDownloadRomWarningSupperssionKey";

@implementation OEHUDAlert (DefaultAlertsAdditions)

+ (OEHUDAlert *)quitApplicationAlert
{
    OEHUDAlert *alert = [[OEHUDAlert alloc] init];

    alert.headlineText = NSLocalizedString(@"Are you sure you want to quit the application?", @"");
    alert.messageText = NSLocalizedString(@"OpenEmu will save and quit all games that are currently running.", @"");
    alert.defaultButtonTitle = NSLocalizedString(@"Quit", @"");
    alert.alternateButtonTitle = NSLocalizedString(@"Cancel", @"");

    return alert;
}

+ (OEHUDAlert *)loadAutoSaveGameAlert
{
    OEHUDAlert *alert = [[OEHUDAlert alloc] init];
    
    alert.headlineText = NSLocalizedString(@"Would you like to continue your last game?", @"");
    alert.messageText = NSLocalizedString(@"Do you want to continue playing where you left off?", @"");
    alert.defaultButtonTitle = NSLocalizedString(@"Yes", @"");
    alert.alternateButtonTitle = NSLocalizedString(@"No", @"");
    
    alert.suppressOnDefaultReturnOnly = NO;
    [alert showSuppressionButtonForUDKey:OELoadAutoSaveAlertSuppressionKey];
    
    return alert;
}


+ (OEHUDAlert *)saveGameAlertWithProposedName:(NSString*)name
{
    OEHUDAlert *alert = [[OEHUDAlert alloc] init];

    [NSUserDefaults.standardUserDefaults registerDefaults:@{OEMaxSaveGameNameLengthKey:@40}];
    
    alert.inputLabelText = NSLocalizedString(@"Save As:", @"");
    alert.defaultButtonTitle = NSLocalizedString(@"Save Game", @"");
    alert.alternateButtonTitle = NSLocalizedString(@"Cancel", @"");
    alert.showsInputField = YES;
    
    NSInteger maxiumumSaveGameLength = [NSUserDefaults.standardUserDefaults integerForKey:OEMaxSaveGameNameLengthKey];
    if(name.length > maxiumumSaveGameLength)
    {
        name = [name substringToIndex:maxiumumSaveGameLength];
    }
    alert.stringValue = name;
    alert.inputLimit = 40;
    
    [alert showSuppressionButtonForUDKey:OESaveGameAlertSuppressionKey];

    return alert;
}

+ (OEHUDAlert *)resetSystemAlert
{
    OEHUDAlert *alert = [[OEHUDAlert alloc] init];
    
    alert.headlineText = NSLocalizedString(@"Are you sure you want to reset the console?", @"");
    alert.defaultButtonTitle = NSLocalizedString(@"Restart", @"");
    alert.alternateButtonTitle = NSLocalizedString(@"Cancel", @"");
    
    [alert setSuppressOnDefaultReturnOnly:YES];
    [alert showSuppressionButtonForUDKey:OEResetSystemAlertSuppressionKey];
    
    return alert;
}

+ (OEHUDAlert *)stopEmulationAlert
{
    OEHUDAlert *alert = [[OEHUDAlert alloc] init];

    alert.headlineText = NSLocalizedString(@"Are you sure you want to stop emulation?", @"");
    alert.defaultButtonTitle = NSLocalizedString(@"Stop", @"");
    alert.alternateButtonTitle = NSLocalizedString(@"Cancel", @"");
    
    alert.suppressOnDefaultReturnOnly = YES;
    [alert showSuppressionButtonForUDKey:OEStopEmulationAlertSuppressionKey];
        
    return alert;
}


+ (OEHUDAlert *)deleteStateAlertWithStateName:(NSString*)stateName
{
    OEHUDAlert *alert = [[OEHUDAlert alloc] init];
    NSString *messageText = [NSString stringWithFormat:NSLocalizedString(@"Are you sure you want to delete the save game called '%@' from your OpenEmu library?", @""), stateName];
    alert.messageText = messageText;
    alert.defaultButtonTitle = NSLocalizedString(@"Delete Save", @"");
    alert.alternateButtonTitle = NSLocalizedString(@"Cancel", @"");
    alert.headlineText = nil;
    [alert showSuppressionButtonForUDKey:OEDeleteSaveStateAlertSuppressionKey];
    
    return alert;
}

+ (OEHUDAlert *)deleteStateAlertWithStateCount:(NSUInteger)count
{
    OEHUDAlert *alert = [[OEHUDAlert alloc] init];
    NSString *messageText = [NSString stringWithFormat:NSLocalizedString(@"Are you sure you want to delete %ld save games from your OpenEmu library?", @""), count];
    alert.messageText = messageText;
    alert.defaultButtonTitle = NSLocalizedString(@"Delete Saves", @"");
    alert.alternateButtonTitle = NSLocalizedString(@"Cancel", @"");
    alert.headlineText = nil;
    [alert showSuppressionButtonForUDKey:OEDeleteSaveStateAlertSuppressionKey];

    return alert;
}



+ (OEHUDAlert *)deleteScreenshotAlertWithScreenshotName:(NSString*)screenshotName
{
    OEHUDAlert *alert = [[OEHUDAlert alloc] init];
    NSString *messageText = [NSString stringWithFormat:NSLocalizedString(@"Are you sure you want to delete the screenshot called '%@' from your OpenEmu library?", @""), screenshotName];
    alert.messageText = messageText;
    alert.defaultButtonTitle = NSLocalizedString(@"Delete Screenshot", @"");
    alert.alternateButtonTitle = NSLocalizedString(@"Cancel", @"");
    alert.headlineText = nil;
    [alert showSuppressionButtonForUDKey:OEDeleteScreenshotAlertSuppressionKey];

    return alert;
}

+ (OEHUDAlert *)deleteScreenshotAlertWithScreenshotCount:(NSUInteger)count
{
    OEHUDAlert *alert = [[OEHUDAlert alloc] init];
    NSString *messageText = [NSString stringWithFormat:NSLocalizedString(@"Are you sure you want to delete %ld screenshots from your OpenEmu library?", @""), count];
    alert.messageText = messageText;
    alert.defaultButtonTitle = NSLocalizedString(@"Delete Screenshots", @"");
    alert.alternateButtonTitle = NSLocalizedString(@"Cancel", @"");
    alert.headlineText = nil;
    [alert showSuppressionButtonForUDKey:OEDeleteScreenshotAlertSuppressionKey];

    return alert;
}

+ (OEHUDAlert *)removeGamesFromCollectionAlert
{
    OEHUDAlert *alert = [[OEHUDAlert alloc] init];
    NSString *messageText = [NSString stringWithFormat:NSLocalizedString(@"Are you sure you want to remove the selected games from the collection?", @"")];
    alert.messageText = messageText;
    alert.defaultButtonTitle = NSLocalizedString(@"Remove", @"");
    alert.alternateButtonTitle = NSLocalizedString(@"Cancel", @"");
    alert.headlineText = nil;
    [alert showSuppressionButtonForUDKey:OERemoveGameFromCollectionAlertSuppressionKey];
    
    return alert;
}


+ (OEHUDAlert *)removeGamesFromLibraryAlert:(BOOL)multipleGames
{
    OEHUDAlert *alert = [[OEHUDAlert alloc] init];
    NSString *messageText = multipleGames ? [NSString stringWithFormat:NSLocalizedString(@"Are you sure you want to delete the selected games from your OpenEmu library?", @"")]
                                          : [NSString stringWithFormat:NSLocalizedString(@"Are you sure you want to delete the selected game from your OpenEmu library?", @"")];
    alert.messageText = messageText;
    alert.defaultButtonTitle = multipleGames?NSLocalizedString(@"Delete Games", @""):NSLocalizedString(@"Delete Game", @"");
    alert.alternateButtonTitle = NSLocalizedString(@"Cancel", @"");
    alert.headlineText = nil;
    [alert showSuppressionButtonForUDKey:OERemoveGameFromCollectionAlertSuppressionKey];
    
    return alert;
}

+ (OEHUDAlert *)removeGameFilesFromLibraryAlert:(BOOL)multipleGames
{
    OEHUDAlert *alert = [[OEHUDAlert alloc] init];
    NSString *headlineText = multipleGames ? [NSString stringWithFormat:NSLocalizedString(@"Move selected games to Trash, or keep them in the Library folder?", @"")] : [NSString stringWithFormat:NSLocalizedString(@"Move selected game to Trash, or keep it in the Library folder?", @"")];
    alert.headlineText = headlineText;
    NSString *messageText = NSLocalizedString(@"Only files in the OpenEmu Library folder will be moved to the Trash.", @"");
    alert.messageText = messageText;
    alert.defaultButtonTitle = NSLocalizedString(@"Move to Trash", @"");
    alert.alternateButtonTitle = NSLocalizedString(@"Keep Files", @"");
    [alert showSuppressionButtonForUDKey:OERemoveGameFilesFromLibraryAlertSuppressionKey];
    alert.suppressOnDefaultReturnOnly = NO;
    
    return alert;
}


+ (OEHUDAlert *)renameSpecialStateAlert
{
    OEHUDAlert *alert = [[OEHUDAlert alloc] init];
    NSString *headlineText = [NSString stringWithFormat:NSLocalizedString(@"Rename Special Save State or something?", @"")];
    alert.headlineText = headlineText;
    NSString *messageText = NSLocalizedString(@"Won't be able to recognize it as special save state…", @"");
    alert.messageText = messageText;
    alert.defaultButtonTitle = NSLocalizedString(@"Rename", @"");
    alert.alternateButtonTitle = NSLocalizedString(@"Cancel", @"");
    [alert showSuppressionButtonForUDKey:OERenameSpecialSaveStateAlertSuppressionKey];
    alert.suppressOnDefaultReturnOnly = YES;

    return alert;
}

+ (OEHUDAlert *)romDownloadRequiredAlert:(NSString*)name server:(NSString*)server
{
    OEHUDAlert *alert = [[OEHUDAlert alloc] init];
    NSString *headlineText = [NSString stringWithFormat:NSLocalizedString(@"Game requires download", @"Download rom dialog headline")];
    alert.headlineText = headlineText;
    NSString *messageText = [NSString stringWithFormat:NSLocalizedString(@"In order to play the game it must be downloaded.", @"")];
    alert.messageText = messageText;
    alert.defaultButtonTitle = NSLocalizedString(@"Download", @"");
    alert.alternateButtonTitle = NSLocalizedString(@"Cancel", @"");
    [alert showSuppressionButtonForUDKey:OEDownloadRomWarningSupperssionKey];
    alert.suppressOnDefaultReturnOnly = YES;

    return alert;
}

+ (OEHUDAlert *)missingBIOSFilesAlert:(NSString*)missingFilesList
{
    NSMutableString *missingFilesMessage = [[NSMutableString alloc] init];
    [missingFilesMessage appendString:[NSString stringWithFormat:NSLocalizedString(@"To run this core you need the following:\n\n%@Drag and drop the required file(s) onto the game library window and try again.", @"Missing files dialog text"), missingFilesList]];

    OEHUDAlert *alert = [OEHUDAlert alertWithMessageText:NSLocalizedString(missingFilesMessage, @"")
                                           defaultButton:NSLocalizedString(@"OK", @"")
                                         alternateButton:NSLocalizedString(@"Learn More", @"")];
    alert.headlineText = NSLocalizedString(@"Required files are missing.", @"");

    return alert;
}

@end

NS_ASSUME_NONNULL_END
