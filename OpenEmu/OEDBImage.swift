// Copyright (c) 2021, OpenEmu Team
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

extension OEDBImage {
    
    // OELibraryDatabase
    @objc(createImageWithDictionary:)
    class func _createImage(with dictionary: [String : Any]?) -> OEDBImage? {
        guard let dictionary = dictionary else {
            return nil
        }
        
        return createImage(with: dictionary)
    }
}

@objc
@objcMembers
final class OEDBImage: OEDBItem {
    
    static let gameArtworkFormat: NSBitmapImageRep.FileType = .jpeg
    static let gameArtworkProperties: [NSBitmapImageRep.PropertyKey : Any] = [.compressionFactor : 0.9]
    
    // MARK: - CoreDataProperties
    
    @NSManaged var format: Int16
    @NSManaged var height: Float
    @NSManaged var relativePath: String?
    @NSManaged var source: String?
    @NSManaged var width: Float
    @NSManaged var Box: OEDBGame?
    
    // MARK: -
    
    override class var entityName: String { "Image" }
    
    // MARK: -
    
    class func prepareImage(withURLString urlString: String) -> [String : Any]? {
        guard
            let url = URL(string: urlString)
        else { return nil }
        return Self.prepareImage(with: url)
    }
    
    class func prepareImage(with url: URL) -> [String : Any] {
        var result: [String : Any] = ["URL" : url]
        
        if let image = NSImage(contentsOf: url) {
            let tempInfo = prepareImage(with: image)
            
            result.merge(tempInfo) { (old, _) in old }
        }
        
        return result
    }
    
    @objc(prepareImageWithNSImage:) // OEDBGame
    class func prepareImage(with image: NSImage) -> [String : Any] {
        
        var result: [String : Any] = [:]
        
        var imageSize = image.size
        let fileName = UUID().uuidString
        let type = Self.gameArtworkFormat
        let properties = Self.gameArtworkProperties
        
        var imageRep: NSBitmapImageRep?
        var maxArea = 0
        for rep in image.representations {
            if let rep = rep as? NSBitmapImageRep {
                let area = rep.pixelsHigh * rep.pixelsWide
                if area >= maxArea {
                    imageRep = rep
                    maxArea = area
                    
                    imageSize.width = CGFloat(rep.pixelsWide)
                    imageSize.height = CGFloat(rep.pixelsHigh)
                }
            }
        }
        
        if imageRep == nil {
            DLog("No NSBitmapImageRep found, creating one…")
            var proposedRect = NSRect(origin: .zero, size: imageSize)
            if let cgImage = image.cgImage(forProposedRect: &proposedRect, context: nil, hints: nil) {
                imageRep = NSBitmapImageRep(cgImage: cgImage)
            }
        }
        
        guard let imageRep = imageRep else {
            DLog("Could not draw NSImage in NSBitmapimage rep, exiting…")
            return result
        }
        
        let data = imageRep.representation(using: type, properties: properties)
        
        let database = OELibraryDatabase.default!
        let coverFolderURL = database.coverFolderURL
        let imageURL = URL(string: fileName, relativeTo: coverFolderURL)!
        
        do {
            try data?.write(to: imageURL, options: .atomic)
        } catch {
            try? FileManager.default.removeItem(at: imageURL)
            DLog("Failed to write image file! Exiting…")
            return result
        }
        
        result["width"]        = imageSize.width
        result["height"]       = imageSize.height
        result["relativePath"] = imageURL.relativeString
        result["format"]       = type
        
        return result
    }
    
    class func createImage(with dictionary: [String : Any]) -> OEDBImage {
        assert(Thread.isMainThread, "Only call on main thread!")
        return createImage(with: dictionary, in: OELibraryDatabase.default!.mainThreadContext)
    }
    
    class func createImage(with dictionary: [String : Any], in context: NSManagedObjectContext) -> OEDBImage {
        
        let image = OEDBImage.createObject(in: context)
        image.sourceURL = dictionary["URL"] as? URL
        image.width = Float(dictionary["width"] as! CGFloat)
        image.height = Float(dictionary["height"] as! CGFloat)
        
        image.relativePath = dictionary["relativePath"] as? String
        image.format = Int16((dictionary["format"] as! NSBitmapImageRep.FileType).rawValue)
        
