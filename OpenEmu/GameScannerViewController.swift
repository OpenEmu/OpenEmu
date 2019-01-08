// Copyright (c) 2018, OpenEmu Team
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
    
    @IBOutlet weak var libraryController: OELibraryController!
    
    @IBOutlet weak var scannerView: NSView!
    
    @IBOutlet var headlineLabel: NSTextField!
    @IBOutlet var togglePauseButton: NSButton!
    @IBOutlet var progressIndicator: NSProgressIndicator!
    @IBOutlet var statusLabel: NSTextField!
    @IBOutlet var fixButton: OEButton!
    
    @IBOutlet var issuesView: NSTableView!
    @IBOutlet var actionPopUpButton: NSPopUpButton!
    @IBOutlet var applyButton: OEButton!
   
    @IBOutlet private weak var bottomBar: NSView!
    @IBOutlet private weak var sourceListScrollView: NSScrollView!
    @IBOutlet private weak var libraryGamesViewController: OELibraryGamesViewController!
    
    private var itemsRequiringAttention = [OEImportOperation]()
    private var isScanningDirectory = false
    private var isGameScannerVisible = true // The game scanner view is already visible in OELibraryGamesViewController.xib.
    
    private var importer: OEROMImporter { return OELibraryDatabase.default!.importer }
    
    required init?(coder: NSCoder) {
        
        super.init(coder: coder)
        
        let notificationCenter = NotificationCenter.default
        notificationCenter.addObserver(self, selector: #selector(gameInfoHelperWillUpdate(_:)), name: NSNotification.Name(rawValue: OEGameInfoHelperWillUpdateNotificationName), object: nil)
        notificationCenter.addObserver(self, selector: #selector(gameInfoHelperDidChangeUpdateProgress(_:)), name: NSNotification.Name(rawValue: OEGameInfoHelperDidChangeUpdateProgressNotificationName), object: nil)
        notificationCenter.addObserver(self, selector: #selector(gameInfoHelperDidUpdate(_:)), name: NSNotification.Name(rawValue: OEGameInfoHelperDidUpdateNotificationName), object: nil)
    }
    
    override var nibName: NSNib.Name? {
        return NSNib.Name(rawValue: "OEGameScanner")
    }
    
    override func awakeFromNib() {
        
        super.awakeFromNib()
        
        _ = view // load other xib as well
        
        guard let scannerView = scannerView, let issuesView = issuesView else { return }
        
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
        
        NotificationCenter.default.addObserver(self, selector: #selector(gameScannerViewFrameChanged(_:)), name: NSView.frameDidChangeNotification, object: scannerView)
        
        createFixButton()
        
        // Show game scanner if importer is running already or game info is downloading.
        if importer.status == .running || OEGameInfoHelper.shared.isUpdating {
            updateProgress()
            showGameScannerView(animated: true)
        }
    }
    
    override func viewWillAppear() {
        
        super.viewWillAppear()
        
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
    
    private func layOutSidebarViews(withVisibleGameScanner visibleGameScanner: Bool, animated: Bool) {
        
        var gameScannerFrame = scannerView.frame
        gameScannerFrame.origin.y = visibleGameScanner ? 0 : -gameScannerFrame.height
        
        var bottomBarFrame = bottomBar.frame
        bottomBarFrame.origin.y = gameScannerFrame.maxY
        
        var sourceListFrame = sourceListScrollView.frame
        sourceListFrame.origin.y = bottomBarFrame.maxY
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
    
    private func createFixButton() {
        
        fixButton = OEButton(frame: NSRect(x: 14,
                                           y: 4,
                                           width: scannerView.frame.width - 20,
                                           height: 20))
        
        fixButton.autoresizingMask = .width
        fixButton.alignment = .left
        fixButton.imagePosition = .imageRight
        fixButton.setThemeKey("game_scanner_fix_issues")
        fixButton.target = libraryGamesViewController
        fixButton.action = #selector(OELibraryGamesViewController.showIssuesView(_:))
        fixButton.title = NSLocalizedString("Resolve Issues", comment: "")
        fixButton.isHidden = true
        
        fixButton.sizeToFit()
        
        scannerView.addSubview(fixButton)
    }
    
    @objc private func updateProgress() {
        
        NSObject.cancelPreviousPerformRequests(withTarget: self, selector: #selector(updateProgress), object: nil)
        
        CATransaction.begin()
        defer { CATransaction.commit() }
        
        let infoHelper = OEGameInfoHelper.shared
        if infoHelper.isUpdating {
            
            progressIndicator.minValue = 0
            progressIndicator.doubleValue = Double(infoHelper.downloadProgress)
            progressIndicator.maxValue = 1
            progressIndicator.isIndeterminate = false
            progressIndicator.startAnimation(self)
            
            fixButton.isHidden = true
            statusLabel.stringValue = NSLocalizedString("Downloading Game DB", comment: "")
            
        } else {
            
            let maxItems = importer.totalNumberOfItems
            
            progressIndicator.minValue = 0
            progressIndicator.doubleValue = Double(importer.numberOfProcessedItems)
            progressIndicator.maxValue = Double(maxItems)
            
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
                
            case .stopped:
                
                progressIndicator.stopAnimation(self)
                progressIndicator.isIndeterminate = true
                
                status = NSLocalizedString("Done", comment: "")
                
            default:
                
                progressIndicator.stopAnimation(self)
                progressIndicator.isIndeterminate = true
                
                status = NSLocalizedString("Scanner Paused", comment: "")
            }
            
            let shouldHideFixButton: Bool
            
            if !itemsRequiringAttention.isEmpty {
                
                fixButton.title = itemsRequiringAttention.count > 1 ?
                    String(format: NSLocalizedString("Resolve %ld Issues", comment: ""), itemsRequiringAttention.count) :
                    String(format: NSLocalizedString("Resolve %ld Issue", comment: ""), itemsRequiringAttention.count)
                
                fixButton.sizeToFit()
                
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
    }
    
    @objc func deselectAll(_ sender: Any?) {
        for item in itemsRequiringAttention {
            item.isChecked = false
        }
        issuesView.reloadData()
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
        
        NotificationCenter.default.post(name: Notification.Name(rawValue: OESidebarSelectionDidChangeNotificationName), object: self)
        
        if importer.numberOfProcessedItems == importer.totalNumberOfItems {
            hideGameScannerView(animated: true)
        }
        
        dismissViewController(self)
    }
    
    @IBAction func buttonAction(_ sender: Any?) {
        
        if NSEvent.modifierFlags.contains(.option) {
            
            importer.pause()
            
            let cancelAlert = OEHUDAlert()
            cancelAlert.headlineText = NSLocalizedString("Do you really want to cancel the import process?", comment: "")
            cancelAlert.messageText = NSLocalizedString("Choose Yes to remove all items from the queue. Items that finished importing will be preserved in your library.", comment: "")
            cancelAlert.defaultButtonTitle = NSLocalizedString("Yes", comment: "")
            cancelAlert.alternateButtonTitle = NSLocalizedString("No", comment: "")
            
            let button = sender as! NSButton
            button.state = button.state == .on ? .off : .on
            
            if cancelAlert.runModal() == .alertFirstButtonReturn {
                
                importer.cancel()
                itemsRequiringAttention.removeAll()
                updateProgress()
                
                hideGameScannerView(animated: true)
                
                button.state = .off
                
                dismissViewController(self)
                
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
    
    @objc func gameScannerViewFrameChanged(_ notification: Notification) {
        
        let bounds = scannerView.bounds
        let width = bounds.width
        
        var frame = progressIndicator.frame
        frame.origin.x = 16
        frame.size.width = width - 16 - 38
        progressIndicator.frame = frame
        
        frame = headlineLabel.frame
        frame.origin.x = 17
        frame.size.width = width - 17 - 12
        headlineLabel.frame = frame
        
        frame = statusLabel.frame
        frame.origin.x = 17
        frame.size.width = width - 17 - 12
        statusLabel.frame = frame
        
        frame = fixButton.frame
        frame.origin.x = 14
        fixButton.frame = frame
        fixButton.sizeToFit()
    }
}

// MARK: - OESidebarItem

extension GameScannerViewController: OESidebarItem {
    
    func sidebarIcon() -> NSImage! {
        return nil
    }
    
    func sidebarName() -> String! {
        return NSLocalizedString("Game Scanner", comment: "")
    }
    
    func sidebarID() -> String! {
        return "Game Scanner"
    }
    
    func viewControllerClassName() -> String! {
        return className
    }
    
    func setSidebarName(_ newName: String!) {}
    
    func isSelectableInSidebar() -> Bool {
        return true
    }
    
    func isEditableInSidebar() -> Bool {
        return false
    }
    
    func isGroupHeaderInSidebar() -> Bool {
        return false
    }
    
    func hasSubCollections() -> Bool {
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

// MARK: - NSWindowDelegate

extension GameScannerViewController: NSWindowDelegate {
    
    func windowWillResize(_ sender: NSWindow, to frameSize: NSSize) -> NSSize {
        return sender.frame.size
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
                NSLog("\(error)")
            }
        }
        
        updateProgress()
    }
}
