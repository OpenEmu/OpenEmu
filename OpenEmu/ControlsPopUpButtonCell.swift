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

final class ControlsPopUpButtonCell: NSPopUpButtonCell {
    
    private var imagePadding: CGFloat = 4
    private var titleLeftPadding: CGFloat = 11
    
    // Used for right-to-left languages
    private var titleRightPadding: CGFloat = 4
    private var arrowWidth: CGFloat = 16
    
    override func drawBorderAndBackground(withFrame cellFrame: NSRect, in controlView: NSView) {
        NSImage(named: "wood_popup_button")?.draw(in: cellFrame)
    }
    
    override func drawImage(_ image: NSImage, withFrame frame: NSRect, in controlView: NSView) {
        image.draw(in: frame)
    }
    
    override func titleRect(forBounds cellFrame: NSRect) -> NSRect {
        var titleRect = super.titleRect(forBounds: cellFrame)
        let imageRect = imageRect(forBounds: cellFrame)
        
        if #available(macOS 11.0, *) {
            #if canImport(AppKit, _version: 2665.8)     // Fix the title for SDKs 26.0 and above
                titleRect.origin.y -= 0.5

                let titleWidth = titleRect.width
                let hasImage = !imageRect.isEmpty
                let imageWidth = imageRect.size.width
                if (userInterfaceLayoutDirection == .rightToLeft) {
                    titleRect.origin.x = cellFrame.maxX - arrowWidth - titleRightPadding - titleWidth
                    
                    if (hasImage) {
                        titleRect.origin.x -= (imageWidth + imagePadding)
                    }
                } else {
                    titleRect.origin.x = cellFrame.minX + titleLeftPadding
                    
                    if (hasImage) {
                        titleRect.origin.x += imageWidth + imagePadding
                    }
                }
            
                if (titleRect.origin.x < titleLeftPadding) {
                    titleRect.origin.x = titleLeftPadding
                }
            #else
                titleRect.origin.y -= 3
            #endif
        } else {
            titleRect.origin.y -= 2
        }
        
        return titleRect
    }
    
    override func imageRect(forBounds rect: NSRect) -> NSRect {
        if #available(macOS 11.0, *) {
            var imageRect = super.imageRect(forBounds: rect)
            
            #if canImport(AppKit, _version: 2665.8)    // Fix the image placement for SDKs 26.0 and above
                imageRect.origin.y += 0.5
            
                if (userInterfaceLayoutDirection == .rightToLeft) {
                    imageRect.origin.x -= 7
                }
                imageRect.origin.x -= 1
            #else
                imageRect.origin.y -= 1
            #endif
            
            return imageRect
        }
        return super.imageRect(forBounds: rect)
    }
    
    override func drawInterior(withFrame cellFrame: NSRect, in controlView: NSView) {
        let titleRect = titleRect(forBounds: cellFrame)
        let imageRect = imageRect(forBounds: cellFrame)
        
        if !titleRect.isEmpty,
            let title = title {
            title.draw(in: titleRect, withAttributes: Self.attributes)
        }
        if !imageRect.isEmpty,
           let image = image {
            drawImage(image, withFrame: imageRect, in: controlView)
        }
    }
    
    private static let attributes: [NSAttributedString.Key : Any] = {
        
        let style = NSMutableParagraphStyle()
        style.lineBreakMode = .byTruncatingTail
        
        let attributes: [NSAttributedString.Key : Any] =
                                          [.font: NSFont.boldSystemFont(ofSize: 11),
                                .foregroundColor: NSColor.black,
                                         .shadow: NSShadow.oeControls,
                                 .paragraphStyle: style]
        
        return attributes
    }()
}
