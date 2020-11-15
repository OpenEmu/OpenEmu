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

protocol CollectionViewExtendedDelegate {
    var supportsQuickLook: Bool { get }
    func collectionView(_ collectionView: CollectionView, menuForItemsAt indexPaths: Set<IndexPath>) -> NSMenu?
    func collectionView(_ collectionView: CollectionView, setTitle title: String, forItemAt indexPath: IndexPath)
    func collectionView(_ collectionView: CollectionView, doubleClickForItemAt indexPath: IndexPath)
}

extension CollectionViewExtendedDelegate {
    var supportsQuickLook: Bool { true }
    
    func collectionView(_ collectionView: CollectionView, menuForItemsAt indexPaths: Set<IndexPath>) -> NSMenu? { nil }
    func collectionView(_ collectionView: CollectionView, setTitle title: String, forItemAt indexPath: IndexPath) {}
    func collectionView(_ collectionView: CollectionView, doubleClickForItemAt indexPath: IndexPath) {}
}

class CollectionView: NSCollectionView {
    
    // MARK: - Title Editing Support
    private var editing: NSCollectionViewItem?
    private var editedItem: IndexPath?
    private var fieldEditor: NSText?
    
    // MARK: - Other State
    
    var extendedDelegate: CollectionViewExtendedDelegate?
    
    override var delegate: NSCollectionViewDelegate? {
        didSet {
            guard let del = delegate as? CollectionViewExtendedDelegate else {
                return
            }
            
            extendedDelegate = del
        }
    }
    
    private var isKey: Bool = false
    private var selectionColor: NSColor {
        if isKey {
            return .selectedContentBackgroundColor
        }
        return .unemphasizedSelectedContentBackgroundColor
    }
    
    @objc func resignKeyWindow() {
        isKey = false
        updateSelectionHighlights()
    }
    
    @objc func becomeKeyWindow() {
        isKey = true
        updateSelectionHighlights()
    }
    
    override func menu(for event: NSEvent) -> NSMenu? {
        let mouseLocationInView = convert(event.locationInWindow, from: nil)
        
        guard
            let index = indexPathForItem(at: mouseLocationInView),
            let ed    = extendedDelegate
            else { return super.menu(for: event) }
        
        let itemIsSelected  = item(at: index)!.isSelected
        let indexPaths      = itemIsSelected ? selectionIndexPaths : Set([index])
        
        if !itemIsSelected {
            selectionIndexPaths = indexPaths
        }
        
        if let menu = ed.collectionView(self, menuForItemsAt: indexPaths) {
            window?.makeFirstResponder(self)
            NSMenu.popUpContextMenu(menu, with: event, for: self)
        }
        
        return nil
    }
    
    override func mouseDown(with event: NSEvent) {
        if editedItem != nil {
            // if we are already editing something, we need to cancel the operation
            endEditing()
        }
        
        if event.clickCount == 2 {
            let local = convert(event.locationInWindow, from: nil)
            guard
                let index = indexPathForItem(at: local),
                let item = item(at: index)
                else { return }
            
            if let tf = item.textField {
                let p = tf.convert(event.locationInWindow, from: nil)
                if tf.isMousePoint(p, in: tf.bounds) {
                    beginEditing(itemAt: index)
                    return
                }
            }
            
            extendedDelegate?.collectionView(self, doubleClickForItemAt: index)
            return
        }
        super.mouseDown(with: event)
    }
    
    override func keyDown(with event: NSEvent) {
        switch Int(event.keyCode) {
        case kVK_Space:
            toggleQuickLook()
            
        case kVK_Return:
            beginEditingWithSelectedItem(self)
        default:
            super.keyDown(with: event)
        }
    }
    
    func updateSelectionHighlights() {
        CATransaction.begin()
        defer { CATransaction.commit() }
        CATransaction.setDisableActions(true)
        
        let color = selectionColor.cgColor
        for index in selectionIndexPaths {
            let v = item(at: index) as? CollectionViewItem
            v?.selectionLayer?.borderColor = color
        }
    }
}

