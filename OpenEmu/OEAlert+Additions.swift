// Copyright (c) 2020, OpenEmu Team
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the OpenEmu Team nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import Foundation

extension OEAlert {
    
    @objc static let OERemoveGameFromCollectionAlertSuppressionKey = "removeGamesFromCollectionWithoutConfirmation"
    @objc static let OERemoveGameFromLibraryAlertSuppressionKey = "removeGamesFromLibraryWithoutConfirmation"
    @objc static let OERemoveCollectionAlertSuppressionKey = "removeCollectionWithoutConfirmation";
    @objc static let OELoadAutoSaveAlertSuppressionKey = "loadAutosaveDialogChoice"
    @objc static let OEDeleteGameAlertSuppressionKey = "removeStatesWithoutConfirmation"
    @objc static let OESaveGameAlertSuppressionKey = "saveGameWithoutConfirmation"
    @objc static let OEChangeCoreAlertSuppressionKey = "changeCoreWithoutConfirmation"
    @objc static let OEResetSystemAlertSuppressionKey = "resetSystemWithoutConfirmation"
    @objc static let OEStopEmulationAlertSuppressionKey = "stopEmulationWithoutConfirmation"
    @objc static let OERemoveGameFilesFromLibraryAlertSuppressionKey = "trashFilesDialogChoice"
    @objc static let OEAutoSwitchCoreAlertSuppressionKey = "changeCoreWhenLoadingStateWitoutConfirmation";
    @objc static let OEGameCoreGlitchesSuppressionKey = "OEGameCoreGlitches"
    @objc static let OERenameSpecialSaveStateAlertSuppressionKey = "OERenameSpecialSaveStateAlertSuppressionKey"
    @objc static let OEDeleteScreenshotAlertSuppressionKey = "OEDeleteScreenshotAlertSuppressionKey"
    @objc static let OEDeleteSaveStateAlertSuppressionKey = "OEDeleteSaveStateAlertSuppressionKey"
    @objc static let OEDownloadRomWarningSuppressionKey = "OEDownloadRomWarningSuppressionKey"
    
    
    final class func quitApplication() -> OEAlert {
        
        let alert = OEAlert()
        alert.messageText = NSLocalizedString("Are you sure you want to quit the application?", comment: "")
        alert.informativeText = NSLocalizedString("OpenEmu will save and quit all games that are currently running.", comment: "")
        alert.defaultButtonTitle = NSLocalizedString("Quit", comment: "")
        alert.alternateButtonTitle = NSLocalizedString("Cancel", comment: "")
        
        return alert
    }
    
    @objc(saveGameAlertWithProposedName:)
    final class func saveGame(proposedName: String) -> OEAlert {
        
        let alert = OEAlert()
        
        alert.inputLabelText = NSLocalizedString("Save As:", comment: "")
        alert.defaultButtonTitle = NSLocalizedString("Save Game", comment: "")
        alert.alternateButtonTitle = NSLocalizedString("Cancel", comment: "")
        alert.showSuppressionButton(forUDKey: OESaveGameAlertSuppressionKey)
        
        alert.showsInputField = true
        alert.stringValue = proposedName
        alert.inputPlaceholderText = proposedName
        alert.inputLimit = 40
        
        return alert
    }
    
    @objc(loadAutoSaveGameAlert)
    final class func loadAutoSaveGame() -> OEAlert {
        
        let alert = OEAlert()
        alert.messageText = NSLocalizedString("Would you like to continue your last game?", comment: "")
        alert.informativeText = NSLocalizedString("Do you want to continue playing where you left off?", comment: "")
        alert.defaultButtonTitle = NSLocalizedString("Resume", comment: "")
        alert.alternateButtonTitle = NSLocalizedString("Restart", comment: "")
        alert.showSuppressionButton(forUDKey: OELoadAutoSaveAlertSuppressionKey)
        alert.suppressOnDefaultReturnOnly = false
        
        return alert
    }
    
    @objc(deleteStateAlertWithStateName:)
    final class func deleteSaveState(name: String) -> OEAlert {
        
