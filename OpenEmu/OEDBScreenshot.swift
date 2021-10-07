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

@objc
@objcMembers
final class OEDBScreenshot: OEDBItem {
    
    static let importRequiredKey = "OEDBScreenshotImportRequired"
    
    // MARK: - CoreDataProperties
    
    @NSManaged var location: String
    @NSManaged var name: String?
    @NSManaged var timestamp: Date?
    // @NSManaged var userDescription: String?
    @NSManaged var rom: OEDBRom?
    
    @objc(URL)
    var url: URL {
        get {
            let screenshotDirectory = libraryDatabase.screenshotFolderURL
            return URL(string: location, relativeTo: screenshotDirectory)!
        }
        set {
            let screenshotDirectory = libraryDatabase.screenshotFolderURL
            location = (newValue as NSURL).url(relativeTo: screenshotDirectory)!.relativeString
        }
    }
    
    var screenshotURL: URL {
        return url
    }
    
    // MARK: -
    
    override class var entityName: String { "Screenshot" }
    
    // MARK: -
    
    @objc(createObjectInContext:forROM:withFile:)
    class func createObject(in context: NSManagedObjectContext, for rom: OEDBRom, with url: URL) -> Self? {
        
        if let isReachable = try? url.checkResourceIsReachable(),
           isReachable {
            let name = url.deletingPathExtension().lastPathComponent
            let screenshot = OEDBScreenshot.createObject(in: context)
            screenshot.url = url
            screenshot.rom = rom
            screenshot.timestamp = Date()
            screenshot.name = name
            
            screenshot.updateFile()
            screenshot.save()
            return screenshot as? Self
        }
        
        return nil
    }
    
    override func prepareForDeletion() {
        try? FileManager.default.trashItem(at: url, resultingItemURL: nil)
    }
    
    func updateFile() {
        let database = libraryDatabase
        let screenshotDirectory = database.screenshotFolderURL
        let fileName = NSURL.validFilename(from: name ?? "")
        let fileExtension = "png"
        var targetURL = screenshotDirectory.appendingPathComponent("\(fileName).\(fileExtension)").standardizedFileURL
        let sourceURL = url
        
        if targetURL == sourceURL {
            return
        }
        
        if let isReachable = try? targetURL.checkResourceIsReachable(),
           isReachable {
            targetURL = (targetURL as NSURL).uniqueURL { triesCount in
                return screenshotDirectory.appendingPathComponent("\(fileName) \(triesCount).\(fileExtension)") as NSURL
            } as URL
        }
        
        do {
            try FileManager.default.moveItem(at: sourceURL, to: targetURL)
            url = targetURL
        } catch {
            DLog("\(error)")
        }
    }
}

// MARK: - NSPasteboardWriting

extension OEDBScreenshot: NSPasteboardWriting {
    
    func writableTypes(for pasteboard: NSPasteboard) -> [NSPasteboard.PasteboardType] {
        return [.fileURL, kUTTypeImage as NSPasteboard.PasteboardType]
    }
    
    func pasteboardPropertyList(forType type: NSPasteboard.PasteboardType) -> Any? {
        let url = url.absoluteURL
        if type == .fileURL {
            return (url as NSURL).pasteboardPropertyList(forType: type)
        } else if type.rawValue == kUTTypeImage as String {
            let image = NSImage(contentsOf: url)
            return image?.pasteboardPropertyList(forType: type)
        }
        return nil
    }
}

// MARK: - QLPreviewItem

extension OEDBScreenshot: QLPreviewItem {
    
    var previewItemURL: URL! {
        return url
    }
    
    var previewItemTitle: String! {
        return name
    }
}
