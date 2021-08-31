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

import Cocoa

final class PrefLibraryController: NSViewController {
    
    @IBOutlet var availableLibrariesViewController: AvailableLibrariesViewController!
    @IBOutlet var librariesView: NSView!
    @IBOutlet var pathField: NSPathControl!
    @IBOutlet var resetLocationButton: NSButton!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        pathField.url = OELibraryDatabase.default?.databaseFolderURL
        showResetLocationButtonIfNeeded()
        
        availableLibrariesViewController.isEnableObservers = true
        addChild(availableLibrariesViewController)
        
        let size = librariesView.frame.size
        let scrollView = availableLibrariesViewController.view as! NSScrollView
        let gridView = librariesView.superview as! NSGridView
        gridView.cell(for: librariesView)?.contentView = scrollView
        librariesView.removeFromSuperview()
        librariesView = scrollView
        
        scrollView.borderType = .bezelBorder
        NSLayoutConstraint.activate([
            scrollView.widthAnchor.constraint(equalToConstant: size.width),
            scrollView.heightAnchor.constraint(equalToConstant: size.height)
        ])
    }
    
    deinit {
        availableLibrariesViewController.isEnableObservers = false
    }
    
    // MARK: - Actions
    
    @IBAction func resetLibraryFolder(_ sender: Any?) {
        
        let defaultDatabasePath = (UserDefaults.standard.string(forKey: OEDefaultDatabasePathKey)! as NSString).expandingTildeInPath
        let location = URL(fileURLWithPath: defaultDatabasePath, isDirectory: true).deletingLastPathComponent()
        
        OELibraryDatabase.default?.move(to: location)
        
        pathField.url = OELibraryDatabase.default?.databaseFolderURL
        showResetLocationButtonIfNeeded()
    }
    
    @IBAction func changeLibraryFolder(_ sender: Any?) {
        
        let alert = OEAlert()
        alert.messageUsesHTML = true
        alert.messageText = NSLocalizedString(
            "Moving the Game Library is not recommended",
            comment: "Message headline (attempted to change location of library)")
        alert.informativeText = NSLocalizedString(
            """
                The OpenEmu Game Library contains a database file which could get \
                corrupted if the library is moved to the following locations:<br><br>\
                <ul><li>Folders managed by cloud synchronization software (like <b>iCloud Drive</b>)</li>\
                <li>Network drives</li></ul><br>\
                Additionally, <b>sharing the same library between multiple computers or users</b> \
                may also corrupt it. This also applies to moving the library \
                to external USB drives.
            """,
            comment: "Message text (attempted to change location of library, HTML)")
        alert.defaultButtonTitle = NSLocalizedString("Cancel", comment: "")
        alert.alternateButtonTitle = NSLocalizedString("I understand the risks",
            comment: "OK button (attempted to change location of library)")
        alert.beginSheetModal(for: view.window!) { result in
            if result == .alertSecondButtonReturn {
                self.moveGameLibrary()
            }
        }
    }
    
    private func moveGameLibrary() {
        
        let openPanel = NSOpenPanel()
        openPanel.canChooseFiles = false
        openPanel.canChooseDirectories = true
        openPanel.canCreateDirectories = true
        openPanel.beginSheetModal(for: view.window!) { result in
            if result == .OK, let url = openPanel.url {
                // give the openpanel some time to fade out
                DispatchQueue.main.async {
                    OELibraryDatabase.default?.move(to: url)
                    self.pathField.url = OELibraryDatabase.default?.databaseFolderURL
                    self.showResetLocationButtonIfNeeded()
                }
            }
        }
    }
    
    private func showResetLocationButtonIfNeeded() {
        
        let defaultDatabasePath = (UserDefaults.standard.string(forKey: OEDefaultDatabasePathKey)! as NSString).expandingTildeInPath
        let defaultLocation = URL(fileURLWithPath: defaultDatabasePath, isDirectory: true)
        
        let currentLocation = OELibraryDatabase.default?.databaseFolderURL
        
        resetLocationButton.isHidden = currentLocation == defaultLocation
    }
    
    @IBAction func resetWarningDialogs(_ sender: Any?) {
        
        let keysToRemove: [String] = [
            OEAlert.OEDeleteGameAlertSuppressionKey,
            OEAlert.OEDeleteSaveStateAlertSuppressionKey,
            OEAlert.OEDeleteScreenshotAlertSuppressionKey,
            OEAlert.OERemoveCollectionAlertSuppressionKey,
            OEAlert.OERemoveGameFromCollectionAlertSuppressionKey,
            OEAlert.OERemoveGameFromLibraryAlertSuppressionKey,
            OEAlert.OERenameSpecialSaveStateAlertSuppressionKey,
            OEAlert.OELoadAutoSaveAlertSuppressionKey,
            OEAlert.OEDownloadRomWarningSuppressionKey,
            OEAlert.OESaveGameAlertSuppressionKey,
            OEAlert.OEResetSystemAlertSuppressionKey,
            OEAlert.OEStopEmulationAlertSuppressionKey,
            OEAlert.OEChangeCoreAlertSuppressionKey,
            OEAlert.OEAutoSwitchCoreAlertSuppressionKey,
            OEAlert.OEGameCoreGlitchesSuppressionKey,
        ]
        
        keysToRemove.forEach { key in
            UserDefaults.standard.removeObject(forKey: key)
        }
    }
}

// MARK: - PreferencePane

extension PrefLibraryController: PreferencePane {
    
    var icon: NSImage? { NSImage(named: "library_tab_icon") }
    
    var panelTitle: String { "Library" }
    
    var viewSize: NSSize { view.fittingSize }
}
