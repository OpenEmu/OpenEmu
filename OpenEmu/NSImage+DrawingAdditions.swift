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

import Foundation
import CoreGraphics

class ThreePartImage: NSImage {
    
    /// Array of the three different parts.
    var parts: [NSImage?]!
    
    /// Image should be rendered vertically.
    var vertical = false
    
    convenience init(imageParts: [NSImage?], vertical: Bool) {
        
        let start  = imageParts[0]?.size ?? NSSize.zero
        let center = imageParts[1]?.size ?? NSSize.zero
        let end    = imageParts[2]?.size ?? NSSize.zero
        
        var size = NSSize.zero
        if vertical {
            size.width = max(max(start.width, center.width), end.width)
            size.height = start.height + center.height + end.height
        } else {
            size.width = start.width + center.width + end.width
            size.height = max(max(start.height, center.height), end.height)
        }
        
        self.init(size: size)
        
        self.parts = imageParts
        self.vertical = vertical
    }
    
    override func draw(in rect: NSRect,
        from fromRect: NSRect,
        operation op: NSCompositingOperation,
        fraction delta: CGFloat) {
            
        draw(in: rect,
            from: fromRect,
            operation: op,
            fraction: delta,
            respectFlipped: false,
            hints: nil)
    }
    
    override func draw(in dstSpacePortionRect: NSRect,
        from srcSpacePortionRect: NSRect,
        operation op: NSCompositingOperation,
        fraction requestedAlpha: CGFloat,
        respectFlipped respectContextIsFlipped: Bool,
        hints: [NSImageRep.HintKey : Any]?) {
            
            if (!vertical && dstSpacePortionRect.height != size.height) ||
                (vertical && dstSpacePortionRect.width != size.width) {
                    DLog("WARNING: Drawing a 3-part image at wrong size.")
            }
            
            let startCap   = parts[0]
            let centerFill = parts[1]
            let endCap     = parts[2]
            
            NSDrawThreePartImage(dstSpacePortionRect,
                startCap,
                centerFill,
                endCap,
                vertical,
                op,
                requestedAlpha,
                NSGraphicsContext.current?.isFlipped ?? false)
    }
}

class NinePartImage: NSImage {
    
    /// Array of the nine different parts.
    var parts: [NSImage?]!
    
    convenience init(imageParts: [NSImage?]) {
        
        let topLeft      = imageParts[0]?.size ?? NSZeroSize
        let topCenter    = imageParts[1]?.size ?? NSZeroSize
        let topRight     = imageParts[2]?.size ?? NSZeroSize
        let leftEdge     = imageParts[3]?.size ?? NSZeroSize
        let centerFill   = imageParts[4]?.size ?? NSZeroSize
        let rightEdge    = imageParts[5]?.size ?? NSZeroSize
        let bottomLeft   = imageParts[6]?.size ?? NSZeroSize
        let bottomCenter = imageParts[7]?.size ?? NSZeroSize
        let bottomRight  = imageParts[8]?.size ?? NSZeroSize
        
        let width1  = topLeft.width + topCenter.width + topRight.width
        let width2  = leftEdge.width + centerFill.width + rightEdge.width
        let width3  = bottomLeft.width + bottomCenter.width + bottomRight.width
        
        let height1 = topLeft.height + leftEdge.height + bottomLeft.height
        let height2 = topCenter.height + centerFill.height + bottomCenter.height
        let height3 = topRight.height + rightEdge.height + bottomRight.height
        
        let size = NSSize(width: max(max(width1, width2), width3),
                     height: max(max(height1, height2), height3))
        
        self.init(size: size)
        
        self.parts = imageParts
    }
    
    override func draw(in rect: NSRect,
        from fromRect: NSRect,
        operation op: NSCompositingOperation,
        fraction delta: CGFloat) {
        
            draw(in: rect,
                from: fromRect,
                operation: op,
                fraction: delta,
                respectFlipped: false,
                hints: nil)
    }
    
    override func draw(in dstSpacePortionRect: NSRect,
        from srcSpacePortionRect: NSRect,
        operation op: NSCompositingOperation,
        fraction requestedAlpha: CGFloat,
        respectFlipped respectContextIsFlipped: Bool,
        hints: [NSImageRep.HintKey : Any]?) {
        
            let topLeftCorner     = parts[0]!
            let topEdgeFill       = parts[1]!
            let topRightCorner    = parts[2]!
            let leftEdgeFill      = parts[3]!
            let centerFill        = parts[4]!
            let rightEdgeFill     = parts[5]!
            let bottomLeftCorner  = parts[6]!
            let bottomEdgeFill    = parts[7]!
            let bottomRightCorner = parts[8]!
            
            NSDrawNinePartImage(dstSpacePortionRect,
                topLeftCorner,
                topEdgeFill,
                topRightCorner,
                leftEdgeFill,
                centerFill,
                rightEdgeFill,
                bottomLeftCorner,
                bottomEdgeFill,
                bottomRightCorner,
                op,
                requestedAlpha,
                respectContextIsFlipped && NSGraphicsContext.current?.isFlipped ?? false)
    }
}

