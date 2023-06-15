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

final class GlossButton: NSButton {
    
    private static let attributes: [NSAttributedString.Key : Any] = {
        
        let font = NSFont.boldSystemFont(ofSize: 11)
        let color = NSColor(white: 0.91, alpha: 1)
        
        let shadow = NSShadow()
        shadow.shadowColor = .black
        shadow.shadowOffset = NSMakeSize(0, 1)
        
        let style = NSMutableParagraphStyle()
        style.alignment = .center
        
        let attributes: [NSAttributedString.Key : Any] =
                                          [.font: font,
                                .foregroundColor: color,
                                         .shadow: shadow,
                                 .paragraphStyle: style]
        
        return attributes
    }()
    
    private static let attributesHighlighted: [NSAttributedString.Key : Any] = {
        
        let font = NSFont.boldSystemFont(ofSize: 11)
        let color = NSColor(white: 0.03, alpha: 1)
        
        let shadow = NSShadow()
        shadow.shadowColor = .white.withAlphaComponent(0.4)
        shadow.shadowOffset = NSMakeSize(0, -1)
        
        let style = NSMutableParagraphStyle()
        style.alignment = .center
        
        let attributes: [NSAttributedString.Key : Any] =
                                          [.font: font,
                                .foregroundColor: color,
                                         .shadow: shadow,
                                 .paragraphStyle: style]
        
        return attributes
    }()
    
    @IBInspectable var color: String? {
        didSet {
            needsDisplay = true
        }
    }
    
    override var isFlipped: Bool {
        return false
    }
    
    private var imageToDraw: NSImage? {
        var imageName = "gloss_button"
        
        if isHighlighted {
            imageName += "_highlighted"
            return NSImage(named: imageName)
        }
        
        if let color {
            imageName += "_\(color)"
        }
        
        if !isEnabled || window?.isMainWindow == false {
            imageName += "_disabled"
        }
        
        return NSImage(named: imageName)
    }
    
    override func draw(_ dirtyRect: NSRect) {
        assert(imageToDraw != nil)
        imageToDraw?.draw(in: bounds)
        
        let attributes = isHighlighted ? Self.attributesHighlighted : Self.attributes
        attributedTitle = NSAttributedString(string: title, attributes: attributes)
        
        super.draw(dirtyRect)
    }
    
    override func viewWillMove(toWindow newWindow: NSWindow?) {
        super.viewWillMove(toWindow: newWindow)
        
        if window != nil {
            NotificationCenter.default.removeObserver(self, name: NSWindow.didBecomeMainNotification, object: window)
            NotificationCenter.default.removeObserver(self, name: NSWindow.didResignMainNotification, object: window)
        }
        
        if newWindow != nil {
            NotificationCenter.default.addObserver(self, selector: #selector(windowKeyChanged), name: NSWindow.didBecomeMainNotification, object: newWindow)
            NotificationCenter.default.addObserver(self, selector: #selector(windowKeyChanged), name: NSWindow.didResignMainNotification, object: newWindow)
        }
    }
    
    @objc func windowKeyChanged() {
        needsDisplay = true
    }
}
