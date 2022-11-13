// Copyright (c) 2019, OpenEmu Team
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

final class SaveStateViewController: ImageCollectionViewController, LibrarySubviewControllerSaveStateSelection {
    var supportsQuickLook: Bool { false }
    
    override var representedObject: Any? {
        willSet {
            precondition(newValue == nil || newValue is OEDBSaveStatesMedia, "unexpected object")
        }
    }
    
    var dataSource = ImagesDataSource<OEDBSaveState>(gameKeyPath: \.rom?.game,
                                                     titleKeyPath: \.displayName,
                                                     timestampKeyPath: \.timestamp,
                                                     imageURLKeyPath: \.screenshotURL,
                                                     sortDescriptors: [
                                                        NSSortDescriptor(keyPath: \OEDBSaveState.rom?.game?.name, ascending: true),
                                                        NSSortDescriptor(keyPath: \OEDBSaveState.timestamp, ascending: true)],
                                                     entityName: OEDBSaveState.entityName)
    
    var selectedSaveStates: [OEDBSaveState] {
        dataSource.items(at: collectionView.selectionIndexPaths)
    }
    
    override func viewDidLoad() {
        self.representedObject = OEDBSaveStatesMedia.shared
        self.dataSourceDelegate = dataSource
        
        super.viewDidLoad()
    }
    
    override func collectionView(_ collectionView: NSCollectionView, pasteboardWriterForItemAt indexPath: IndexPath) -> NSPasteboardWriting? {
        return dataSource.item(at: indexPath)?.url.absoluteURL as NSPasteboardWriting?
    }
}

extension SaveStateViewController: CollectionViewExtendedDelegate, NSMenuItemValidation {
    func collectionView(_ collectionView: CollectionView, setTitle title: String, forItemAt indexPath: IndexPath) {
        guard let item = dataSource.item(at: indexPath), !title.isEmpty, title != item.displayName  else { return }
        
        if title.hasPrefix(OEDBSaveState.specialNamePrefix) {
            return
        }
        
        if !item.isSpecialState || OEAlert.renameSpecialSaveState().runModal() == .alertFirstButtonReturn {
            item.name = title
            item.moveToDefaultLocation()
            if !item.writeToDisk() {
                NSLog("Writing save state '%@' failed. It should be deleted!", title)
            }
            item.save()
        }
        
        if let itemView = self.collectionView.item(at: indexPath) as? ImageCollectionViewItem {
            dataSource.loadItemView(itemView, at: indexPath)
        }
    }
    
    func validateMenuItem(_ menuItem: NSMenuItem) -> Bool {
        switch menuItem.action {
        case #selector(copy(_:)),
             #selector(delete(_:)),
             #selector(showInFinder(_:)):
            return collectionView.selectionIndexPaths.count > 0
        case #selector(startSelectedGame(_:)):
            return collectionView.selectionIndexPaths.count == 1
        default:
            return true
        }
    }
    
    func collectionView(_ collectionView: CollectionView, menuForItemsAt indexPaths: Set<IndexPath>, point: CGPoint) -> NSMenu? {
        let menu = NSMenu()
        
        if indexPaths.count == 1 {
            menu.addItem(withTitle: NSLocalizedString("Play Save State", comment: "SaveState View Context menu"),
                         action: #selector(LibraryController.startSaveState(_:)),
                         keyEquivalent: "")
            
            menu.addItem(withTitle: NSLocalizedString("Rename", comment: "SaveState View Context menu"),
                         action: #selector(CollectionView.beginEditingWithSelectedItem(_:)),
                         keyEquivalent: "")
            
            menu.addItem(withTitle: NSLocalizedString("Show in Finder", comment: "SaveState View Context menu"),
                         action: #selector(showInFinder(_:)),
                         keyEquivalent: "")
            
            menu.addItem(withTitle: NSLocalizedString("Delete Save State", comment: "SaveState View Context menu"),
                         action: #selector(deleteSelectedItems(_:)),
                         keyEquivalent: "")
        } else {
            menu.addItem(withTitle: NSLocalizedString("Show in Finder", comment: "SaveState View Context menu"),
                         action: #selector(showInFinder(_:)),
                         keyEquivalent: "")
            
            menu.addItem(withTitle: NSLocalizedString("Delete Save States", comment: "SaveState View Context menu (plural)"),
                         action: #selector(deleteSelectedItems(_:)),
                         keyEquivalent: "")
        }
        
        return menu
    }
    
