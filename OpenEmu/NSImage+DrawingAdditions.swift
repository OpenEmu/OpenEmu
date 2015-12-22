/*
 Copyright (c) 2015, OpenEmu Team

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

class ThreePartImage: NSImage {
    
    /// Array of the three different parts.
    var parts: [NSImage?]!
    
    /// Image should be rendered vertically.
    var vertical = false
    
    convenience init(imageParts: [NSImage?], vertical: Bool) {
        
        self.init()
        
        self.parts = imageParts
        self.vertical = vertical
        
        let start  = parts[0]?.size ?? NSZeroSize
        let center = parts[1]?.size ?? NSZeroSize
        let end    = parts[2]?.size ?? NSZeroSize
        
        var size = NSZeroSize
        if vertical {
            size.width = max(max(start.width, center.width), end.width)
            size.height = start.height + center.height + end.height
        } else {
            size.width = start.width + center.width + end.width
            size.height = max(max(start.height, center.height), end.height)
        }
        
        self.size = size
    }
    
    override func drawInRect(rect: NSRect,
        fromRect: NSRect,
        operation op: NSCompositingOperation,
        fraction delta: CGFloat) {
            
        drawInRect(rect,
            fromRect: fromRect,
            operation: op,
            fraction: delta,
            respectFlipped: false,
            hints: nil)
    }
    
    override func drawInRect(dstSpacePortionRect: NSRect,
        fromRect srcSpacePortionRect: NSRect,
        operation op: NSCompositingOperation,
        fraction requestedAlpha: CGFloat,
        respectFlipped respectContextIsFlipped: Bool,
        hints: [String : AnyObject]?) {
            
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
                NSGraphicsContext.currentContext()?.flipped ?? false)
    }
}

class NinePartImage: NSImage {
    
    /// Array of the nine different parts.
    var parts: [NSImage?]!
    
    convenience init(imageParts: [NSImage?]) {
        
        self.init()
        
        self.parts = imageParts
        
        let topLeft      = parts[0]?.size ?? NSZeroSize
        let topCenter    = parts[1]?.size ?? NSZeroSize
        let topRight     = parts[2]?.size ?? NSZeroSize
        let leftEdge     = parts[3]?.size ?? NSZeroSize
        let centerFill   = parts[4]?.size ?? NSZeroSize
        let rightEdge    = parts[5]?.size ?? NSZeroSize
        let bottomLeft   = parts[6]?.size ?? NSZeroSize
        let bottomCenter = parts[7]?.size ?? NSZeroSize
        let bottomRight  = parts[8]?.size ?? NSZeroSize
        
        let width1  = topLeft.width + topCenter.width + topRight.width
        let width2  = leftEdge.width + centerFill.width + rightEdge.width
        let width3  = bottomLeft.width + bottomCenter.width + bottomRight.width
        
        let height1 = topLeft.height + leftEdge.height + bottomLeft.height
        let height2 = topCenter.height + centerFill.height + bottomCenter.height
        let height3 = topRight.height + rightEdge.height + bottomRight.height
        
        size = NSSize(width: max(max(width1, width2), width3),
                     height: max(max(height1, height2), height3))
    }
    
    override func drawInRect(rect: NSRect,
        fromRect: NSRect,
        operation op: NSCompositingOperation,
        fraction delta: CGFloat) {
        
            drawInRect(rect,
                fromRect: fromRect,
                operation: op,
                fraction: delta,
                respectFlipped: false,
                hints: nil)
    }
    
    override func drawInRect(dstSpacePortionRect: NSRect,
        fromRect srcSpacePortionRect: NSRect,
        operation op: NSCompositingOperation,
        fraction requestedAlpha: CGFloat,
        respectFlipped respectContextIsFlipped: Bool,
        hints: [String : AnyObject]?) {
        
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
                respectContextIsFlipped && NSGraphicsContext.currentContext()?.flipped ?? false)
    }
}

extension NSImage {
    
    func subImageFromRect(rect: NSRect) -> NSImage {
        
        return NSImage(size: rect.size, flipped: false) { [unowned self] dstRect in
            self.drawInRect(dstRect,
                fromRect: rect,
                operation: .CompositeSourceOver,
                fraction: 1)

            return true
        }
    }
    
    func imageFromParts(parts: [AnyObject], vertical: Bool) -> NSImage {
        
        guard !parts.isEmpty else {
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
                return self.subImageFromRect(subImageRect)
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
    
    func ninePartImageWithStretchedRect(rect: NSRect) -> NSImage {
        
        let top    = NSRect(x: 0, y: rect.maxY, width: size.width, height: size.height - rect.maxY)
        let middle = NSRect(x: 0, y: rect.minY, width: size.width, height: rect.height)
        let bottom = NSRect(x: 0, y: 0, width: size.width, height: rect.minY)
        
        let left   = NSRect(x: 0, y: 0, width: rect.minX, height: size.height)
        let center = NSRect(x: rect.minX, y: 0, width: rect.width, height: size.height)
        let right  = NSRect(x: rect.maxX, y: 0, width: size.width - rect.maxX, height: size.height)
        
        let parts = [bottom.intersect(left),
                     bottom.intersect(center),
                     bottom.intersect(right),
                     middle.intersect(left),
                     middle.intersect(center),
                     middle.intersect(right),
                     top.intersect(left),
                     top.intersect(center),
                     top.intersect(right)]
        
        return imageFromParts(parts.map { String($0) }, vertical: false)
    }
}
