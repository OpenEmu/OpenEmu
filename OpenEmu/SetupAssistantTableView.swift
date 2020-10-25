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

import Cocoa

@objc(OESetupAssistantTableView)
class SetupAssistantTableView: NSTableView, NSTableViewDataSource {
    
    override init(frame: NSRect) {
        super.init(frame: frame)
        commonInit()
    }
    
    required init?(coder: NSCoder) {
        super.init(coder: coder)
        commonInit()
    }
    
    func commonInit() {
        rowHeight = 23
        backgroundColor = .clear
    }
    
    // MARK: - Drawing
    
    override func highlightSelection(inClipRect clipRect: NSRect) {
        
        guard window != nil else {
            return
        }
        
        let isActive = window!.isMainWindow && window!.firstResponder == self
        
        var fillColor: NSColor?
        if isActive {
            fillColor = NSColor(red: 27/255, green: 49/255, blue: 139/255, alpha: 0.5)
        } else {
            fillColor = NSColor(white: 0.55, alpha: 0.5)
        }
        
        fillColor?.setFill()
        
        let selectedRows = selectedRowIndexes as NSIndexSet
        selectedRows.enumerate({ [self] idx, stop in
            let frame = rect(ofRow: idx)
            frame.fill()
        })
    }
    
    override func drawBackground(inClipRect clipRect: NSRect) {
        
        let rowBackground = NSColor(white: 0, alpha: 0.1)
        let alternateRowBackground = NSColor(white: 1, alpha: 0.01)
        
        rowBackground.setFill()
        
        let rowHeight = self.rowHeight + intercellSpacing.height
        
        var i = 0
        while i < Int(bounds.maxY) {
            NSRect(x: bounds.origin.x, y: CGFloat(i), width: bounds.size.width, height: rowHeight).fill()
            i += 2 * Int(rowHeight)
        }
        
        alternateRowBackground.setFill()
        
        i = Int(rowHeight)
        while i < Int(bounds.maxY) {
            NSRect(x: bounds.origin.x, y: CGFloat(i), width: bounds.size.width, height: rowHeight).fill()
            i += 2 * Int(rowHeight)
        }
    }
}
