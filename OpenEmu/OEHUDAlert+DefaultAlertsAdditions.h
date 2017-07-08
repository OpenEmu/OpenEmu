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


#import "OEHUDAlert.h"

NS_ASSUME_NONNULL_BEGIN

extern NSString *const OEMaxSaveGameNameLengthKey;

extern NSString *const OERemoveGameFromCollectionAlertSuppressionKey;
extern NSString *const OELoadAutoSaveAlertSuppressionKey;
extern NSString *const OEDeleteGameAlertSuppressionKey;
extern NSString *const OESaveGameAlertSuppressionKey;
extern NSString *const OEChangeCoreAlertSuppressionKey;
extern NSString *const OEResetSystemAlertSuppressionKey;
extern NSString *const OEStopEmulationAlertSuppressionKey;
extern NSString *const OEDeleteSaveStateAlertSuppressionKey;
extern NSString *const OEDeleteScreenshotAlertSuppressionKey;
extern NSString *const OERemoveGameFilesFromLibraryAlertSuppressionKey;
extern NSString *const OERenameSpecialSaveStateAlertSuppressionKey;
extern NSString *const OEGameCoreGlitchesSuppressionKey;
extern NSString *const OEDownloadRomWarningSupperssionKey;

@interface OEHUDAlert (DefaultAlertsAdditions)

+ (OEHUDAlert *)quitApplicationAlert;
+ (OEHUDAlert *)saveGameAlertWithProposedName:(NSString *)name;
+ (OEHUDAlert *)loadAutoSaveGameAlert;
+ (OEHUDAlert *)deleteStateAlertWithStateName:(NSString *)stateName;
+ (OEHUDAlert *)deleteStateAlertWithStateCount:(NSUInteger)count;
+ (OEHUDAlert *)resetSystemAlert;
+ (OEHUDAlert *)stopEmulationAlert;

+ (OEHUDAlert *)deleteScreenshotAlertWithScreenshotName:(NSString*)screenshotName;
+ (OEHUDAlert *)deleteScreenshotAlertWithScreenshotCount:(NSUInteger)count;

+ (OEHUDAlert *)removeGamesFromCollectionAlert;
+ (OEHUDAlert *)removeGamesFromLibraryAlert:(BOOL)multipleGames;
+ (OEHUDAlert *)removeGameFilesFromLibraryAlert:(BOOL)multipleGames;
+ (OEHUDAlert *)renameSpecialStateAlert;

+ (OEHUDAlert *)romDownloadRequiredAlert:(NSString*)name server:(NSString*)server;

+ (OEHUDAlert *)missingBIOSFilesAlert:(NSString*)missingFilesList;

@end

NS_ASSUME_NONNULL_END
