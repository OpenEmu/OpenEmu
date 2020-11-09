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

@objc(OEHUDButton)
final class HUDButton: HoverButton {
    
    @objc var imageName: String? {
        didSet {
            needsDisplay = true
        }
    }
    
    @objc var alternateImageName: String? {
        didSet {
            needsDisplay = true
        }
    }
    
    @objc var backgroundColor: NSColor? {
        didSet {
            if backgroundColor == .black {
                backgroundSubImageRect = NSRect(x: 0, y: 69, width: 51, height: 23)
            }
            else if backgroundColor == .red {
                backgroundSubImageRect = NSRect(x: 0, y: 0, width: 51, height: 23)
            }
            else if backgroundColor == .blue {
                backgroundSubImageRect = NSRect(x: 0, y: 23, width: 51, height: 23)
            }
            needsDisplay = true
        }
    }
    
    private var backgroundSubImageRect: NSRect?
    private lazy var backgroundSubImageHighlightRect = NSRect(x: 0, y: 46, width: 51, height: 23)
    
    override func draw(_ dirtyRect: NSRect) {
        
        if backgroundSubImageRect != nil {
            let bgSubImageRect = isHighlighted ? backgroundSubImageHighlightRect : backgroundSubImageRect!
            NSImage(named: "HUD/hud_button")?.subImage(from: bgSubImageRect).draw(in: dirtyRect)
        }
        
        let image = subImageFromImageName
        
        let imageRect = NSRect(x: frame.size.width/2 - image.size.width/2,
                               y: frame.size.height/2 - image.size.height/2,
                           width: image.size.width,
                          height: image.size.height)
        
        image.draw(in: imageRect)
    }
    
    private var subImageFromImageName: NSImage {
        
        if var image = NSImage(named: imageName ?? "") {
            
            if self.state == .on, alternateImageName != nil {
                image = NSImage(named: alternateImageName!)!
            }
            
            let imageWidth = image.size.width
            let imageHeight = image.size.height
            let numberOfSubImages = round(imageWidth/imageHeight)
            
            var index: CGFloat
            
            if numberOfSubImages > 1 {
                if (numberOfSubImages == 3 && isHighlighted) {
                    index = 2
                }
                else if (numberOfSubImages == 3 && isHovering ) || (numberOfSubImages == 2 && isHighlighted) {
                    index = 1
                }
                else {
                    index = 0
                }
                
                let subImageRect = NSRect(x: imageWidth/numberOfSubImages*index, y: 0, width: imageWidth/numberOfSubImages, height: imageHeight)
                
                return image.subImage(from: subImageRect)
            }
            return image
        }
        return NSImage(named: NSImage.cautionName)!
    }
}