    @IBAction func copy(_ sender: Any?) {
        copySelectedItems(sender)
    }
    
    @IBAction func copySelectedItems(_ sender: Any?) {
        let items = dataSource.items(at: collectionView.selectionIndexPaths)
        guard !items.isEmpty else { return }
        
        let fileURLs = items.map { $0.url.absoluteURL as NSURL }
        
        let pboard = NSPasteboard.general
        pboard.clearContents()
        pboard.declareTypes([.fileURL], owner: nil)
        pboard.writeObjects(fileURLs)
    }
    
    @IBAction override func delete(_ sender: Any?) {
        deleteSelectedItems(sender)
    }
    
    @IBAction func deleteSelectedItems(_ sender: Any?) {
        let items = dataSource.items(at: collectionView.selectionIndexPaths)
        guard !items.isEmpty else { return }
        
        var alert: OEAlert
        if items.count == 1 {
            alert = .deleteSaveState(name: items.first!.name)
        } else {
            alert = .deleteSaveStates(count: items.count)
        }
        
        if alert.runModal() == .alertFirstButtonReturn {
            items.forEach { $0.deleteAndRemoveFiles() }
            try? OELibraryDatabase.default?.mainThreadContext.save()
            collectionView.selectionIndexPaths = []
            return
        }
        
        reloadData()
    }
    
    @IBAction func showInFinder(_ sender: Any?) {
        let items = dataSource.items(at: collectionView.selectionIndexPaths)
        let urls = items.compactMap { $0.url.absoluteURL }
        
        NSWorkspace.shared.activateFileViewerSelecting(urls)
    }
    
    @IBAction func startSelectedGame(_ sender: Any?) {
        NSApp.sendAction(#selector(LibraryController.startSaveState(_:)), to: nil, from: self)
    }
    
    func collectionView(_ collectionView: CollectionView, doubleClickForItemAt indexPath: IndexPath) {
        guard let item = dataSource.item(at: indexPath) else { return }
        NSApp.sendAction(#selector(LibraryController.startSaveState(_:)), to: nil, from: item)
    }
}

// MARK: - Touch Bar

private extension NSTouchBarItem.Identifier {
    static let resumeSaveState = NSTouchBarItem.Identifier("org.openemu.SaveStateViewController.saveStatesTouchbar.resume")
}

extension SaveStateViewController {
    
    override func makeTouchBar() -> NSTouchBar? {
        
        let touchBar = NSTouchBar()
        touchBar.delegate = self
        touchBar.defaultItemIdentifiers = [.resumeSaveState,
                                           .otherItemsProxy]
        return touchBar
    }
    
    override func selectionDidChange() {
        guard let touchBar = self.touchBar else { return }
        let selected = self.collectionView.selectionIndexPaths
        
        if let playItem = touchBar.item(forIdentifier: .resumeSaveState) as? NSCustomTouchBarItem {
            let button = playItem.view as! NSButton
            button.isEnabled = selected.count == 1
        }
    }
}

extension SaveStateViewController: NSTouchBarDelegate {
    
    func touchBar(_ touchBar: NSTouchBar, makeItemForIdentifier identifier: NSTouchBarItem.Identifier) -> NSTouchBarItem? {
        
        switch identifier {
        
        case .resumeSaveState:
            
            let item = NSCustomTouchBarItem(identifier: identifier)
            item.customizationLabel = NSLocalizedString("Resume Game", comment: "")
            
            let button = NSButton(image: NSImage(named: NSImage.touchBarPlayTemplateName)!, target: nil, action: #selector(LibraryController.startSaveState(_:)))
            
            button.isEnabled = collectionView.selectionIndexPaths.count == 1
            
            item.view = button
            
            return item
            
        default:
            return nil
        }
    }
}

