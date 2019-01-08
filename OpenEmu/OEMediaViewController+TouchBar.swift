/*
 Copyright (c) 2016, OpenEmu Team
 
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

import Cocoa

@available(OSX 10.12.2, *)
fileprivate extension NSTouchBar.CustomizationIdentifier {
    static let saveStatesTouchBar = NSTouchBar.CustomizationIdentifier("org.openemu.OEMediaViewController.saveStatesTouchBar")
    static let screenshotsTouchBar = NSTouchBar.CustomizationIdentifier("org.openemu.OEMediaViewController.screenshotsTouchBar")
}

@available(OSX 10.12.2, *)
fileprivate extension NSTouchBarItem.Identifier {
    
    // Save States
    static let deleteSaveState = NSTouchBarItem.Identifier("org.openemu.OEMediaViewController.saveStatesTouchbar.delete")
    static let resumeSaveState = NSTouchBarItem.Identifier("org.openemu.OEMediaViewController.saveStatesTouchbar.resume")
    
    // Screenshots
    static let deleteScreenshot = NSTouchBarItem.Identifier("org.openemu.OEMediaViewController.screenshotsTouchBar.delete")
    static let showScreenshotInFinder = NSTouchBarItem.Identifier("org.openemu.OEMediaViewController.screenshotsTouchBar.showInFinder")
    static let shareScreenshot = NSTouchBarItem.Identifier("org.openemu.OEMediaViewController.screenshotsTouchBar.share")
}

@available(OSX 10.12.2, *)
extension OEMediaViewController {
    
    open override func makeTouchBar() -> NSTouchBar? {
        
        let touchBar = MediaTouchBar()
        
        touchBar.mediaViewController = self
        touchBar.delegate = self
        
        if saveStateMode {
            
            touchBar.customizationIdentifier = .saveStatesTouchBar
            touchBar.defaultItemIdentifiers = [.deleteSaveState,
                                               .flexibleSpace,
                                               .resumeSaveState,
                                               .otherItemsProxy]
            touchBar.customizationAllowedItemIdentifiers = [.deleteSaveState,
                                                            .resumeSaveState]
            touchBar.principalItemIdentifier = .resumeSaveState
            
        } else {
            
            touchBar.customizationIdentifier = .screenshotsTouchBar
            touchBar.defaultItemIdentifiers = [.deleteScreenshot,
                                               .flexibleSpace,
                                               .showScreenshotInFinder,
                                               .shareScreenshot,
                                               .otherItemsProxy]
            touchBar.customizationAllowedItemIdentifiers = [.deleteScreenshot,
                                                            .showScreenshotInFinder,
                                                            .shareScreenshot]
            touchBar.principalItemIdentifier = .showScreenshotInFinder
        }
        
        return touchBar
    }
}

@available(OSX 10.12.2, *)
extension OEMediaViewController: NSTouchBarDelegate {
    
    public func touchBar(_ touchBar: NSTouchBar, makeItemForIdentifier identifier: NSTouchBarItem.Identifier) -> NSTouchBarItem? {
        
        switch identifier {
            
        case .deleteSaveState:
            fallthrough
        case .deleteScreenshot:
            
            let item = NSCustomTouchBarItem(identifier: identifier)
            item.customizationLabel = NSLocalizedString("Delete", comment: "")
            
            let button = NSButton(image: NSImage(named: .touchBarDeleteTemplate)!, target: nil, action: #selector(deleteSelectedItems(_:)))
            
            button.isEnabled = !selectionIndexes.isEmpty
            button.bezelColor = #colorLiteral(red: 0.5665243268, green: 0.2167189717, blue: 0.2198875844, alpha: 1)
            
            item.view = button
            
            return item
            
        case .resumeSaveState:
            
            let item = NSCustomTouchBarItem(identifier: identifier)
            item.customizationLabel = NSLocalizedString("Resume Game", comment: "")
            
            let button = NSButton(image: NSImage(named: .touchBarPlayTemplate)!, target: nil, action: #selector(OELibraryController.startSaveState(_:)))
            
            button.isEnabled = selectionIndexes.count == 1
            
            item.view = button
            
            return item
            
        case .showScreenshotInFinder:
            
            let item = NSCustomTouchBarItem(identifier: identifier)
            item.customizationLabel = NSLocalizedString("Show In Finder", comment: "")
            
            let button = NSButton(image: NSImage(named: .revealFreestandingTemplate)!, target: nil, action: #selector(showInFinder(_:)))
            
            button.isEnabled = !selectionIndexes.isEmpty
            
            item.view = button
            
            return item
            
        case .shareScreenshot:
            
            let item = NSSharingServicePickerTouchBarItem(identifier: identifier)
            
            item.delegate = self
            item.isEnabled = !selectionIndexes.isEmpty
            
            return item
            
        default:
            return nil
        }
    }
}

@available(OSX 10.12.2, *)
extension OEMediaViewController: NSSharingServicePickerTouchBarItemDelegate {
    
    public func items(for pickerTouchBarItem: NSSharingServicePickerTouchBarItem) -> [Any] {
        return selectedScreenshots.compactMap { NSImage(contentsOf: $0.url as URL) }
    }
}

@available(OSX 10.12.2, *)
private class MediaTouchBar: NSTouchBar {
    
    weak var mediaViewController: OEMediaViewController? {
        didSet {
            
            if let oldMediaViewController = oldValue {
                NotificationCenter.default.removeObserver(self, name: Notification.Name.OEMediaViewControllerDidSetSelectionIndexes, object: oldMediaViewController)
            }
            
            if let newMediaViewController = mediaViewController {
                NotificationCenter.default.addObserver(self, selector: #selector(MediaTouchBar.selectionIndexesDidChange(_:)), name: Notification.Name.OEMediaViewControllerDidSetSelectionIndexes, object: newMediaViewController)
            }
        }
    }
    
    @objc func selectionIndexesDidChange(_ notification: Notification) {
        updateButtonStatesForMediaViewControllerSelection()
    }
    
    func updateButtonStatesForMediaViewControllerSelection() {
        
        guard let mediaViewController = mediaViewController else {
            return
        }
        
        if mediaViewController.saveStateMode {
            
            if let deleteItem = item(forIdentifier: .deleteSaveState) as? NSCustomTouchBarItem {
                let button = deleteItem.view as! NSButton
                button.isEnabled = !mediaViewController.selectionIndexes.isEmpty
            }
            
            if let playItem = item(forIdentifier: .resumeSaveState) as? NSCustomTouchBarItem {
                let button = playItem.view as! NSButton
                button.isEnabled = mediaViewController.selectionIndexes.count == 1
            }
            
        } else {
            
            if let deleteItem = item(forIdentifier: .deleteScreenshot) as? NSCustomTouchBarItem {
                let button = deleteItem.view as! NSButton
                button.isEnabled = !mediaViewController.selectionIndexes.isEmpty
            }
            
            if let showInFinderItem = item(forIdentifier: .showScreenshotInFinder) as? NSCustomTouchBarItem {
                let button = showInFinderItem.view as! NSButton
                button.isEnabled = !mediaViewController.selectionIndexes.isEmpty
            }
         
            if let shareItem = item(forIdentifier: .shareScreenshot) as? NSSharingServicePickerTouchBarItem {
                shareItem.isEnabled = !mediaViewController.selectionIndexes.isEmpty
            }
        }
    }
}