// MARK: - Quick Look support

extension CollectionView: QLPreviewPanelDataSource, QLPreviewPanelDelegate {
    func toggleQuickLook() {
        let panel = QLPreviewPanel.shared()!
        if !acceptsPreviewPanelControl(panel) {
            return
        }
        
        if panel.isVisible {
            panel.orderOut(nil)
        } else {
            panel.makeKeyAndOrderFront(nil)
        }
    }
    
    func refreshPreviewPanelIfNeeded() {
        if QLPreviewPanel.sharedPreviewPanelExists() {
            let panel = QLPreviewPanel.shared()!
            if panel.isVisible && panel.delegate === self {
                panel.reloadData()
            }
        }
    }
    
    override func acceptsPreviewPanelControl(_ panel: QLPreviewPanel!) -> Bool {
        return extendedDelegate?.supportsQuickLook ?? false
    }
    
    override func beginPreviewPanelControl(_ panel: QLPreviewPanel!) {
        panel.dataSource = self
        panel.delegate   = self
    }
    
    override func endPreviewPanelControl(_ panel: QLPreviewPanel!) {
        panel.dataSource = nil
        panel.delegate   = nil
    }
    
    func numberOfPreviewItems(in panel: QLPreviewPanel!) -> Int {
        return selectionIndexPaths.count
    }
    
    func previewPanel(_ panel: QLPreviewPanel!, previewItemAt index: Int) -> QLPreviewItem! {
        let sip = selectionIndexPaths
        let si = sip.index(sip.startIndex, offsetBy: index)
        return item(at: sip[si]) as? QLPreviewItem
    }
    
    func previewPanel(_ panel: QLPreviewPanel!, sourceFrameOnScreenFor item: QLPreviewItem!) -> NSRect {
        guard
            let item = item as? NSCollectionViewItem,
            let view = item.imageView,
            let win  = window
            else { return NSRect.zero }
        var rect = view.croppedBounds
        rect = item.view.convert(rect, from: view)
        let windowRect = item.view.convert(rect, to: nil)
        return win.convertToScreen(windowRect)
    }
    
    func previewPanel(_ panel: QLPreviewPanel!, handle event: NSEvent!) -> Bool {
        if event.type == .keyDown || event.type == .keyUp {
            if let window = self.window {
                window.sendEvent(event)
                return true
            }
        }
        return false
    }
}

// MARK: - Text Editing

extension CollectionView: NSTextDelegate {
    @objc public func beginEditingWithSelectedItem(_ sender: Any?) {
        if selectionIndexPaths.count != 1 {
            return
        }
        
        beginEditing(itemAt: selectionIndexPaths.first!)
    }
    
    private func beginEditing(itemAt indexPath: IndexPath) {
        if editedItem != nil {
            endEditing()
        }
        
        guard
            let item = item(at: indexPath),
            let tf = item.textField
        else { return }
        
        if let editor = tf.window?.fieldEditor(true, for: tf) {
            fieldEditor = editor
            editedItem = indexPath
            editor.delegate = self
            tf.isEditable = true
            tf.select(withFrame: tf.bounds, editor: editor, delegate: self, start: 0, length: 0)
        }
    }

    func textDidEndEditing(_ notification: Notification) {
        if let ed = extendedDelegate,
            let editor = fieldEditor,
            let item = editedItem {
            ed.collectionView(self, setTitle: editor.string, forItemAt: item)
        }
        endEditing()
    }
    
    override func cancelOperation(_ sender: Any?) {
        endEditing()
    }
    
    func endEditing() {
        guard let indexPath = editedItem else { return }
        editedItem = nil
        
        guard let editing = item(at: indexPath) else { return }
        
        guard let tf = editing.textField else { return }
        tf.isEditable = false
        if let editor = fieldEditor {
            fieldEditor = nil
            editor.delegate = nil
            tf.endEditing(editor)
        }
        window?.makeFirstResponder(self)
    }
    
}