        let alert = OEAlert()
        alert.messageText = .localizedStringWithFormat(NSLocalizedString("Are you sure you want to delete the save game called '%@' from your OpenEmu library?", comment: ""), name)
        alert.defaultButtonTitle = NSLocalizedString("Delete Save", comment: "")
        alert.alternateButtonTitle = NSLocalizedString("Cancel", comment: "")
        alert.showSuppressionButton(forUDKey: OEDeleteSaveStateAlertSuppressionKey)
        
        return alert
    }
    
    @objc(deleteStateAlertWithStateCount:)
    final class func deleteSaveStates(count: Int) -> OEAlert {
        
        let alert = OEAlert()
        alert.messageText = .localizedStringWithFormat(NSLocalizedString("Are you sure you want to delete %ld save games from your OpenEmu library?", comment: ""), count)
        alert.defaultButtonTitle = NSLocalizedString("Delete Saves", comment: "")
        alert.alternateButtonTitle = NSLocalizedString("Cancel", comment: "")
        alert.showSuppressionButton(forUDKey: OEDeleteSaveStateAlertSuppressionKey)
        
        return alert
    }
    
    @objc(deleteScreenshotAlertWithScreenshotName:)
    final class func deleteScreenshot(name: String) -> OEAlert {
        
        let alert = OEAlert()
        alert.messageText = .localizedStringWithFormat(NSLocalizedString("Are you sure you want to delete the screenshot called '%@' from your OpenEmu library?", comment: ""), name)
        alert.defaultButtonTitle = NSLocalizedString("Delete Screenshot", comment: "")
        alert.alternateButtonTitle = NSLocalizedString("Cancel", comment: "")
        alert.showSuppressionButton(forUDKey: OEDeleteScreenshotAlertSuppressionKey)
        
        return alert
    }
    
    @objc(deleteScreenshotAlertWithScreenshotCount:)
    final class func deleteScreenshots(count: Int) -> OEAlert {
        
        let alert = OEAlert()
        alert.messageText = .localizedStringWithFormat(NSLocalizedString("Are you sure you want to delete %ld screenshots from your OpenEmu library?", comment: ""), count)
        alert.defaultButtonTitle = NSLocalizedString("Delete Screenshots", comment: "")
        alert.alternateButtonTitle = NSLocalizedString("Cancel", comment: "")
        alert.showSuppressionButton(forUDKey: OEDeleteScreenshotAlertSuppressionKey)
        
        return alert
    }
    
    @objc(removeGamesFromCollectionAlert:)
    final class func removeGamesFromCollection(multipleGames: Bool) -> OEAlert {
        
        let alert = OEAlert()
        alert.messageText = multipleGames ? NSLocalizedString("Are you sure you want to remove the selected games from the collection?", comment: "") : NSLocalizedString("Are you sure you want to remove the selected game from the collection?", comment: "")
        alert.defaultButtonTitle = multipleGames ? NSLocalizedString("Remove Games", comment: "") : NSLocalizedString("Remove Game", comment: "")
        alert.alternateButtonTitle = NSLocalizedString("Cancel", comment: "")
        alert.showSuppressionButton(forUDKey: OERemoveGameFromCollectionAlertSuppressionKey)
        
        return alert
    }
    
    @objc(removeGamesFromLibraryAlert:)
    final class func removeGamesFromLibrary(multipleGames: Bool) -> OEAlert {
        
        let alert = OEAlert()
        alert.messageText = multipleGames ? NSLocalizedString("Are you sure you want to delete the selected games from your OpenEmu library?", comment: "") : NSLocalizedString("Are you sure you want to delete the selected game from your OpenEmu library?", comment: "")
        alert.defaultButtonTitle = multipleGames ? NSLocalizedString("Delete Games", comment: "") : NSLocalizedString("Delete Game", comment: "")
        alert.alternateButtonTitle = NSLocalizedString("Cancel", comment: "")
        alert.showSuppressionButton(forUDKey: OERemoveGameFromLibraryAlertSuppressionKey)
        
        return alert
    }
    
    @objc(removeGameFilesFromLibraryAlert:)
    final class func removeGameFilesFromLibrary(multipleGames: Bool) -> OEAlert {
        
