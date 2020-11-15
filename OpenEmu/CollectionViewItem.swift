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

class CollectionViewItem: NSCollectionViewItem {
    
    var selectionLayer: CALayer?
    
    var imageURL: URL?
    
    override var highlightState: NSCollectionViewItem.HighlightState {
        didSet {
            if oldValue != highlightState {
                self.updateHighlightState()
            }
        }
    }
    
    override var isSelected: Bool {
        didSet {
            if oldValue != isSelected {
                self.updateHighlightState()
            }
        }
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()
        view.wantsLayer = true
    }
    
    override func prepareForReuse() {
        super.prepareForReuse()
        selectionLayer?.removeFromSuperlayer()
        selectionLayer = nil
    }
    
    override func viewDidLayout() {
        super.viewDidLayout()
        
        if let layer = selectionLayer, let imageView = imageView {
            var rect = imageView.croppedBounds
            rect = view.convert(rect, from: imageView)
            
            CATransaction.begin()
            defer { CATransaction.commit() }
            CATransaction.setDisableActions(true)
            
            layer.frame = rect.insetBy(dx: -6, dy: -6)
        }
    }
    
    func updateHighlightState() {
        if (isSelected || highlightState == .forSelection) && selectionLayer == nil {
            guard let layer = view.layer else { return }
            
            let sel = CALayer()
            sel.borderColor = NSColor.selectedContentBackgroundColor.cgColor
            sel.borderWidth = 4.0
            sel.cornerRadius = 3.0
            
            selectionLayer = sel
            layer.addSublayer(sel)
        } else if let sel = selectionLayer, (!isSelected || highlightState == .forDeselection) {
            sel.removeFromSuperlayer()
            selectionLayer = nil
        }
    }
}
