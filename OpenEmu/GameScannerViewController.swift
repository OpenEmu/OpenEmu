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

@objc(OEGameScannerViewController)
class GameScannerViewController: NSViewController {

    @objc public static let OESidebarHideBottomBarKey = "OESidebarHideBottomBar"
    public static let OESidebarBottomBarDidChange = Notification.Name("OESidebarBottomBarDidChangeNotification")
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
    
    @IBOutlet private weak var bottomBar: NSView!
    @IBOutlet private weak var sourceListScrollView: NSScrollView!
    
    private var itemsRequiringAttention = [OEImportOperation]()
    private var itemsFailedImport = [OEImportOperation]()
    private var isScanningDirectory = false
    private var isGameScannerVisible = true // The game scanner view is already visible in OELibraryGamesViewController.xib.
    
    private var importer: OEROMImporter { return OELibraryDatabase.default!.importer }
    
    required init?(coder: NSCoder) {
        
        super.init(coder: coder)
        
        let notificationCenter = NotificationCenter.default
        notificationCenter.addObserver(self, selector: #selector(gameInfoHelperWillUpdate(_:)), name: .OEGameInfoHelperWillUpdate, object: nil)
        notificationCenter.addObserver(self, selector: #selector(gameInfoHelperDidChangeUpdateProgress(_:)), name: .OEGameInfoHelperDidChangeUpdateProgress, object: nil)
        notificationCenter.addObserver(self, selector: #selector(gameInfoHelperDidUpdate(_:)), name: .OEGameInfoHelperDidUpdate, object: nil)
        notificationCenter.addObserver(self, selector: #selector(bottomBarDidChange), name: GameScannerViewController.OESidebarBottomBarDidChange, object: nil)
    }
    
    override var nibName: NSNib.Name? {
        return "OEGameScanner"
    }
    
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
        
        layOutSidebarViews(withVisibleGameScanner: false, animated: false)
        
        itemsRequiringAttention = []
        
        importer.delegate = self
        
        // Show game scanner if importer is running already or game info is downloading.
        if importer.status == .running || OEGameInfoHelper.shared.isUpdating {
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
        let menuItems: [NSMenuItem] = systemIDs.map { systemID in
            
            let system = OEDBSystem(forPluginIdentifier: systemID, in: context)
            let menuItem = NSMenuItem(title: system.name, action: nil, keyEquivalent: "")
            menuItem.image = system.icon
            menuItem.representedObject = systemID
            
            return menuItem
            
        }.sorted { $0.title.localizedStandardCompare($1.title) == .orderedAscending }
        
        menu.addItem(withTitle: NSLocalizedString("Don't Import Selected", comment: ""), action: nil, keyEquivalent: "")
        
        menu.addItem(NSMenuItem.separator())
        
        for item in menuItems {
            menu.addItem(item)
        }
        
        actionPopUpButton.menu = menu
    }
    
    @objc func bottomBarDidChange() {
        layOutSidebarViews(withVisibleGameScanner: isGameScannerVisible, animated: false)
    }
    
    private func layOutSidebarViews(withVisibleGameScanner visibleGameScanner: Bool, animated: Bool) {
        
        if UserDefaults.standard.bool(forKey: GameScannerViewController.OESidebarHideBottomBarKey) {
            bottomBar.isHidden = true
        }
        else {
            bottomBar.isHidden = false
        }
        
        var gameScannerFrame = scannerView.frame
        gameScannerFrame.origin.y = visibleGameScanner ? 0 : -gameScannerFrame.height
        
        var bottomBarFrame = bottomBar.frame
        bottomBarFrame.origin.y = gameScannerFrame.maxY
        
        var sourceListFrame = sourceListScrollView.frame
        sourceListFrame.origin.y = bottomBar.isHidden ? gameScannerFrame.maxY : bottomBarFrame.maxY
        sourceListFrame.size.height = sourceListScrollView.superview!.frame.height - sourceListFrame.minY
        
        if animated {
            
            NSAnimationContext.beginGrouping()
            
            // Set frames through animator proxies to implicitly animate changes.
            scannerView.animator().frame = gameScannerFrame
            bottomBar.animator().frame = bottomBarFrame
            sourceListScrollView.animator().frame = sourceListFrame
            
            NSAnimationContext.endGrouping()
            
        } else {
            
            // Set frames directly without implicit animations.
            scannerView.frame = gameScannerFrame
            bottomBar.frame = bottomBarFrame
            sourceListScrollView.frame = sourceListFrame
        }
        
        isGameScannerVisible = visibleGameScanner
    }
    
    @objc private func updateProgress() {
        
        NSObject.cancelPreviousPerformRequests(withTarget: self, selector: #selector(updateProgress), object: nil)
        
        CATransaction.begin()
        defer { CATransaction.commit() }
        
        headlineLabel.stringValue = NSLocalizedString("Game Scanner", comment: "")
        
        let infoHelper = OEGameInfoHelper.shared
        if infoHelper.isUpdating {
            
            progressIndicator.minValue = 0
            progressIndicator.doubleValue = Double(infoHelper.downloadProgress)
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
                togglePauseButton.icon = "game_scanner_pause"
                
            case .stopped:
                
                progressIndicator.stopAnimation(self)
                progressIndicator.isIndeterminate = true
                
                status = NSLocalizedString("Done", comment: "")
                
                togglePauseButton.isEnabled = !itemsRequiringAttention.isEmpty
                togglePauseButton.icon = itemsRequiringAttention.isEmpty ? "game_scanner_pause" : "game_scanner_cancel"
                
            default:
                
                progressIndicator.stopAnimation(self)
                progressIndicator.isIndeterminate = true
                
                status = NSLocalizedString("Scanner Paused", comment: "")
                
                togglePauseButton.isEnabled = true
                togglePauseButton.icon = "game_scanner_continue"
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
    
    @objc(showGameScannerViewAnimated:)
    func showGameScannerView(animated: Bool) {
        layOutSidebarViews(withVisibleGameScanner: true, animated: animated)
    }
    
    @objc(hideGameScannerViewAnimated:)
    func hideGameScannerView(animated: Bool) {
        guard itemsRequiringAttention.isEmpty else { return }
        layOutSidebarViews(withVisibleGameScanner: false, animated: animated)
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
        
        if !itemsRequiringAttention.isEmpty {
            
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

// MARK: - OESidebarItem

extension GameScannerViewController: OESidebarItem {
    
    var sidebarIcon: NSImage? {
        return nil
    }
    
    var sidebarName: String {
        return NSLocalizedString("Game Scanner", comment: "")
    }
    
    var sidebarID: String? {
        return "Game Scanner"
    }
    
    var viewControllerClassName: String? {
        return className
    }
    
    func setSidebarName(_ newName: String) {}
    
    var isSelectableInSidebar: Bool {
        return true
    }
    
    var isEditableInSidebar: Bool {
        return false
    }
    
    var isGroupHeaderInSidebar: Bool {
        return false
    }
    
    var hasSubCollections: Bool {
        return false
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

extension GameScannerViewController: OEROMImporterDelegate {
    
    func romImporterDidStart(_ importer: OEROMImporter) {
        
        DLog("")
        
        isScanningDirectory = false
        
        DispatchQueue.main.asyncAfter(deadline: .now() + .seconds(1)) {
            if self.importer.totalNumberOfItems != self.importer.numberOfProcessedItems {
                self.updateProgress()
                self.showGameScannerView(animated: true)
            }
        }
    }
    
    func romImporterDidCancel(_ importer: OEROMImporter) {
        DLog("")
        updateProgress()
    }
    
    func romImporterDidPause(_ importer: OEROMImporter) {
        DLog("")
        updateProgress()
    }
    
    func romImporterDidFinish(_ importer: OEROMImporter) {
        
        DLog("")
        
        DispatchQueue.main.asyncAfter(deadline: .now() + .milliseconds(500)) {
            if self.importer.totalNumberOfItems == self.importer.numberOfProcessedItems && !OEGameInfoHelper.shared.isUpdating {
                self.hideGameScannerView(animated: true)
            }
        }

        // Show items that failed during import operation
        if !itemsFailedImport.isEmpty {
            showFailedImportItems()
            itemsFailedImport.removeAll()
        }
    }
    
    func romImporterChangedItemCount(_ importer: OEROMImporter) {
        DispatchQueue.main.asyncAfter(deadline: .now() + .milliseconds(200)) {
            self.updateProgress()
        }
    }
    
    func romImporter(_ importer: OEROMImporter, stoppedProcessingItem item: OEImportOperation) {

        if let error = item.error {
            
            if (error as NSError).domain == OEImportErrorDomainResolvable && (error as NSError).code == OEImportErrorCode.multipleSystems.rawValue {
                
                itemsRequiringAttention.append(item)
                issuesView.reloadData()
                setUpActionsMenu()
                showGameScannerView(animated: true)
                
            } else {
                os_log(.debug, log: OE_LOG_IMPORT, "Import error: %{public}@", error.localizedDescription)

                // Track item that failed import
                //if item.exitStatus == OEImportExitStatus.errorFatal {
                if (error as NSError).domain == OEImportErrorDomainFatal || (error as NSError).domain == OEDiscDescriptorErrorDomain || (error as NSError).domain == OECUESheetErrorDomain || (error as NSError).domain == OEDreamcastGDIErrorDomain {

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
            let error = (failedItem.error! as NSError)
            let failedFilename = failedItem.url.lastPathComponent

            if error.domain == OEImportErrorDomainFatal && error.code == OEImportErrorCode.alreadyInDatabase.rawValue {
                if itemsAlreadyInDatabase == nil {
                    itemsAlreadyInDatabase = NSLocalizedString("Already in library:", comment:"")
                }
                itemsAlreadyInDatabase! += "\n• " + String.init(format: NSLocalizedString("\"%@\" in %@", comment:"Import error description: file already imported (first item: filename, second item: system library name)"), failedFilename, failedItem.romLocation!)

            } else if error.domain == OEImportErrorDomainFatal && error.code == OEImportErrorCode.alreadyInDatabaseFileUnreachable.rawValue {
                if itemsAlreadyInDatabaseFileUnreachable == nil {
                    itemsAlreadyInDatabaseFileUnreachable = NSLocalizedString("Already in library, but manually deleted or unreachable:", comment:"")
                }
                itemsAlreadyInDatabaseFileUnreachable! += "\n• " + String.init(format: NSLocalizedString("\"%@\" in %@", comment:"Import error description: file already imported (first item: filename, second item: system library name)"), failedFilename, failedItem.romLocation!)

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

            } else if error.domain == OEDiscDescriptorErrorDomain && error.code == OEDiscDescriptorUnreadableFileError {
                if itemsDiscDescriptorUnreadableFile == nil {
                    itemsDiscDescriptorUnreadableFile = NSLocalizedString("Unexpected error:", comment:"")
                }
                let underlyingError = error.userInfo[NSUnderlyingErrorKey] as! NSError
                itemsDiscDescriptorUnreadableFile! += "\n• \"\(failedFilename)\""
                itemsDiscDescriptorUnreadableFile! += "\n\n\(underlyingError)"

            } else if error.domain == OEDiscDescriptorErrorDomain && error.code == OEDiscDescriptorMissingFilesError {
                let underlyingError = error.userInfo[NSUnderlyingErrorKey] as! NSError
                if underlyingError.code == CocoaError.fileReadNoSuchFile.rawValue {
                    if itemsDiscDescriptorMissingFiles == nil {
                        itemsDiscDescriptorMissingFiles = NSLocalizedString("Missing referenced file:", comment:"")
                    }
                    let missingFilename = (underlyingError.userInfo[NSFilePathErrorKey] as! NSString).lastPathComponent
                    let notFoundMsg = String.init(format: NSLocalizedString("NOT FOUND: \"%@\"", comment:"Import error description: referenced file not found"), missingFilename)
                    itemsDiscDescriptorMissingFiles! += "\n• \"\(failedFilename)\"\n   - \(notFoundMsg)"
                }

            } else if error.domain == OEDiscDescriptorErrorDomain && error.code == OEDiscDescriptorNotPlainTextFileError {
                if itemsDiscDescriptorNotPlainTextFile == nil {
                    itemsDiscDescriptorNotPlainTextFile = NSLocalizedString("Not plain text:", comment:"")
                }
                itemsDiscDescriptorNotPlainTextFile! += "\n• \"\(failedFilename)\""

            } else if error.domain == OEDiscDescriptorErrorDomain && error.code == OEDiscDescriptorNoPermissionReadFileError {
                if itemsDiscDescriptorNoPermissionReadFile == nil {
                    itemsDiscDescriptorNoPermissionReadFile = NSLocalizedString("No permission to open:", comment:"")
                }
                itemsDiscDescriptorNoPermissionReadFile! += "\n• \"\(failedFilename)\""

            } else if error.domain == OECUESheetErrorDomain {
                if itemsCueSheetInvalidFileFormat == nil {
                    itemsCueSheetInvalidFileFormat = NSLocalizedString("Invalid cue sheet format:", comment:"")
                }
                itemsCueSheetInvalidFileFormat! += "\n• \"\(failedFilename)\""

            } else if error.domain == OEDreamcastGDIErrorDomain {
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

        // HACK: walk up hierarchy to find a controller with a window
        var current: NSViewController = self
        while let parent = current.parent {
            if let win = parent.view.window {
                alert.beginSheetModal(for: win) { result in
                    if result == .alertFirstButtonReturn {
                        NSWorkspace.shared.open(GameScannerViewController.importGuideURL)
                    }
                }
                return
            }
            current = parent
        }
    }
}