        return image
    }
    
    // MARK: -
    
    @objc(convertToFormat:withProperties:)
    func convert(to type: NSBitmapImageRep.FileType, withProperties attributes: [NSBitmapImageRep.PropertyKey : Any]) -> Bool {
        guard
            let image = image,
            let context = managedObjectContext,
            let newURL = write(image, withType: type, withProperties: attributes, in: context)
        else {
            DLog("converting image failed!")
            return false
        }
        
        context.performAndWait {
            if let url = imageURL {
                try? FileManager.default.removeItem(at: url)
            }
            format = Int16(type.rawValue)
            relativePath = newURL.relativeString
            save()
        }
        
        return true
    }
    
    // MARK: -
    
    private func write(_ url: URL, withType type: NSBitmapImageRep.FileType, outSize size: inout NSSize?, in context: NSManagedObjectContext) -> URL? {
        if let data = try? Data(contentsOf: url) {
            return write(data, withType: type, outSize: &size, in: context)
        } else {
            return nil
        }
    }
    
    private func write(_ data: Data, withType type: NSBitmapImageRep.FileType, outSize size: inout NSSize?, in context: NSManagedObjectContext) -> URL? {
        if let image = NSImage(data: data) {
            size = image.size
            return write(image, withType: type, in: context)
        } else {
            return nil
        }
    }
    
    private func write(_ image: NSImage, withType type: NSBitmapImageRep.FileType, in context: NSManagedObjectContext) -> URL? {
        return write(image, withType: type, withProperties: Self.gameArtworkProperties, in: context)
    }
    
    private func write(_ image: NSImage, withType type: NSBitmapImageRep.FileType, withProperties properties: [NSBitmapImageRep.PropertyKey : Any], in context: NSManagedObjectContext) -> URL? {
        
        let imageSize = image.size
        let fileName = UUID().uuidString
        
        var imageRep: NSBitmapImageRep?
        var maxArea = 0
        for rep in image.representations {
            if let rep = rep as? NSBitmapImageRep {
                let area = rep.pixelsHigh * rep.pixelsWide
                if area >= maxArea {
                    imageRep = rep
                    maxArea = area
                }
            }
        }
        
        if imageRep == nil {
            DLog("No NSBitmapImageRep found, creating one…")
            var proposedRect = NSRect(origin: .zero, size: imageSize)
            if let cgImage = image.cgImage(forProposedRect: &proposedRect, context: nil, hints: nil) {
                imageRep = NSBitmapImageRep(cgImage: cgImage)
            }
        }
        
        guard let imageRep = imageRep else {
            DLog("Could not draw NSImage in NSBitmapimage rep, exiting…")
            return nil
        }
        
        let data = imageRep.representation(using: type, properties: properties)
        
        // TODO: get database from context
        let database = OELibraryDatabase.default!
        let coverFolderURL = database.coverFolderURL
        let imageURL = URL(string: fileName, relativeTo: coverFolderURL)!
        
        do {
            try data?.write(to: imageURL, options: .atomic)
        } catch {
            try? FileManager.default.removeItem(at: imageURL)
            DLog("Failed to write image file! Exiting…")
            return nil
        }
        
        return imageURL
    }
    
    // MARK: -
    
    override func prepareForDeletion() {
        if managedObjectContext?.parent == nil,
           let url = imageURL {
            try? FileManager.default.removeItem(at: url)
        }
    }
    
    @objc(UUID)
    var uuid: String? {
        return relativePath
    }
    
    var image: NSImage? {
        if let imageURL = imageURL {
            return NSImage(contentsOf: imageURL)
        } else {
            return nil
        }
    }
    
    var imageURL: URL? {
        if let relativePath = relativePath {
            return libraryDatabase.coverFolderURL.appendingPathComponent(relativePath)
        } else {
            return nil
        }
    }
    
    var sourceURL: URL? {
        get {
            if let source = source {
                return URL(string: source)
            } else {
                return nil
            }
        }
        set {
            source = newValue?.standardizedFileURL.absoluteString
        }
    }
    
    var isLocalImageAvailable: Bool {
        return (try? imageURL?.checkResourceIsReachable()) ?? false
    }
}
