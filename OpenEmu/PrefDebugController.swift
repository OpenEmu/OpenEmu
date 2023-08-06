// Copyright (c) 2021, OpenEmu Team
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
import OpenEmuSystem
import OpenEmuKit

final class PrefDebugController: NSViewController {
    
    @IBOutlet var contentView: NSGridView!
    
    private var fittingWidth: CGFloat = 0
    
    private static let keyDescriptions: [Any] = [
        Group(label: "General"),
        Checkbox(key: PreferencesWindowController.debugModeKey, label: "Debug Mode"),
        Checkbox(key: SetupAssistant.hasFinishedKey, label: "Setup Assistant has finished"),
        Popover(key: OELocalizationHelper.OERegionKey, label: "Region:", action: #selector(changeRegion(_:)), options: [
            Option(label: "Auto (region)", value: -1),
            Option(label: "North America", value: 0),
            Option(label: "Japan", value: 1),
            Option(label: "Europe", value: 2),
            Option(label: "Other", value: 3),
        ]),
        Popover(key: OEGameCoreManagerModePreferenceKey, label: "Run games using:", action: #selector(changeGameMode(_:)), options: [
            Option(label: "XPC", value: OEXPCGameCoreManager.className()),
            Option(label: "Background Thread", value: OEThreadGameCoreManager.className()),
        ]),
        Popover(key: OEAppearance.Application.key, label: "Appearance:", action: #selector(changeAppAppearance(_:)), options: [
            Option(label: "System", value: OEAppearance.Application.system.rawValue),
            Option(label: "Dark (default)", value: OEAppearance.Application.dark.rawValue),
            Option(label: "Light", value: OEAppearance.Application.light.rawValue),
        ]),
        
        Separator(),
        
        Group(label: "Library Window"),
        Checkbox(key: OEImportManualSystems, label: "Manually choose system on import"),
        Checkbox(key: OEDBSaveStatesMedia.showsAutoSavesKey, label: "Show autosave states in save state category"),
        Checkbox(key: OEDBSaveStatesMedia.showsQuickSavesKey, label: "Show quicksave states in save state category"),
        Checkbox(key: Key.useNewSaveStatesViewController.rawValue, label: "Use new save states view"),
        Checkbox(key: Key.useNewScreenshotsViewController.rawValue, label: "Use new screenshots view"),
        Button(label: "Reset main window size", action: #selector(resetMainWindow(_:))),
        Button(label: "Toggle game scanner view", action: #selector(toggleGameScannerView(_:))),
        
        Separator(),
        
        Group(label: "HUD Bar / Gameplay"),
        Checkbox(key: OEGameLayerNotificationView.OEShowNotificationsKey, label: "Show notifications during gameplay"),
        Checkbox(key: OEDBSaveState.useQuickSaveSlotsKey, label: "Use quicksave slots"),
        Checkbox(key: GameControlsBar.showsQuickSaveStateKey, label: "Show quicksave in menu"),
        Checkbox(key: GameControlsBar.showsAutoSaveStateKey, label: "Show autosave in menu"),
        Checkbox(key: GameControlsBar.showsAudioOutputKey, label: "Show audio output device in menu"),
        Checkbox(key: OETakeNativeScreenshots, label: "Take screenshots in native size"),
        Checkbox(key: OEScreenshotAspectRatioCorrectionDisabled, label: "Disable aspect ratio correction in screenshots"),
        Checkbox(key: OEPopoutGameWindowTreatScaleFactorAsPixels, label: "Change scale menu unit from points to pixels"),
        Checkbox(key: OEAdaptiveSyncEnabledKey, label: "Enable adaptive sync for supported displays (full screen)"),
        Popover(key: OEAppearance.HUDBar.key, label: "Appearance:", action: #selector(changeHUDBarAppearance(_:)), options: [
            Option(label: "Vibrant", value: OEAppearance.HUDBar.vibrant.rawValue),
            Option(label: "Dark", value: OEAppearance.HUDBar.dark.rawValue),
        ]),
        ColorWell(key: OEPopoutGameWindowBackgroundColorKey, label: "Game View Background color:"),
        
        Separator(),
        
        Group(label: "Controls Setup"),
        Checkbox(key: ControlsButtonSetupView.controlsButtonHighlightRollsOver, label: "Select first field after setting the last"),
        Checkbox(key: ControllerImageView.drawControllerMaskKey, label: "Draw button mask above image"),
        Checkbox(key: "logsHIDEvents", label: "Log HID Events"),
        Checkbox(key: "logsHIDEventsNoKeyboard", label: "Log Keyboard Events"),
        Checkbox(key: OEPrefControlsShowAllGlobalKeys, label: "Show all special keys"),
        Popover(key: OEAppearance.ControlsPrefs.key, label: "Appearance:", action: #selector(changeControlsPrefsAppearance(_:)), options: [
            Option(label: "Wood", value: OEAppearance.ControlsPrefs.wood.rawValue),
            Option(label: "Vibrant", value: OEAppearance.ControlsPrefs.vibrant.rawValue),
            Option(label: "Vibrant Wood", value: OEAppearance.ControlsPrefs.woodVibrant.rawValue),
        ]),
        NumericTextField(key: "OESystemResponderADCThreshold", label: "Threshold for analog controls bound to buttons:", numberFormatter: NumericTextField.NF(allowsFloats: true, minimum: 0.01, maximum: 0.99, numberStyle: .decimal)),
        
        Separator(),
        
        Group(label: "Save States"),
        Button(label: "Set default save states directory", action: #selector(restoreSaveStatesDirectory(_:))),
        Button(label: "Cleanup autosave state", action: #selector(cleanupAutoSaveStates(_:))),
        Button(label: "Cleanup Save States", action: #selector(cleanupSaveStates(_:))),
        
        Separator(),
        
        Group(label: "Shaders"),
        Button(label: "Clear shader cache", action: #selector(clearShaderCache(_:))),
        Button(label: "Download additional shaders", action: #selector(downloadShaders(_:))),
        Button(label: "Reveal user shader folder", action: #selector(openUserShaderFolder(_:))),
        
        Separator(),
        
        Group(label: "OpenVGDB"),
        Button(label: "Update OpenVGDB", action: #selector(updateOpenVGDB(_:))),
        
        Separator(),
        
        Group(label: "Database Actions"),
        Button(label: "Delete useless image objects", action: #selector(removeUselessImages(_:))),
        Button(label: "Delete artwork that can be downloaded", action: #selector(removeArtworkWithRemoteBacking(_:))),
        Button(label: "Sync games without artwork", action: #selector(syncGamesWithoutArtwork(_:))),
        Button(label: "Download missing artwork…", action: #selector(downloadMissingArtwork(_:))),
        Button(label: "Remove untracked artwork files", action: #selector(removeUntrackedImageFiles(_:))),
        Button(label: "Cleanup rom hashes", action: #selector(cleanupHashes(_:))),
        Button(label: "Remove duplicated roms", action: #selector(removeDuplicatedRoms(_:))),
        Button(label: "Cancel cover sync for all games", action: #selector(cancelCoverArtSync(_:))),
        Label(label: ""),
        Button(label: "Perform Sanity Check on Database", action: #selector(sanityCheck(_:))),
    ]
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        setUpGridView()
    }
    
    private func setUpGridView() {
        
        let gridView = NSGridView(numberOfColumns: 2, rows: 0)
        gridView.column(at: 0).xPlacement = .trailing
        gridView.rowAlignment = .firstBaseline
        
        for item in Self.keyDescriptions {
            createRow(for: gridView, item: item)
        }
        
        gridView.setContentHuggingPriority(.defaultLow-1, for: .horizontal)
        gridView.setContentHuggingPriority(.defaultHigh-1, for: .vertical)
        gridView.translatesAutoresizingMaskIntoConstraints = false
        contentView.addSubview(gridView)
        
        let scrollerWidth = NSScroller.scrollerWidth(for: .regular, scrollerStyle: .overlay)
        var fittingSize = gridView.fittingSize
        fittingSize.width += 60 - scrollerWidth
        fittingSize.height += 40
        contentView.setFrameSize(fittingSize)
        contentView.enclosingScrollView?.contentView.scroll(to: NSPoint(x: 0, y: fittingSize.height))
        
        fittingSize.width += scrollerWidth
        fittingWidth = fittingSize.width
        
        NSLayoutConstraint.activate([
            gridView.leadingAnchor.constraint(equalTo: contentView.leadingAnchor, constant: 30),
            gridView.topAnchor.constraint(equalTo: contentView.topAnchor, constant: 20)
        ])
    }
    
    private func createRow(for gridView: NSGridView, item: Any) {
        
        let defaultDefaults = UserDefaults.standard.volatileDomain(forName: UserDefaults.registrationDomain)
        
        if let item = item as? Checkbox {
            let label = NSLocalizedString(item.label, tableName: "Debug", comment: "")
            let key = item.key
            let negated = item.negated
            
            let checkbox = NSButton(checkboxWithTitle: label, target: nil, action: nil)
            checkbox.title = label
            
            var options: [NSBindingOption : Any] = [.continuouslyUpdatesValue: true as NSNumber]
            if negated {
                options[.valueTransformerName] = NSValueTransformerName.negateBooleanTransformerName
            }
            
            checkbox.bind(.value, to: NSUserDefaultsController.shared, withKeyPath: "values.\(key)", options: options)
            
            if let originalValue = defaultDefaults[key] as? NSNumber {
                let origbool = originalValue.boolValue != negated
                let fmt = NSLocalizedString("Default Value: %@", tableName: "Debug", comment: "Default value tooltip format in the Debug Preferences")
                let val = origbool
                    ? NSLocalizedString("Checked", tableName: "Debug", comment: "Default value tooltip for checkboxes: checked default")
                    : NSLocalizedString("Unchecked", tableName: "Debug", comment: "Default value tooltip for checkboxes: unchecked default")
                
                checkbox.toolTip = String(format: fmt, val)
            }
            
            gridView.addRow(with: [NSGridCell.emptyContentView, checkbox])
        }
        else if let item = item as? ColorWell {
            let label = NSLocalizedString(item.label, tableName: "Debug", comment: "")
            let key = item.key
            
            let labelField = NSTextField(labelWithString: label)
            labelField.alignment = .right
            let colorWell: NSColorWell
            if #available(macOS 13.0, *) {
            #if swift(>=5.7)
                colorWell = NSColorWell(style: .minimal)
            #else
                colorWell = NSColorWell()
            #endif
            } else {
                colorWell = NSColorWell()
            }
            
            if let colorString = UserDefaults.standard.string(forKey: key),
               let color = NSColor(from: colorString) {
                colorWell.color = color
            } else {
                colorWell.color = .black
            }
            colorWell.action = #selector(changeColor(_:))
            colorWell.target = self
            colorWell.stringValue = item.key
            colorWell.autoresizingMask = [.width, .height]
            
            let row = gridView.addRow(with: [labelField, colorWell])
            row.rowAlignment = .none
            row.yPlacement = .center
            
            NSLayoutConstraint.activate([
                colorWell.widthAnchor.constraint(equalToConstant: 60),
                colorWell.heightAnchor.constraint(equalToConstant: 24)
            ])
        }
        else if let item = item as? Group {
            let label = NSLocalizedString(item.label, tableName: "Debug", comment: "")
            
            let field = NSTextField(labelWithString: label)
            field.font = NSFont.boldSystemFont(ofSize: 0)
            
            let row = gridView.addRow(with: [NSGridCell.emptyContentView, field])
            row.bottomPadding = 4
        }
        else if let item = item as? Label {
            let label = NSLocalizedString(item.label, tableName: "Debug", comment: "")
            
            let labelField = NSTextField(labelWithString: label)
            
            gridView.addRow(with: [NSGridCell.emptyContentView, labelField])
        }
        else if let item = item as? Button {
            let label = NSLocalizedString(item.label, tableName: "Debug", comment: "")
            let action = item.action
            
            let button = NSButton(title: label, target: self, action: action)
            
            gridView.addRow(with: [NSGridCell.emptyContentView, button])
        }
        else if let item = item as? Popover {
            let label = NSLocalizedString(item.label, tableName: "Debug", comment: "")
            let options = item.options
            let action = item.action
            
            let labelField = NSTextField(labelWithString: label)
            labelField.alignment = .right
            
            let popup = NSPopUpButton()
            popup.action = action
            popup.target = self
            
            let menu = NSMenu()
            for option in options {
                let item = NSMenuItem()
                item.title = NSLocalizedString(option.label, tableName: "Debug", comment: "")
                item.representedObject = option.value
                menu.addItem(item)
            }
            popup.menu = menu
            
            setUpSelectedItem(for: popup, item: item)
            popup.sizeToFit()
            
            gridView.addRow(with: [labelField, popup])
        }
        else if let item = item as? NumericTextField {
            let label = NSLocalizedString(item.label, tableName: "Debug", comment: "")
            let nf = item.numberFormatter
            let key = item.key
            
            let numberFormatter = NumberFormatter()
            numberFormatter.allowsFloats = nf.allowsFloats
            numberFormatter.minimum = nf.minimum
            numberFormatter.maximum = nf.maximum
            numberFormatter.numberStyle = nf.numberStyle
            
            let labelField = NSTextField(labelWithString: label)
            labelField.alignment = .right
            
            let inputField = NSTextField(string: "")
            inputField.formatter = numberFormatter
            inputField.bind(.value, to: NSUserDefaultsController.shared, withKeyPath: "values.\(key)", options: nil)
            
            let validRangeFormat = NSLocalizedString("Range: %@ to %@", tableName: "Debug", comment: "Range indicator tooltip for numeric text boxes in the Debug Preferences")
            let min = numberFormatter.string(from: nf.minimum) ?? ""
            let max = numberFormatter.string(from: nf.maximum) ?? ""
            var tooltip = String(format: validRangeFormat, min, max)
            
            if let defaultv = defaultDefaults[key] as? NSNumber {
                let fmt = NSLocalizedString("Default Value: %@", tableName: "Debug", comment: "Default value tooltip format in the Debug Preferences")
                let defaultstr = numberFormatter.string(from: defaultv) ?? ""
                tooltip += "\n" + String(format: fmt, defaultstr)
            }
            inputField.toolTip = tooltip
            
            gridView.addRow(with: [labelField, inputField])
            inputField.widthAnchor.constraint(equalToConstant: 70).isActive = true
        }
        else if item is Separator {
            let separator = NSBox()
            separator.boxType = .separator
            
            let row = gridView.addRow(with: [separator])
            row.mergeCells(in: NSRange(location: 0, length: 2))
            row.topPadding = 6
            row.bottomPadding = 6
        }
    }
    
    private func setUpSelectedItem(for button: NSPopUpButton, item: Popover) {
        
        let key = item.key
        let currentValue = UserDefaults.standard.object(forKey: key)
        
        let index = button.indexOfItem(withRepresentedObject: currentValue)
        if index != -1 {
            button.selectItem(at: index)
        }
    }
}

// MARK: - Actions

@objc private extension PrefDebugController {
    
    func changeRegion(_ sender: NSPopUpButton) {
        let item = sender.selectedItem
        let value = (item?.representedObject as? Int) ?? -1
        
        let defaults = UserDefaults.standard
        if value == -1 {
            defaults.removeObject(forKey: OELocalizationHelper.OERegionKey)
        } else {
            defaults.set(value, forKey: OELocalizationHelper.OERegionKey)
        }
        
        NotificationCenter.default.post(Notification(name: .OEDBSystemAvailabilityDidChange))
    }
    
    func changeGameMode(_ sender: NSPopUpButton) {
        let selectedItem = sender.selectedItem
        UserDefaults.standard.set(selectedItem?.representedObject, forKey: OEGameCoreManagerModePreferenceKey)
    }
    
    func changeAppAppearance(_ sender: NSPopUpButton) {
        let selectedItem = sender.selectedItem
        UserDefaults.standard.set(selectedItem?.representedObject, forKey: OEAppearance.Application.key)
    }
    
    func changeHUDBarAppearance(_ sender: NSPopUpButton) {
        let selectedItem = sender.selectedItem
        UserDefaults.standard.set(selectedItem?.representedObject, forKey: OEAppearance.HUDBar.key)
    }
    
    func changeControlsPrefsAppearance(_ sender: NSPopUpButton) {
        let selectedItem = sender.selectedItem
        UserDefaults.standard.set(selectedItem?.representedObject, forKey: OEAppearance.ControlsPrefs.key)
        
        let alert = OEAlert()
        alert.messageText = NSLocalizedString("You need to restart the application to commit the change", comment: "")
        alert.addButton(withTitle: NSLocalizedString("OK", comment: ""))
        alert.runModal()
    }
    
    func changeColor(_ sender: NSColorWell) {
        let color = sender.color.hexString ?? "#000000"
        UserDefaults.standard.set(color, forKey: sender.stringValue)
    }
    
    // MARK: - Library Window
    
    func resetMainWindow(_ sender: Any?) {
        
        let defaults = UserDefaults.standard
        defaults.removeObject(forKey: "NSWindow Frame LibraryWindow")
        defaults.removeObject(forKey: "NSSplitView Subview Frames OELibraryGamesSplitView")
        defaults.removeObject(forKey: OELastGridSizeKey)
        
        let mainWindow = NSApp.windows.first(where: { $0.windowController is MainWindowController })
        
        // Matches the content size specified in MainWindow.xib.
        mainWindow?.setFrame(NSRect(x: 0, y: 0, width: 845, height: 555 + 22), display: false)
        mainWindow?.center()
        
        NotificationCenter.default.post(Notification(name: .OELibrarySplitViewResetSidebar))
    }
    
    func toggleGameScannerView(_ sender: Any?) {
        NotificationCenter.default.post(Notification(name: GameScannerViewController.OEGameScannerToggleNotification))
    }
    
    // MARK: - OpenVGDB
    
    func updateOpenVGDB(_ sender: Any?) {
        DLog("Removing OpenVGDB update check date and version from user defaults to force update.")
        
        let defaults = UserDefaults.standard
        defaults.removeObject(forKey: OpenVGDB.updateCheckKey)
        defaults.removeObject(forKey: OpenVGDB.versionKey)
        
        let helper = OpenVGDB.shared
        helper.checkForUpdates { url, version in
            if let url = url, let version = version {
                helper.install(with: url, version: version)
            }
        }
    }
    
    // MARK: - Shaders
    
    func clearShaderCache(_ sender: Any?) {
        if let url = OEShaderStore.shared.shadersCachePath {
            try? FileManager.default.removeItem(at: url)
        }
    }
    
    func downloadShaders(_ sender: Any?) {
        let downloadURL = URL(string: "https://github.com/OpenEmu/slang-shaders/releases/download/")!
        let releasesURL = URL(string: "https://api.github.com/repos/OpenEmu/slang-shaders/releases?page=1&per_page=1")!
        
        var request = URLRequest(url: releasesURL, cachePolicy: .reloadIgnoringCacheData, timeoutInterval: 30)
        request.setValue("OpenEmu", forHTTPHeaderField: "User-Agent")
        
        let task = URLSession.shared.dataTask(with: request) { result, response, error in
            
            if let result = result,
               let releases = try? JSONSerialization.jsonObject(with: result, options: .allowFragments) as? [AnyHashable],
               let release = releases.first as? [AnyHashable : Any],
               let tagName = release["tag_name"] as? String {
                let url = downloadURL.appendingPathComponent("\(tagName)/Shaders-\(tagName).zip")
                
                DispatchQueue.main.async {
                    let request = URLRequest(url: url)
                    let downloadSession = URLSession(configuration: .default)
                    let downloadTask = downloadSession.downloadTask(with: request) { location, response, error in
                        
                        if let location = location {
                            
                            let tmpDir = FileManager.default.temporaryDirectory
                                .appendingPathComponent("org.openemu.OpenEmu", isDirectory: true)
                                .appendingPathComponent(UUID().uuidString, isDirectory: true)
                            
                            ArchiveHelper.decompressFileInArchive(at: location, toDirectory: tmpDir)
                            
                            let urls = try? FileManager.default.contentsOfDirectory(at: tmpDir, includingPropertiesForKeys: nil)
                            for url in urls ?? [] {
                                ImportOperation.importShaderPlugin(at: url)
                            }
                            
                            try? FileManager.default.removeItem(at: tmpDir)
                        }
                    }
                    
                    downloadTask.resume()
                }
            }
        }
        
        task.resume()
    }
    
    func openUserShaderFolder(_ sender: Any?) {
        if let url = OEShaderStore.shared.userShadersPath {
            NSWorkspace.shared.open(url)
        }
    }
    
    // MARK: - Save States
    
    func restoreSaveStatesDirectory(_ sender: Any?) {
        UserDefaults.standard.removeObject(forKey: OELibraryDatabase.saveStateFolderURLKey)
    }
    
    func cleanupAutoSaveStates(_ sender: Any?) {
        OELibraryDatabase.default?.cleanupAutoSaveStates()
    }
    
    func cleanupSaveStates(_ sender: Any?) {
        OELibraryDatabase.default?.cleanupSaveStates()
    }
    
    // MARK: - Database Actions
    
    func removeUselessImages(_ sender: Any?) {
        OELibraryDatabase.default?.removeUselessImages()
    }
    
    func removeArtworkWithRemoteBacking(_ sender: Any?) {
        OELibraryDatabase.default?.removeArtworkWithRemoteBacking()
    }
    
    func syncGamesWithoutArtwork(_ sender: Any?) {
        OELibraryDatabase.default?.syncGamesWithoutArtwork()
    }
    
    func downloadMissingArtwork(_ sender: Any?) {
        OELibraryDatabase.default?.downloadMissingArtwork()
    }
    
    func removeUntrackedImageFiles(_ sender: Any?) {
        OELibraryDatabase.default?.removeUntrackedImageFiles()
    }
    
    func cleanupHashes(_ sender: Any?) {
        OELibraryDatabase.default?.cleanupHashes()
    }
    
    func removeDuplicatedRoms(_ sender: Any?) {
        OELibraryDatabase.default?.removeDuplicatedRoms()
    }
    
    func cancelCoverArtSync(_ sender: Any?) {
        OELibraryDatabase.default?.cancelCoverArtSync()
    }
    
    func sanityCheck(_ sender: Any?) {
        OELibraryDatabase.default?.sanityCheck()
    }
}

// MARK: - Structs

private extension PrefDebugController {
    
    struct Separator {
    }
    struct Group {
        let label: String
    }
    struct Label {
        let label: String
    }
    struct Checkbox {
        let key: String
        let label: String
        var negated: Bool = false
    }
    struct Popover {
        let key: String
        let label: String
        let action: Selector
        let options: [Option]
    }
    struct Option {
        let label: String
        let value: Any
    }
    struct Button {
        let label: String
        let action: Selector
    }
    struct ColorWell {
        let key: String
        let label: String
    }
    struct NumericTextField {
        let key: String
        let label: String
        let numberFormatter: NF
        
        struct NF {
            var allowsFloats: Bool
            var minimum: NSNumber
            var maximum: NSNumber
            var numberStyle: NumberFormatter.Style
        }
    }
}

// MARK: - PreferencePane

extension PrefDebugController: PreferencePane {
    
    var icon: NSImage? { NSImage(named: "debug_tab_icon") }
    
    var panelTitle: String { "Secrets" }
    
    var viewSize: NSSize { NSSize(width: fittingWidth, height: 500) }
}
