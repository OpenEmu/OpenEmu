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

import Foundation

final class ImageCacheService {
    private var cache = NSCache<NSString, NSImage>()
    
    public static var shared = ImageCacheService()
    
    public func fetchImage(_ url: URL, success: @escaping (NSImage) -> Void) {
        if let img = cache.object(forKey: url.absoluteString as NSString) {
            success(img)
            return
        }
        
        DispatchQueue.global(qos: .userInteractive).async {
            let imageSource = CGImageSourceCreateWithURL(url as CFURL, nil)
            if let imageSource = imageSource {
                guard CGImageSourceGetType(imageSource) != nil else { return }
                if let thumbnail = self.getThumbnailImage(imageSource: imageSource) {
                    let bytes = Int(thumbnail.size.width * thumbnail.size.height * 4)
                    self.cache.setObject(thumbnail, forKey: url.absoluteString as NSString, cost: bytes)
                    DispatchQueue.main.async {
                        success(thumbnail)
                    }
                }
            }
        }
    }
    
    private func getThumbnailImage(imageSource: CGImageSource) -> NSImage? {
        let thumbnailOptions = [
            String(kCGImageSourceCreateThumbnailFromImageIfAbsent): true,
            String(kCGImageSourceThumbnailMaxPixelSize): 320
            ] as [String : Any]
        guard let thumbnailRef = CGImageSourceCreateThumbnailAtIndex(imageSource, 0, thumbnailOptions as CFDictionary) else { return nil}
        return NSImage(cgImage: thumbnailRef, size: .zero)
    }
}