        let alert = OEAlert()
        alert.messageText = multipleGames ? NSLocalizedString("Move selected games to Trash, or keep them in the Library folder?", comment: "") : NSLocalizedString("Move selected game to Trash, or keep it in the Library folder?", comment: "")
        alert.informativeText = NSLocalizedString("Only files in the OpenEmu Library folder will be moved to the Trash.", comment: "")
        alert.defaultButtonTitle = NSLocalizedString("Move to Trash", comment: "")
        alert.alternateButtonTitle = NSLocalizedString("Keep Files", comment: "")
        alert.showSuppressionButton(forUDKey: OERemoveGameFilesFromLibraryAlertSuppressionKey)
        alert.suppressOnDefaultReturnOnly = false
        
        return alert
    }
    
    @objc(removeCollectionAlertWithName:)
    final class func removeCollection(name: String) -> OEAlert {
        
        let alert = OEAlert()
        alert.messageText = .localizedStringWithFormat(NSLocalizedString("Are you sure you want to remove the collection '%@'?", comment: ""), name)
        alert.defaultButtonTitle = NSLocalizedString("Remove Collection", comment: "")
        alert.alternateButtonTitle = NSLocalizedString("Cancel", comment: "")
        alert.showSuppressionButton(forUDKey: OERemoveCollectionAlertSuppressionKey)
        
        return alert
    }
    
    @objc(resetSystemAlert)
    final class func resetSystem() -> OEAlert {
        
        let alert = OEAlert()
        alert.messageText = NSLocalizedString("Are you sure you want to restart the game?", comment: "")
        alert.defaultButtonTitle = NSLocalizedString("Restart", comment: "")
        alert.alternateButtonTitle = NSLocalizedString("Cancel", comment: "")
        alert.showSuppressionButton(forUDKey: OEResetSystemAlertSuppressionKey)
        
        return alert
    }
    
    @objc(stopEmulationAlert)
    final class func stopEmulation() -> OEAlert {
        
        let alert = OEAlert()
        alert.messageText = NSLocalizedString("Are you sure you want to quit the game?", comment: "")
        alert.defaultButtonTitle = NSLocalizedString("Quit", comment: "")
        alert.alternateButtonTitle = NSLocalizedString("Cancel", comment: "")
        alert.showSuppressionButton(forUDKey: OEStopEmulationAlertSuppressionKey)
        
        return alert
    }
    
    @objc(renameSpecialStateAlert)
    final class func renameSpecialSaveState() -> OEAlert {
        
        let alert = OEAlert()
        alert.messageText = NSLocalizedString("Rename Special Save State or something?", comment: "")
        alert.informativeText = NSLocalizedString("Won't be able to recognize it as special save state…", comment: "")
        alert.defaultButtonTitle = NSLocalizedString("Rename", comment: "")
        alert.alternateButtonTitle = NSLocalizedString("Cancel", comment: "")
        alert.showSuppressionButton(forUDKey: OERenameSpecialSaveStateAlertSuppressionKey)
        
        return alert
    }
    
    @objc(romDownloadRequiredAlert:)
    final class func romDownloadRequired(name: String) -> OEAlert {
        
        let alert = OEAlert()
        alert.messageText = NSLocalizedString("Game requires download", comment: "")
        alert.informativeText = NSLocalizedString("In order to play the game it must be downloaded.", comment: "")
        alert.defaultButtonTitle = NSLocalizedString("Download", comment: "")
        alert.alternateButtonTitle = NSLocalizedString("Cancel", comment: "")
        alert.showSuppressionButton(forUDKey: OEDownloadRomWarningSuppressionKey)
        
        return alert
    }
    
    final class func missingBIOSFiles(_ missingFilesList: String) -> OEAlert {
        
        let alert = OEAlert()
        alert.messageText = NSLocalizedString("Required files are missing.", comment: "")
        alert.informativeText = .localizedStringWithFormat(NSLocalizedString("To run this core you need the following:\n\n%@Drag and drop the required file(s) onto the game library window and try again.", comment: ""), missingFilesList)
        alert.defaultButtonTitle = NSLocalizedString("OK", comment: "")
        alert.alternateButtonTitle = NSLocalizedString("Learn More", comment: "")
        
        return alert
    }
}
