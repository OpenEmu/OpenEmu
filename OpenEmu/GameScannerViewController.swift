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
import OpenEmuSystem

final class GameScannerViewController: NSViewController {

    @objc public static let OEGameScannerToggleNotification = Notification.Name("OEGameScannerToggleNotification")
    private static let importGuideURL = URL(string: "https://github.com/OpenEmu/OpenEmu/wiki/User-guide:-Importing")!

    @IBOutlet weak var scannerView: NSView!
    
    @IBOutlet var headlineLabel: NSTextField!
    @IBOutlet var togglePauseButton: GameScannerButton!
    @IBOutlet var progressIndicator: NSProgressIndicator!
    @IBOutlet var statusLabel: NSTextField!
    @IBOutlet var fixButton: TextButton!
    
    @IBOutlet var issuesView: NSTableView!
    @IBOutlet var actionPopUpButton: NSPopUpButton!
    @IBOutlet var applyButton: NSButton!
    
    @IBOutlet private weak var sourceListScrollView: NSScrollView!
    
    private var itemsRequiringAttention = [ImportOperation]()
    private var itemsFailedImport = [ImportOperation]()
    private var isScanningDirectory = false
    private var isGameScannerVisible = true // The game scanner view is already visible in SidebarController.xib.
    
    private var importer: ROMImporter { return OELibraryDatabase.default!.importer }
    
