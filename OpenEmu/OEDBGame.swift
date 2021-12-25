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

extension NSPasteboard.PasteboardType {
    static let game = NSPasteboard.PasteboardType("org.openemu.game")
}

extension OEDBGame {
    // OEImportOperation
    @objc var mutableCollections: NSMutableSet {
        mutableSetValue(forKeyPath: #keyPath(collections))
    }
    // OEDBGame
    @objc var mutableRoms: NSMutableSet {
        mutableSetValue(forKeyPath: #keyPath(roms))
    }
}

@objc
extension OEDBGame {
    
    @objc(OEDBGameStatus)
    enum Status: Int16 {
        @objc(OEDBGameStatusOK) case ok
        case downloading, alert, processing
    }
    
    // MARK: - CoreDataProperties
    
    @NSManaged var gameDescription: String?
    @NSManaged var gameTitle: String?
    @NSManaged var importDate: Date?
    @NSManaged var lastInfoSync: Date?
    @NSManaged var name: String
    @NSManaged var rating: NSNumber?
    // @NSManaged var status: NSNumber?
    @NSManaged var boxImage: OEDBImage?
    @NSManaged var collections: Set<OEDBCollection>
    @NSManaged var roms: Set<OEDBRom>
    @NSManaged var system: OEDBSystem?
    
    var status: Status {
        get {
            let key = "status"
            willAccessValue(forKey: key)
            let status = primitiveValue(forKey: key) as? Int16 ?? 0
            didAccessValue(forKey: key)
            return Status(rawValue: status)!
        }
        set {
            let key = "status"
            willChangeValue(forKey: key)
            setPrimitiveValue(newValue.rawValue as NSNumber, forKey: key)
            didChangeValue(forKey: key)
        }
    }
    
    // MARK: -
    
    open override class var entityName: String { "Game" }
}

// MARK: - NSPasteboardWriting

extension OEDBGame: NSPasteboardWriting {
    
    public func writableTypes(for pasteboard: NSPasteboard) -> [NSPasteboard.PasteboardType] {
        return [.game, .fileURL]
    }
    
    public func pasteboardPropertyList(forType type: NSPasteboard.PasteboardType) -> Any? {
        switch type {
        case .game:
            return permanentIDURI.absoluteString
        case .fileURL:
            let url = defaultROM.url?.absoluteURL as NSURL?
            return url?.pasteboardPropertyList(forType: .fileURL)
        default:
            return nil
        }
    }
}

// MARK: - NSPasteboardReading
/*
extension OEDBGame: NSPasteboardReading {
    
    public static func readableTypes(for pasteboard: NSPasteboard) -> [NSPasteboard.PasteboardType] {
        return [.game]
    }
    
    public static func readingOptions(forType type: NSPasteboard.PasteboardType, pasteboard: NSPasteboard) -> NSPasteboard.ReadingOptions {
        return .asString
    }
    
    public required init?(pasteboardPropertyList propertyList: Any, ofType type: NSPasteboard.PasteboardType) {
        
    }
}
*/

// MARK: - OECoverGridDataSourceItem

extension OEDBGame: OECoverGridDataSourceItem {
    
    public
    func gridStatus() -> Int {
        return Int(status.rawValue)
    }
    
    public
    func gridRating() -> UInt {
        return rating?.uintValue ?? 0
    }
    
    public
    func setGridRating(_ newRating: UInt) {
        rating = newRating as NSNumber
        try? managedObjectContext?.save()
    }
    
    public
    func setGridTitle(_ title: String!) {
        displayName = title
        try? managedObjectContext?.save()
    }
    
    var hasImage: Bool {
        return boxImage != nil
    }
}

// MARK: - IKImageBrowserItem

extension OEDBGame {
    
    private static let artworkPlaceholderCache: NSCache<NSString, NSImage> = {
        let cache = NSCache<NSString, NSImage>()
        cache.countLimit = 20
        return cache
    }()
    
    private static func artworkPlaceholder(aspectRatio ratio: CGFloat) -> NSImage {
        let key = "\(ratio)" as NSString
        if let image = artworkPlaceholderCache.object(forKey: key) {
            return image
        } else {
            let image = NSImage()
            image.size = NSSize(width: 300, height: 300 * ratio)
            artworkPlaceholderCache.setObject(image, forKey: key)
            image.cacheMode = .bySize
            return image
        }
    }
    
    open
    override func imageUID() -> String! {
        if let image = boxImage, image.isLocalImageAvailable, image.relativePath != nil {
            return image.uuid
        } else {
            return ":MissingArtwork(\(system?.coverAspectRatio ?? 0))"
        }
    }
    
    open
    override func imageRepresentationType() -> String! {
        if let image = boxImage, image.isLocalImageAvailable, image.relativePath != nil {
            return IKImageBrowserNSURLRepresentationType
        } else {
            return IKImageBrowserNSImageRepresentationType
        }
    }
    
    open
    override func imageRepresentation() -> Any! {
        if let image = boxImage, image.isLocalImageAvailable, image.relativePath != nil {
            return image.imageURL
        } else {
            return Self.artworkPlaceholder(aspectRatio: system?.coverAspectRatio ?? 0)
        }
    }
    
    open
    override func imageTitle() -> String! {
        return displayName
    }
    
    open
    override func imageSubtitle() -> String! {
        return nil
    }
}

// MARK: - OEListViewDataSourceItem

extension OEDBGame: OEListViewDataSourceItem {
    
    public
    func listViewStatus() -> String! {
        if hasOpenDocument {
            return "list_indicator_playing"
        } else if status == .alert {
            return "list_indicator_missing"
        } else if lastPlayed == nil {
            return "list_indicator_unplayed"
        } else {
            return nil
        }
    }
    
    public
    func listViewRating() -> NSNumber! {
        return rating
    }
    
    public
    func listViewTitle() -> String! {
        return displayName
    }
    
    public
    func listViewLastPlayed() -> String! {
        if let lastPlayed = lastPlayed {
            return Self.listViewDateFormatter.string(from: lastPlayed)
        } else {
            return ""
        }
    }
    
    public
    func listViewConsoleName() -> String! {
        return NSLocalizedString(system?.name ?? "", comment: "")
    }
    
    public
    func listViewSaveStateCount() -> NSNumber! {
        return saveStateCount.uintValue > 0 ? saveStateCount : nil
    }
    
    public
    func listViewPlayCount() -> NSNumber! {
        return playCount.uintValue > 0 ? playCount : nil
    }
    
    public
    func listViewPlayTime() -> String! {
        return playTime.doubleValue > 0 ? localizedStringFromElapsedTime(playTime.doubleValue) : ""
    }
    
    public
    func setListViewRating(_ newRating: NSNumber!) {
        rating = newRating as NSNumber
        try? managedObjectContext?.save()
    }
    
    public
    func setListViewTitle(_ title: String!) {
        displayName = title
    }
    
    private var hasOpenDocument: Bool {
        let doc = NSDocumentController.shared.documents.first {
            if let gameDoc = $0 as? OEGameDocument {
                return gameDoc.rom.game == self
            } else {
                return false
            }
        }
        
        return doc != nil
    }
    
    private static let listViewDateFormatter: DateFormatter = {
        let formatter = DateFormatter()
        formatter.dateStyle = .medium
        formatter.timeStyle = .none
        return formatter
    }()
    
    private static let listViewDCF: DateComponentsFormatter = {
        var dcf = DateComponentsFormatter()
        dcf.unitsStyle = .abbreviated
        dcf.allowedUnits = [.second, .minute, .hour, .day]
        return dcf
    }()
    
    private func localizedStringFromElapsedTime(_ timeInterval: TimeInterval) -> String? {
        return Self.listViewDCF.string(from: timeInterval)
    }
}

// MARK: - Debug

#if DEBUG
extension OEDBGame {
    
    func dump(prefix: String = "---") {
        NSLog("\(prefix) Beginning of game dump")
        
        NSLog("\(prefix) Game name is \(name)")
        NSLog("\(prefix) title is \(gameTitle ?? "nil")")
        NSLog("\(prefix) rating is \(rating?.description ?? "nil")")
        NSLog("\(prefix) description is \(gameDescription ?? "nil")")
        NSLog("\(prefix) import date is \(importDate?.description  ?? "nil")")
        NSLog("\(prefix) last info sync is \(lastInfoSync?.description ?? "nil")")
        NSLog("\(prefix) last played is \(lastPlayed?.description ?? "nil")")
        NSLog("\(prefix) status is \(status)")
        
        NSLog("\(prefix) Number of ROMs for this game is \(roms.count)")
        
        roms.forEach { $0.dump(prefix: prefix + "-----") }
        
        NSLog("\(prefix) End of game dump\n\n")
    }
}
#endif