extension NSImage {
    
    @objc(subImageFromRect:)
    func subImage(from rect: NSRect) -> NSImage {
        if rect.origin == NSZeroPoint && rect.size == self.size {
            return self.copy() as! NSImage
        }
        
        let newImage = NSImage(size: rect.size)
        let flippedRect = NSMakeRect(rect.origin.x, (self.size.height-(rect.origin.y+rect.size.height)), rect.size.width, rect.size.height)
        for representation in self.representations {
            if representation.pixelsWide == 0 && representation.pixelsHigh == 0 {
                continue
            }
            let cgrep = representation.cgImage(forProposedRect: nil, context: nil, hints: nil)
            let xscale = CGFloat(representation.pixelsHigh) / representation.size.height;
            let yscale = CGFloat(representation.pixelsWide) / representation.size.width;
            if let croppedrep = cgrep?.cropping(to: CGRect(x:flippedRect.origin.x * xscale, y:flippedRect.origin.y * yscale, width:flippedRect.size.width * xscale, height:flippedRect.size.height * yscale)) {
                let newrep = NSBitmapImageRep(cgImage: croppedrep)
                newrep._appearanceName = representation._appearanceName
                newrep.size = rect.size
                newImage.addRepresentation(newrep)
            }
        }
        
        if newImage.representations.count == 0 {
            return NSImage(size: rect.size, flipped: false) { [unowned self] dstRect in
                self.draw(in: dstRect,
                    from: rect,
                    operation: .sourceOver,
                    fraction: 1)

                return true
            }
        }
        return newImage
    }
    
    @objc(imageFromParts:vertical:)
    func image(fromParts parts: [AnyObject]?, vertical: Bool) -> NSImage {
        
        guard let parts = parts, !parts.isEmpty else {
            return self
        }
        
        let count: Int = {
            
            switch parts.count {
                
            case 9..<Int.max:
                return 9
                
            case 3..<9:
                return 3
                
            case 1..<3:
                return 1
                
            default:
                return 0
            }
        }()
        
        let imageParts: [NSImage?] = parts.map { part in
            
            let rect: NSRect
            
            switch part {
                
            case let part as String:
                rect = NSRectFromString(part)
                
            case let part as NSValue:
                rect = part.rectValue
                
            default:
                NSLog("Unable to parse NSRect from part: \(part)")
                rect = NSZeroRect
            }
            
            if !rect.isEmpty {
                var subImageRect = rect
                subImageRect.origin.y = size.height - rect.origin.y - rect.height
                return self.subImage(from: subImageRect)
            } else {
                return nil
            }
        }
        
        switch count {
            
        case 9:
            return NinePartImage(imageParts: imageParts)
            
        case 3:
            return ThreePartImage(imageParts: imageParts, vertical: vertical)
            
        case 1:
            fallthrough
        default:
            return imageParts.last!!
        }
    }
    
    @objc(ninePartImageWithStretchedRect:)
    func ninePartImage(withStretched rect: NSRect) -> NSImage {
        
        let top    = NSRect(x: 0, y: rect.maxY, width: size.width, height: size.height - rect.maxY)
        let middle = NSRect(x: 0, y: rect.minY, width: size.width, height: rect.height)
        let bottom = NSRect(x: 0, y: 0, width: size.width, height: rect.minY)
        
        let left   = NSRect(x: 0, y: 0, width: rect.minX, height: size.height)
        let center = NSRect(x: rect.minX, y: 0, width: rect.width, height: size.height)
        let right  = NSRect(x: rect.maxX, y: 0, width: size.width - rect.maxX, height: size.height)
        
        let parts = [bottom.intersection(left),
                     bottom.intersection(center),
                     bottom.intersection(right),
                     middle.intersection(left),
                     middle.intersection(center),
                     middle.intersection(right),
                     top.intersection(left),
                     top.intersection(center),
                     top.intersection(right)]
        
        return image(fromParts: parts.map { NSStringFromRect($0) as AnyObject }, vertical: false)
    }
    
    // https://gist.github.com/usagimaru/c0a03ef86b5829fb9976b650ec2f1bf4
    @objc(imageWithTintColor:)
    func withTintColor(_ color: NSColor) -> NSImage {
        if !isTemplate {
            return self
        }
        
        let image = self.copy() as! NSImage
        image.lockFocus()
        
        color.set()
        
        let imageRect = NSRect(origin: .zero, size: image.size)
        imageRect.fill(using: .sourceIn)
        
        image.unlockFocus()
        image.isTemplate = false
        
        return image
    }
}
