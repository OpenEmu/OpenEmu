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
    
    private static let queue = DispatchQueue(label: "org.openemu.OpenEmu.imageLoader", qos: .userInteractive, attributes: [.concurrent], autoreleaseFrequency: .inherit, target: nil)
    
    public static var shared = ImageCacheService()
    
    @available(macOS, deprecated: 10.15)
    public func fetchImage(_ url: URL, completionHandler: @escaping (NSImage) -> Void) {
        if let img = cache.object(forKey: url.absoluteString as NSString) {
            completionHandler(img)
            return
        }
        
        Self.queue.async {
            var res: NSImage? = nil
            defer {
                let res = res
                DispatchQueue.main.async {
                    completionHandler(res ?? NSImage(named: NSImage.cautionName)!)
                }
            }
            
            let imageSource = CGImageSourceCreateWithURL(url as CFURL, nil)
            if let imageSource {
                guard CGImageSourceGetType(imageSource) != nil else { return }
                if let thumbnail = self.getThumbnailImage(imageSource: imageSource) {
                    res = thumbnail
                    let bytes = Int(thumbnail.size.width * thumbnail.size.height * 4)
                    self.cache.setObject(thumbnail, forKey: url.absoluteString as NSString, cost: bytes)
                }
            }
        }
    }
    
    @available(macOS 10.15, *)
    public func fetchImage(_ url: URL) async -> NSImage {
        if let img = cache.object(forKey: url.absoluteString as NSString) {
            return img
        }
        
        return await withCheckedContinuation { continuation in
            Self.queue.async {
                var res: NSImage? = nil
                defer {
                    let res = res
                    continuation.resume(returning: res ?? NSImage(named: NSImage.cautionName)!)
                }
                
                let imageSource = CGImageSourceCreateWithURL(url as CFURL, nil)
                if let imageSource {
                    guard CGImageSourceGetType(imageSource) != nil else { return }
                    if let thumbnail = self.getThumbnailImage(imageSource: imageSource) {
                        res = thumbnail
                        let bytes = Int(thumbnail.size.width * thumbnail.size.height * 4)
                        self.cache.setObject(thumbnail, forKey: url.absoluteString as NSString, cost: bytes)
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
