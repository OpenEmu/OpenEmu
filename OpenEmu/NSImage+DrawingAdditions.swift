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

extension NSImage {
    
    @objc(subImageFromRect:)
    func subImage(from rect: NSRect) -> NSImage {
        if rect.origin == .zero && rect.size == self.size {
            return self.copy() as! NSImage
        }
        
        let newImage = NSImage(size: rect.size)
        let flippedRect = NSRect(x: rect.origin.x, y: (self.size.height-(rect.origin.y+rect.size.height)), width: rect.size.width, height: rect.size.height)
        for representation in self.representations {
            if representation.pixelsWide == 0 && representation.pixelsHigh == 0 {
                continue
            }
            let cgrep = representation.cgImage(forProposedRect: nil, context: nil, hints: nil)
            let xscale = CGFloat(representation.pixelsHigh) / representation.size.height
            let yscale = CGFloat(representation.pixelsWide) / representation.size.width
            if let croppedrep = cgrep?.cropping(to: CGRect(x: flippedRect.origin.x * xscale, y: flippedRect.origin.y * yscale, width: flippedRect.size.width * xscale, height: flippedRect.size.height * yscale)) {
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
    
    // MARK: - Missing Artwork Image Generation
    
    private static let missingArtworkImageCache: NSCache<NSString, NSImage> = {
        let cache = NSCache<NSString, NSImage>()
        cache.countLimit = 25
        return cache
    }()
    
    @objc
    static func missingArtworkImage(size: NSSize) -> NSImage? {
        guard size != .zero else { return nil }
        
        let key = "\(size)" as NSString
        
        if let missingArtwork = missingArtworkImageCache.object(forKey: key) {
            return missingArtwork
        }
        
        let missingArtwork = NSImage(size: size)
        missingArtwork.lockFocus()
        
        let currentContext = NSGraphicsContext.current
        currentContext?.saveGraphicsState()
        currentContext?.shouldAntialias = false
        
        let scanLineImage = NSImage(named: "missing_artwork")!
        let scanLineImageSize = scanLineImage.size
        
        var scanLineRect = CGRect(x: 0, y: 0, width: size.width, height: scanLineImageSize.height)
        
        for y in stride(from: 0, to: size.height, by: scanLineImageSize.height) {
            scanLineRect.origin.y = y
            scanLineImage.draw(in: scanLineRect, from: .zero, operation: .copy, fraction: 1)
        }
        
        currentContext?.restoreGraphicsState()
        missingArtwork.unlockFocus()
        
        // Cache the image for later use
        missingArtworkImageCache.setObject(missingArtwork, forKey: key, cost: Int(size.width * size.height))
        
        return missingArtwork
    }
}