    required init?(coder: NSCoder) {
        
        super.init(coder: coder)
        
        let notificationCenter = NotificationCenter.default
        notificationCenter.addObserver(self, selector: #selector(gameInfoHelperWillUpdate(_:)), name: .GameInfoHelperWillUpdate, object: nil)
        notificationCenter.addObserver(self, selector: #selector(gameInfoHelperDidChangeUpdateProgress(_:)), name: .GameInfoHelperDidChangeUpdateProgress, object: nil)
        notificationCenter.addObserver(self, selector: #selector(gameInfoHelperDidUpdate(_:)), name: .GameInfoHelperDidUpdate, object: nil)
        notificationCenter.addObserver(self, selector: #selector(toggleGameScannerView), name: GameScannerViewController.OEGameScannerToggleNotification, object: nil)
    }
    
    override var nibName: NSNib.Name? { "GameScanner" }
    
    override func awakeFromNib() {
        
        super.awakeFromNib()
        
        _ = view // load other xib as well
        
        guard let issuesView = issuesView else { return }
        
        setUpActionsMenu()
        
        var item: NSMenuItem!
        let menu = NSMenu()
        
        item = NSMenuItem(title: NSLocalizedString("Select All", comment: ""), action: #selector(selectAll(_:)), keyEquivalent: "")
        item.target = self
        menu.addItem(item)
        
        item = NSMenuItem(title: NSLocalizedString("Deselect All", comment: ""), action: #selector(deselectAll(_:)), keyEquivalent: "")
        item.target = self
        menu.addItem(item)
        
        issuesView.menu = menu
        
        statusLabel.font = .monospacedDigitSystemFont(ofSize: 11, weight: .regular)
        fixButton.font = .monospacedDigitSystemFont(ofSize: 11, weight: .bold)
        
        layOutSidebarViews(withVisibleGameScanner: false, animated: false)
        
        itemsRequiringAttention = []
        
        importer.delegate = self
        
        // Show game scanner if importer is running already or game info is downloading.
        if importer.status == .running || OpenVGDB.shared.isUpdating {
            updateProgress()
            showGameScannerView(animated: true)
        }
    }
    
    override func viewWillAppear() {
        
        super.viewWillAppear()
        
        var styleMask = self.view.window!.styleMask
        styleMask.remove(.resizable)
        self.view.window!.styleMask = styleMask
        
        issuesView.reloadData()
        enableOrDisableApplyButton()
    }
    
    private func setUpActionsMenu() {
        
        let systemIDs = Set(itemsRequiringAttention.flatMap { $0.systemIdentifiers })
        
        let menu = NSMenu()
        
        let context = OELibraryDatabase.default!.mainThreadContext
        let menuItems: [NSMenuItem] = systemIDs.compactMap { systemID in
            
            guard let system = OEDBSystem.system(for: systemID, in: context) else { return nil }
            let menuItem = NSMenuItem(title: system.name, action: nil, keyEquivalent: "")
            menuItem.image = system.icon
            menuItem.representedObject = systemID
            
            return menuItem
            
        }.sorted { $0.title.localizedStandardCompare($1.title) == .orderedAscending }
        
        menu.addItem(withTitle: NSLocalizedString("Don't Import Selected", comment: ""), action: nil, keyEquivalent: "")
        
        menu.addItem(.separator())
        
        for item in menuItems {
            menu.addItem(item)
        }
        
        actionPopUpButton.menu = menu
    }
    
    private func layOutSidebarViews(withVisibleGameScanner visibleGameScanner: Bool, animated: Bool) {
        
        var gameScannerFrame = scannerView.frame
        gameScannerFrame.origin.y = visibleGameScanner ? 0 : -gameScannerFrame.height
        
        var sourceListFrame = sourceListScrollView.frame
        sourceListFrame.origin.y = gameScannerFrame.maxY
        sourceListFrame.size.height = sourceListScrollView.superview!.frame.height - sourceListFrame.minY
        
        let reduceMotion = NSWorkspace.shared.accessibilityDisplayShouldReduceMotion
        if animated && !reduceMotion {
            
            NSAnimationContext.beginGrouping()
            
            // Set frames through animator proxies to implicitly animate changes.
            scannerView.animator().frame = gameScannerFrame
            sourceListScrollView.animator().frame = sourceListFrame
            
            NSAnimationContext.endGrouping()
            
        } else {
            
            // Set frames directly without implicit animations.
            scannerView.frame = gameScannerFrame
            sourceListScrollView.frame = sourceListFrame
        }
        
        isGameScannerVisible = visibleGameScanner
    }
    
    @objc private func updateProgress() {
        
        NSObject.cancelPreviousPerformRequests(withTarget: self, selector: #selector(updateProgress), object: nil)
        
        CATransaction.begin()
        defer { CATransaction.commit() }
        
        headlineLabel.stringValue = NSLocalizedString("Game Scanner", comment: "")
        
        let helper = OpenVGDB.shared
        if helper.isUpdating {
            
            progressIndicator.minValue = 0
            progressIndicator.doubleValue = helper.downloadProgress
            progressIndicator.maxValue = 1
            progressIndicator.isIndeterminate = false
            progressIndicator.startAnimation(self)
            
            fixButton.isHidden = true
            togglePauseButton.isHidden = true
            statusLabel.stringValue = NSLocalizedString("Downloading Game DB", comment: "")
            
        } else {
            
            let maxItems = importer.totalNumberOfItems
            
            progressIndicator.minValue = 0
            progressIndicator.doubleValue = Double(importer.numberOfProcessedItems)
            progressIndicator.maxValue = Double(maxItems)
            
            togglePauseButton.isHidden = false
            
            var status: String
            
            switch importer.status {
                
            case .running:
                
                progressIndicator.isIndeterminate = false
                progressIndicator.startAnimation(self)
                
                let count = min(importer.numberOfProcessedItems + 1, maxItems)
                
                if isScanningDirectory {
                    status = NSLocalizedString("Scanning Directory", comment: "")
                } else {
                    status = String(format: NSLocalizedString("Game %ld of %ld", comment: ""), count, maxItems)
                }
                
                togglePauseButton.isEnabled = true
                togglePauseButton.icon = .pause
                
            case .stopped:
                
                progressIndicator.stopAnimation(self)
                progressIndicator.isIndeterminate = true
                
                status = NSLocalizedString("Done", comment: "")
                
                togglePauseButton.isEnabled = !itemsRequiringAttention.isEmpty
                togglePauseButton.icon = itemsRequiringAttention.isEmpty ? .pause : .cancel
                
            case .paused:
                
                progressIndicator.stopAnimation(self)
                progressIndicator.isIndeterminate = true
                
                status = NSLocalizedString("Scanner Paused", comment: "")
                
                togglePauseButton.isEnabled = true
                togglePauseButton.icon = .continue
            }
            
            let shouldHideFixButton: Bool
            
            if !itemsRequiringAttention.isEmpty {
                
                fixButton.title = itemsRequiringAttention.count > 1 ?
                    String(format: NSLocalizedString("Resolve %ld Issues", comment: ""), itemsRequiringAttention.count) :
                    String(format: NSLocalizedString("Resolve %ld Issue", comment: ""), itemsRequiringAttention.count)
                
                shouldHideFixButton = false
                
                status = ""
                
            } else {
                shouldHideFixButton = true
            }
            
            fixButton.isHidden = shouldHideFixButton
            statusLabel.stringValue = status
        }
    }
    
    private func enableOrDisableApplyButton() {
        applyButton.isEnabled = itemsRequiringAttention.contains { $0.isChecked }
    }
    
    func showGameScannerView(animated: Bool = true) {
        layOutSidebarViews(withVisibleGameScanner: true, animated: animated)
    }
    
    func hideGameScannerView(animated: Bool = true) {
        guard itemsRequiringAttention.isEmpty else { return }
        layOutSidebarViews(withVisibleGameScanner: false, animated: animated)
    }
    
    @objc func toggleGameScannerView() {
        if isGameScannerVisible {
            hideGameScannerView()
        } else {
            showGameScannerView()
        }
    }
    
    @objc override func selectAll(_ sender: Any?) {
        for item in itemsRequiringAttention {
            item.isChecked = true
        }
        issuesView.reloadData()
        enableOrDisableApplyButton()
    }
    
    @objc func deselectAll(_ sender: Any?) {
        for item in itemsRequiringAttention {
            item.isChecked = false
        }
        issuesView.reloadData()
        enableOrDisableApplyButton()
    }
    
    @IBAction func resolveIssues(_ sender: Any?) {
        
        let selectedItemIndexes = itemsRequiringAttention.indices.filter { itemsRequiringAttention[$0].isChecked }
        
        issuesView.beginUpdates()
        issuesView.removeRows(at: IndexSet(selectedItemIndexes), withAnimation: .effectGap)
        issuesView.endUpdates()
        
        if let selectedSystemID = actionPopUpButton.selectedItem?.representedObject as? String {
            
            for item in selectedItemIndexes.map({ itemsRequiringAttention[$0] }) {
                item.systemIdentifiers = [selectedSystemID]
                item.importer.rescheduleOperation(item)
                item.completionBlock = nil
                item.completionHandler = nil
            }
            
        } else {
            
            for item in selectedItemIndexes.map({ itemsRequiringAttention[$0] }) {
                if let extractedFileURL = item.extractedFileURL {
                    try? FileManager.default.removeItem(at: extractedFileURL)
                }
            }
        }
        
        for i in selectedItemIndexes.reversed() {
            itemsRequiringAttention.remove(at: i)
        }
        
        setUpActionsMenu()
        updateProgress()
        
        issuesView.reloadData()
        
        importer.start()
        
        NotificationCenter.default.post(name: .OESidebarSelectionDidChange, object: self)
        
        if importer.numberOfProcessedItems == importer.totalNumberOfItems {
            hideGameScannerView(animated: true)
        }
        
        dismiss(self)
    }
    
    @IBAction func buttonAction(_ sender: Any?) {
        
        if NSEvent.modifierFlags.contains(.option) || importer.status == .stopped && !itemsRequiringAttention.isEmpty {
            
            importer.pause()
            
            let cancelAlert = OEAlert()
            cancelAlert.messageText = NSLocalizedString("Do you really want to cancel importation?", comment: "")
            cancelAlert.informativeText = NSLocalizedString("This will remove all items from the queue. Items that finished importing will be preserved in your library.", comment: "")
            cancelAlert.defaultButtonTitle = NSLocalizedString("Stop", comment: "")
            cancelAlert.alternateButtonTitle = NSLocalizedString("Resume", comment: "")
            
            let button = sender as! NSButton
            button.state = button.state == .on ? .off : .on
            
            if cancelAlert.runModal() == .alertFirstButtonReturn {
                
                importer.cancel()
                itemsRequiringAttention.removeAll()
                updateProgress()
                
                hideGameScannerView(animated: true)
                
                button.state = .off
                
                dismiss(self)
                
            } else {
                importer.start()
            }
            
        } else {
            importer.togglePause()
        }
        
        updateProgress()
    }
}

// MARK: - Notifications

extension GameScannerViewController {
    
    @objc func gameInfoHelperWillUpdate(_ notification: Notification) {
        DispatchQueue.main.async {
            self.updateProgress()
            self.showGameScannerView(animated: true)
        }
    }
    
    @objc func gameInfoHelperDidChangeUpdateProgress(_ notification: Notification) {
        updateProgress()
    }
    
    @objc func gameInfoHelperDidUpdate(_ notification: Notification) {
        
        updateProgress()
        
        DispatchQueue.main.asyncAfter(deadline: .now() + .milliseconds(500)) {
            if self.importer.totalNumberOfItems == self.importer.numberOfProcessedItems {
                self.hideGameScannerView(animated: true)
            }
        }
    }
}

// MARK: - NSTableViewDataSource

extension GameScannerViewController: NSTableViewDataSource {
    
    func numberOfRows(in tableView: NSTableView) -> Int {
        return itemsRequiringAttention.count
    }
    
    func tableView(_ tableView: NSTableView, objectValueFor tableColumn: NSTableColumn?, row: Int) -> Any? {
        
        let item = itemsRequiringAttention[row]

        switch tableColumn!.identifier.rawValue {
            
        case "path":
            return item.url.lastPathComponent
            
        case "checked":
            return item.isChecked
            
        default:
            return nil
        }
    }
    
    func tableView(_ tableView: NSTableView, setObjectValue object: Any?, for tableColumn: NSTableColumn?, row: Int) {
        
        if tableColumn!.identifier.rawValue == "checked" {
            let item = itemsRequiringAttention[row]
            item.isChecked = (object as? Bool) ?? false
            
        }
        
        enableOrDisableApplyButton()
    }
}

// MARK: - NSTableViewDelegate

extension GameScannerViewController: NSTableViewDelegate {
    
    func tableView(_ tableView: NSTableView, shouldSelectRow row: Int) -> Bool {
        return false
    }
    
    func tableView(_ tableView: NSTableView, shouldTrackCell cell: NSCell, for tableColumn: NSTableColumn?, row: Int) -> Bool {
        return true
    }
    
    func tableView(_ tableView: NSTableView, toolTipFor cell: NSCell, rect: NSRectPointer, tableColumn: NSTableColumn?, row: Int, mouseLocation: NSPoint) -> String {
        
        guard row < itemsRequiringAttention.count else { return "" }

        if tableColumn!.identifier.rawValue == "path" {
            let item = itemsRequiringAttention[row]
            return item.sourceURL.path
        } else {
            return ""
        }
    }
}

// MARK: - OEROMImporterDelegate

extension GameScannerViewController: ROMImporterDelegate {
    
    func romImporterDidStart(_ importer: ROMImporter) {
        
        DLog("")
        
        isScanningDirectory = false
        
        DispatchQueue.main.asyncAfter(deadline: .now() + .seconds(1)) {
            if self.importer.totalNumberOfItems != self.importer.numberOfProcessedItems {
                self.updateProgress()
                self.showGameScannerView(animated: true)
            }
        }
    }
    
    func romImporterDidCancel(_ importer: ROMImporter) {
        DLog("")
        updateProgress()
    }
    
    func romImporterDidPause(_ importer: ROMImporter) {
        DLog("")
        updateProgress()
    }
    
    func romImporterDidFinish(_ importer: ROMImporter) {
        
        DLog("")
        
        DispatchQueue.main.asyncAfter(deadline: .now() + .milliseconds(500)) {
            if self.importer.totalNumberOfItems == self.importer.numberOfProcessedItems && !OpenVGDB.shared.isUpdating {
                self.hideGameScannerView(animated: true)
            }
        }

        // Show items that failed during import operation
        if !itemsFailedImport.isEmpty {
            showFailedImportItems()
            itemsFailedImport.removeAll()
        }
    }
    
    func romImporterChangedItemCount(_ importer: ROMImporter) {
        DispatchQueue.main.asyncAfter(deadline: .now() + .milliseconds(200)) {
            self.updateProgress()
        }
    }
    
    func romImporter(_ importer: ROMImporter, stoppedProcessingItem item: ImportOperation) {

        if let error = item.error {
            
            if (error as NSError).domain == OEImportErrorDomainResolvable && (error as NSError).code == OEImportErrorCode.multipleSystems.rawValue {
                
                itemsRequiringAttention.append(item)
                issuesView.reloadData()
                setUpActionsMenu()
                showGameScannerView(animated: true)
                
            } else {
                os_log(.debug, log: .import, "Import error: %{public}@", error.localizedDescription)

                // Track item that failed import
                //if item.exitStatus == .errorFatal {
                if (error as NSError).domain == OEImportErrorDomainFatal || error is OEDiscDescriptorErrors || error is OECUESheetErrors || error is OEDreamcastGDIErrors {

                    itemsFailedImport.append(item)
                }
            }
        }
        
        updateProgress()
    }

    private func showFailedImportItems() {
        var itemsAlreadyInDatabase: String?
        var itemsAlreadyInDatabaseFileUnreachable: String?
        var itemsNoSystem: String?
        var itemsDisallowArchivedFile: String?
        var itemsEmptyFile: String?
        var itemsDiscDescriptorUnreadableFile: String?
        var itemsDiscDescriptorMissingFiles: String?
        var itemsDiscDescriptorNotPlainTextFile: String?
        var itemsDiscDescriptorNoPermissionReadFile: String?
        var itemsCueSheetInvalidFileFormat: String?
        var itemsDreamcastGDIInvalidFileFormat: String?

        // Build messages for failed items
        for failedItem in itemsFailedImport {
            let error = failedItem.error! as NSError
            let failedFilename = failedItem.url.lastPathComponent

            if error.domain == OEImportErrorDomainFatal && error.code == OEImportErrorCode.alreadyInDatabase.rawValue {
                if itemsAlreadyInDatabase == nil {
                    itemsAlreadyInDatabase = NSLocalizedString("Already in library:", comment:"")
                }
                itemsAlreadyInDatabase! += "\n• " + String(format: NSLocalizedString("\"%@\" in %@", comment:"Import error description: file already imported (first item: filename, second item: system library name)"), failedFilename, failedItem.romLocation!)

            } else if error.domain == OEImportErrorDomainFatal && error.code == OEImportErrorCode.alreadyInDatabaseFileUnreachable.rawValue {
                if itemsAlreadyInDatabaseFileUnreachable == nil {
                    itemsAlreadyInDatabaseFileUnreachable = NSLocalizedString("Already in library, but manually deleted or unreachable:", comment:"")
                }
                itemsAlreadyInDatabaseFileUnreachable! += "\n• " + String(format: NSLocalizedString("\"%@\" in %@", comment:"Import error description: file already imported (first item: filename, second item: system library name)"), failedFilename, failedItem.romLocation!)

            } else if error.domain == OEImportErrorDomainFatal && error.code == OEImportErrorCode.noSystem.rawValue {
                if itemsNoSystem == nil {
                    itemsNoSystem = NSLocalizedString("No valid system detected:", comment:"")
                }
                itemsNoSystem! += "\n• \"\(failedFilename)\""

            } else if error.domain == OEImportErrorDomainFatal && error.code == OEImportErrorCode.disallowArchivedFile.rawValue {
                if itemsDisallowArchivedFile == nil {
                    itemsDisallowArchivedFile = NSLocalizedString("Must not be compressed:", comment:"")
                }
                itemsDisallowArchivedFile! += "\n• \"\(failedFilename)\""

            } else if error.domain == OEImportErrorDomainFatal && error.code == OEImportErrorCode.emptyFile.rawValue {
                if itemsEmptyFile == nil {
                    itemsEmptyFile = NSLocalizedString("Contains no data:", comment:"")
                }
                itemsEmptyFile! += "\n• \"\(failedFilename)\""

            } else if let error2 = error as? OEDiscDescriptorErrors {
                switch error2.code {
                case .unreadableFileError:
                    if itemsDiscDescriptorUnreadableFile == nil {
                        itemsDiscDescriptorUnreadableFile = NSLocalizedString("Unexpected error:", comment:"")
                    }
                    let underlyingError = error2.userInfo[NSUnderlyingErrorKey] as! NSError
                    itemsDiscDescriptorUnreadableFile! += "\n• \"\(failedFilename)\""
                    itemsDiscDescriptorUnreadableFile! += "\n\n\(underlyingError)"
                    
                case .missingFilesError:
                    if let underlyingError = error2.userInfo[NSUnderlyingErrorKey] as? CocoaError, underlyingError.code == .fileReadNoSuchFile {
                        if itemsDiscDescriptorMissingFiles == nil {
                            itemsDiscDescriptorMissingFiles = NSLocalizedString("Missing referenced file:", comment:"")
                        }
                        let missingFilename = (underlyingError.userInfo[NSFilePathErrorKey] as! NSString).lastPathComponent
                        let notFoundMsg = String(format: NSLocalizedString("NOT FOUND: \"%@\"", comment:"Import error description: referenced file not found"), missingFilename)
                        itemsDiscDescriptorMissingFiles! += "\n• \"\(failedFilename)\"\n   - \(notFoundMsg)"
                    }

                case .notPlainTextFileError:
                    if itemsDiscDescriptorNotPlainTextFile == nil {
                        itemsDiscDescriptorNotPlainTextFile = NSLocalizedString("Not plain text:", comment:"")
                    }
                    itemsDiscDescriptorNotPlainTextFile! += "\n• \"\(failedFilename)\""
                    
                case .noPermissionReadFileError:
                    if itemsDiscDescriptorNoPermissionReadFile == nil {
                        itemsDiscDescriptorNoPermissionReadFile = NSLocalizedString("No permission to open:", comment:"")
                    }
                    itemsDiscDescriptorNoPermissionReadFile! += "\n• \"\(failedFilename)\""

                @unknown default:
                    continue
                }

            } else if error is OECUESheetErrors {
                if itemsCueSheetInvalidFileFormat == nil {
                    itemsCueSheetInvalidFileFormat = NSLocalizedString("Invalid cue sheet format:", comment:"")
                }
                itemsCueSheetInvalidFileFormat! += "\n• \"\(failedFilename)\""

            } else if error is OEDreamcastGDIErrors {
                if itemsDreamcastGDIInvalidFileFormat == nil {
                    itemsDreamcastGDIInvalidFileFormat = NSLocalizedString("Invalid gdi format:", comment:"")
                }
                itemsDreamcastGDIInvalidFileFormat! += "\n• \"\(failedFilename)\""

            }
        }
        
        // Add instructions to fix permission errors
        if itemsDiscDescriptorNoPermissionReadFile != nil {
            itemsDiscDescriptorNoPermissionReadFile! += "\n\n"+NSLocalizedString("Choose Apple menu  > System Preferences, click Security & Privacy then select the Privacy tab. Remove the existing Files and Folders permission for OpenEmu, if exists, and instead grant Full Disk Access.", comment: "")
        }

        // Concatenate messages
        let failedMessage = [itemsAlreadyInDatabase, itemsAlreadyInDatabaseFileUnreachable, itemsNoSystem, itemsDisallowArchivedFile, itemsEmptyFile, itemsDiscDescriptorUnreadableFile, itemsDiscDescriptorMissingFiles, itemsDiscDescriptorNotPlainTextFile, itemsDiscDescriptorNoPermissionReadFile, itemsCueSheetInvalidFileFormat, itemsDreamcastGDIInvalidFileFormat].compactMap{$0}.joined(separator:"\n\n")

        let alert = OEAlert()
        alert.messageText = NSLocalizedString("Files failed to import.", comment: "")
        alert.informativeText = failedMessage
        alert.defaultButtonTitle = NSLocalizedString("View Guide in Browser", comment:"")
        alert.alternateButtonTitle = NSLocalizedString("Dismiss", comment:"")
        
        if let window = NSApp.windows.first(where: { $0.windowController is MainWindowController }) {
            alert.beginSheetModal(for: window) { result in
                if result == .alertFirstButtonReturn {
                    NSWorkspace.shared.open(GameScannerViewController.importGuideURL)
                }
            }
        }
    }
}
