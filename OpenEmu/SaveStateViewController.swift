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

@objc
class SaveStateViewController: ImageCollectionViewController {
    var supportsQuickLook: Bool { false }
    
    override var representedObject: Any? {
        willSet {
            precondition(newValue == nil || newValue is OEDBSavedGamesMedia, "unexpected object")
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
        self.representedObject = OEDBSavedGamesMedia.shared
        self.dataSourceDelegate = dataSource
        
        super.viewDidLoad()
    }
}

extension SaveStateViewController: CollectionViewExtendedDelegate, NSMenuItemValidation {
    func collectionView(_ collectionView: CollectionView, setTitle title: String, forItemAt indexPath: IndexPath) {
        guard let item = dataSource.item(at: indexPath), !title.isEmpty else { return }
        
        if title.hasPrefix("OESpecialState_") {
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
    
    @objc func validateMenuItem(_ menuItem: NSMenuItem) -> Bool {
        if let sel = menuItem.action, sel == #selector(showInFinder(_:)) {
            return collectionView.selectionIndexPaths.count > 0
        }
        
        return true
    }
    
    func collectionView(_ collectionView: CollectionView, menuForItemsAt indexPaths: Set<IndexPath>) -> NSMenu? {
        let menu = NSMenu()
        menu.autoenablesItems = true
        
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
        
    @IBAction func deleteSelectedItems(_ sender: Any?) {
        let items = dataSource.items(at: collectionView.selectionIndexPaths)
        if items.count == 0 {
            return
        }
        
        var alert: OEAlert
        if items.count == 1 {
            alert = .deleteSaveState(name: items.first!.name!)
        } else {
            alert = .deleteSaveStates(count: items.count)
        }
        
        if alert.runModal() == .alertFirstButtonReturn {
            items.forEach { $0.delete() }
            try? OELibraryDatabase.default?.mainThreadContext.save()
            return
        }
        
        reloadData()
    }
    
    func collectionView(_ collectionView: CollectionView, doubleClickForItemAt indexPath: IndexPath) {
        guard let item = dataSource.item(at: indexPath) else { return }
        NSApp.sendAction(#selector(LibraryController.startSaveState(_:)), to: nil, from: item)
    }
}

// MARK: - Touch Bar

fileprivate extension NSTouchBar.CustomizationIdentifier {
    static let saveStatesTouchBar = "org.openemu.SaveStateViewController.saveStatesTouchBar"
}

fileprivate extension NSTouchBarItem.Identifier {
    static let deleteSaveState = NSTouchBarItem.Identifier("org.openemu.SaveStateViewController.saveStatesTouchbar.delete")
    static let resumeSaveState = NSTouchBarItem.Identifier("org.openemu.SaveStateViewController.saveStatesTouchbar.resume")
}

extension SaveStateViewController {
    
    open override func makeTouchBar() -> NSTouchBar? {
        
        let touchBar = NSTouchBar()
        
        touchBar.delegate = self
        
        touchBar.customizationIdentifier = .saveStatesTouchBar
        touchBar.defaultItemIdentifiers = [.deleteSaveState,
                                           .flexibleSpace,
                                           .resumeSaveState,
                                           .otherItemsProxy]
        touchBar.customizationAllowedItemIdentifiers = [.deleteSaveState,
                                                        .resumeSaveState]
        touchBar.principalItemIdentifier = .resumeSaveState
        
        return touchBar
    }
    
    override func selectionDidChange() {
        guard let touchBar = self.touchBar else { return }
        let selected = self.collectionView.selectionIndexPaths
        
        if let deleteItem = touchBar.item(forIdentifier: .deleteSaveState) as? NSCustomTouchBarItem {
            let button = deleteItem.view as! NSButton
            button.isEnabled = !selected.isEmpty
        }
        
        if let playItem = touchBar.item(forIdentifier: .resumeSaveState) as? NSCustomTouchBarItem {
            let button = playItem.view as! NSButton
            button.isEnabled = selected.count == 1
        }
    }
}

extension SaveStateViewController: NSTouchBarDelegate {
    
    public func touchBar(_ touchBar: NSTouchBar, makeItemForIdentifier identifier: NSTouchBarItem.Identifier) -> NSTouchBarItem? {
        
        switch identifier {
            
        case .deleteSaveState:
            
            let item = NSCustomTouchBarItem(identifier: identifier)
            item.customizationLabel = NSLocalizedString("Delete", comment: "")
            
            let button = NSButton(image: NSImage(named: NSImage.touchBarDeleteTemplateName)!, target: nil, action: #selector(deleteSelectedItems(_:)))
            
            button.isEnabled = !collectionView.selectionIndexPaths.isEmpty
            button.bezelColor = #colorLiteral(red: 0.5665243268, green: 0.2167189717, blue: 0.2198875844, alpha: 1)
            
            item.view = button
            
            return item
            
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

