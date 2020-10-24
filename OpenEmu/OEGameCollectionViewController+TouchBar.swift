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

fileprivate extension NSTouchBar.CustomizationIdentifier {
    static let touchBar = "org.openemu.OEGameCollectionViewController.play"
}

fileprivate extension NSTouchBarItem.Identifier {
    static let delete = NSTouchBarItem.Identifier("org.openemu.OEGameCollectionViewController.delete")
    static let play = NSTouchBarItem.Identifier("org.openemu.OEGameCollectionViewController.play")
}

extension OEGameCollectionViewController {
    
    open override func makeTouchBar() -> NSTouchBar? {
        
        let touchBar = GamesTouchBar()
        
        touchBar.gameCollectionViewController = self
        
        touchBar.delegate = self
        touchBar.customizationIdentifier = .touchBar
        touchBar.defaultItemIdentifiers = [.delete,
                                           .play,
                                           .otherItemsProxy]
        touchBar.customizationAllowedItemIdentifiers = [.delete,
                                                        .play]
        touchBar.principalItemIdentifier = .play
        
        return touchBar
    }
}

extension OEGameCollectionViewController: NSTouchBarDelegate {
    
    public func touchBar(_ touchBar: NSTouchBar, makeItemForIdentifier identifier: NSTouchBarItem.Identifier) -> NSTouchBarItem? {
        
        switch identifier {
            
        case .delete:
            
            let item = NSCustomTouchBarItem(identifier: identifier)
            item.customizationLabel = NSLocalizedString("Delete", comment: "")
            
            let button = NSButton(image: NSImage(named: NSImage.touchBarDeleteTemplateName)!, target: nil, action: #selector(deleteSelectedItems(_:)))
            
            button.isEnabled = !selectionIndexes.isEmpty
            button.bezelColor = #colorLiteral(red: 0.5665243268, green: 0.2167189717, blue: 0.2198875844, alpha: 1)
            
            item.view = button
            
            return item
            
        case .play:
            
            let item = NSCustomTouchBarItem(identifier: identifier)
            item.customizationLabel = NSLocalizedString("Play", comment: "")
            
            let button = NSButton(image: NSImage(named: NSImage.touchBarPlayTemplateName)!, target: nil, action: #selector(LibraryController.startSelectedGame(_:)))
            
            button.isEnabled = selectionIndexes.count == 1
            
            item.view = button
            
            return item
            
        default:
            return nil
        }
    }
}

fileprivate class GamesTouchBar: NSTouchBar {
    
    weak var gameCollectionViewController: OEGameCollectionViewController? {
        didSet {
            
            if let oldGameCollectionViewController = oldValue {
                NotificationCenter.default.removeObserver(self, name: Notification.Name.OEGameCollectionViewControllerDidSetSelectionIndexes, object: oldGameCollectionViewController)
            }
            
            if let newGameCollectionViewController = gameCollectionViewController {
                NotificationCenter.default.addObserver(self, selector: #selector(GamesTouchBar.selectionIndexesDidChange(_:)), name: Notification.Name.OEGameCollectionViewControllerDidSetSelectionIndexes, object: newGameCollectionViewController)
            }
        }
    }
    
    @objc func selectionIndexesDidChange(_ notification: Notification) {
        updateButtonStatesForGameCollectionViewControllerSelection()
    }
    
    func updateButtonStatesForGameCollectionViewControllerSelection() {
        
        guard let gameCollectionViewController = gameCollectionViewController else {
            return
        }
        
        if let deleteItem = item(forIdentifier: .delete) as? NSCustomTouchBarItem {
            let button = deleteItem.view as! NSButton
            button.isEnabled = !gameCollectionViewController.selectionIndexes.isEmpty
        }
            
        if let playItem = item(forIdentifier: .play) as? NSCustomTouchBarItem {
            let button = playItem.view as! NSButton
            button.isEnabled = gameCollectionViewController.selectionIndexes.count == 1
        }
    }
}
