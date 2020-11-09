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

@objc(OETextButton)
class TextButton: HoverButton {
    
    @objc var textColor: NSColor?
    @objc var textColorHover: NSColor?
    @objc var textShadow: NSShadow?
    
    @objc var showArrow: Bool = false
    
    private var arrowImage: NSImage? {
        if isHighlighted {
            return NSImage(named: "arrow_right")?.withTintColor(NSColor.tertiaryLabelColor)
        }
        else if isHovering {
            return NSImage(named: "arrow_right")?.withTintColor(NSColor.labelColor)
        }
        else {
            return NSImage(named: "arrow_right")?.withTintColor(NSColor.secondaryLabelColor)
        }
    }
    
    required init?(coder: NSCoder) {
        super.init(coder: coder)
        commonInit()
    }
    
    override init(frame frameRect: NSRect) {
        super.init(frame: frameRect)
        commonInit()
    }
    
    private func commonInit() {
        bezelStyle = .shadowlessSquare
        isBordered = false
        setupAttributes()
    }
    
    func setupAttributes() {
    }
    
    private var attributes: [NSAttributedString.Key : Any] {
        
        let font = self.font ?? .systemFont(ofSize: NSFont.systemFontSize)
        let color = textColor ?? .labelColor
        let hoverColor = textColorHover ?? NSColor.labelColor.withSystemEffect(.rollover)
        
        var attributes: [NSAttributedString.Key : Any] =
                                          [.font: font,
                                .foregroundColor: isHovering ? hoverColor : color]
        
        if let shadow = textShadow {
            attributes[.shadow] = shadow
        }
        
        return attributes
    }
    
    override func draw(_ dirtyRect: NSRect) {
        
        attributedTitle = NSAttributedString(string: title, attributes: attributes)
        
        if showArrow {
            image = arrowImage
            imagePosition = .imageRight
        }
        
        super.draw(dirtyRect)
    }
    
    override func sizeToFit() {
        super.sizeToFit()
        if showArrow {
            var frameSize = attributedTitle.size()
            frameSize.width += 24
            setFrameSize(frameSize)
        }
    }
}
