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

class SetupAssistantTextCell: NSTextFieldCell {
    
    override func drawInterior(withFrame cellFrame: NSRect, in controlView: NSView) {
        
        var cellFrame = cellFrame
        let contentSize = cellSize
        
        cellFrame.origin.y += (cellFrame.size.height - contentSize.height) / 2
        cellFrame.size.height = contentSize.height
        
        cellFrame = cellFrame.insetBy(dx: 3, dy: 0)
        cellFrame.origin.y += 2
        
        let string = NSAttributedString(string: stringValue, attributes: attributes())
        
        string.draw(in: cellFrame)
    }
    
    func attributes() -> [NSAttributedString.Key : Any] {
        return [:]
    }
}

@objc(OESetupAssistantMajorTextCell)
class SetupAssistantMajorTextCell: SetupAssistantTextCell {
    
    override func attributes() -> [NSAttributedString.Key : Any] {
        let font = NSFont.boldSystemFont(ofSize: 11.5)
        let color = NSColor(deviceWhite: 0.89, alpha: 1)
        
        let style = NSMutableParagraphStyle()
        style.alignment = .left
        
        return [.font: font, .foregroundColor: color, .paragraphStyle: style]
    }
}

@objc(OESetupAssistantMinorTextCell)
class SetupAssistantMinorTextCell: SetupAssistantTextCell {
    
    override func attributes() -> [NSAttributedString.Key : Any] {
        let font = NSFont.systemFont(ofSize: 11)
        let color = NSColor(deviceWhite: 0.86, alpha: 1)
        
        let style = NSMutableParagraphStyle()
        style.alignment = .left
        style.lineBreakMode = .byTruncatingTail
        
        return [.font: font, .foregroundColor: color, .paragraphStyle: style]
    }
}
