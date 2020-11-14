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

extension NSImageView {
    var imageRatio: NSSize {
        guard
            let imageSize = image?.size,
            let frameRect = cell?.drawingRect(forBounds: bounds)
            else { return NSZeroSize }
        
        let frameSize = frameRect.size
        var ratio = NSZeroSize
        let ratioX = frameSize.width / imageSize.width
        let ratioY = frameSize.height / imageSize.height
        
        switch imageScaling {
        case .scaleProportionallyDown:
            let scale = min(min(ratioX, ratioY), 1.0)
            ratio = NSSize(width: scale, height: scale)
            
        case .scaleProportionallyUpOrDown:
            let scale = min(ratioX, ratioY);
            ratio = NSSize(width: scale, height: scale)
            
        case .scaleAxesIndependently:
            ratio = NSSize(width: ratioX, height: ratioY)
            
        default:
            ratio = NSSize(width: 1.0, height: 1.0)
        }
        
        return ratio
    }
    
    
    /// The image's bounds, which expresses its location and size in its own coordinate system after
    /// factoring imageScaling and imageAlignment properties.
    var croppedBounds: NSRect {
        guard
            let imageSize = image?.size,
            let frameRect = cell?.drawingRect(forBounds: bounds)
            else { return NSZeroRect }

        let frameSize = frameRect.size
        let ratio     = imageRatio
        
        let scaledSize = NSSize(width: imageSize.width * ratio.width, height: imageSize.height * ratio.height);
        
        let Xmin = CGFloat(0.0);
        let Xmax = frameSize.width - scaledSize.width;
        let verticalCenter = (frameSize.width - scaledSize.width) / 2;
        let Ymin = CGFloat(0.0);
        let Ymax = frameSize.height - scaledSize.height;
        let horizontalCenter = (frameSize.height - scaledSize.height) / 2;
        
        var scaledRect = NSZeroRect
        switch imageAlignment {
        case .alignCenter:
            scaledRect = NSRect(x: verticalCenter, y: horizontalCenter, width: scaledSize.width, height: scaledSize.height)
            
        case .alignLeft:
            scaledRect = NSRect(x: Xmin, y: horizontalCenter, width: scaledSize.width, height: scaledSize.height)
            
        case .alignRight:
            scaledRect = NSRect(x: Xmax, y: horizontalCenter, width: scaledSize.width, height: scaledSize.height)
            
        case .alignTop:
            scaledRect = NSRect(x: verticalCenter, y: Ymax, width: scaledSize.width, height: scaledSize.height)
            
        case .alignTopLeft:
            scaledRect = NSRect(x: Xmin, y: Ymax, width: scaledSize.width, height: scaledSize.height)
            
        case .alignTopRight:
            scaledRect = NSRect(x: Xmax, y: Ymax, width: scaledSize.width, height: scaledSize.height)
            
        case .alignBottom:
            scaledRect = NSRect(x: verticalCenter, y: Ymin, width: scaledSize.width, height: scaledSize.height)
            
        case .alignBottomLeft:
            scaledRect = NSRect(x: Xmin, y: Ymin, width: scaledSize.width, height: scaledSize.height)
            
        case .alignBottomRight:
            scaledRect = NSRect(x: Xmax, y: Ymin, width: scaledSize.width, height: scaledSize.height)
            
        @unknown default:
            fatalError("unknown value")
        }
        
        return scaledRect.offsetBy(dx: frameRect.origin.x, dy: frameRect.origin.y)
    }
}
