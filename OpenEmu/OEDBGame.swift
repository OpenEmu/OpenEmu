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

@objc
final class OEDBGame: OEDBItem {
    
    @objc(OEDBGameStatus)
    enum Status: Int16 {
        @objc(OEDBGameStatusOK)
        case ok = 0
        case downloading = 1
        case alert = 2
        case processing = 3
    }
    
    static let displayGameTitleKey = "displayGameTitle"
    
    private var romDownload: Download?
    
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
    
    override class var entityName: String { "Game" }
    
    // MARK: -
    
    class func createGame(withName name: String, andSystem system: OEDBSystem, in context: NSManagedObjectContext) -> OEDBGame {
        
        var game: OEDBGame!
        context.performAndWait {
            game = OEDBGame.createObject(in: context)
            game.name = name
            game.importDate = Date()
            game.system = system
        }
        
        return game
    }
    
    /// Returns the game from the default database that represents the file at `url`.
    @nonobjc
    class func game(withURL url: URL) throws -> OEDBGame? {
        guard let context = OELibraryDatabase.default?.mainThreadContext else { return nil }
        return try OEDBGame.game(withURL: url, in: context)
    }
    
    /// Returns the game from the specified `context` that represents the file at `url`
    @nonobjc
    class func game(withURL url: URL, in context: NSManagedObjectContext) throws -> OEDBGame? {
        var err: Error?
        
        let url = url.standardizedFileURL
        var urlReachable = false
        do {
            urlReachable = try url.checkResourceIsReachable()
        } catch {
            err = error
        }
        
        // TODO: FIX
        var game: OEDBGame?
        
        do {
            if let rom = try OEDBRom.rom(with: url, in: context) {
                game = rom.game
            }
        } catch {
            err = error
        }
        
        if game == nil && urlReachable {
            do {
                let md5 = try FileManager.default.hashFile(at: url)
                if let rom = try OEDBRom.rom(withMD5HashString: md5, in: context) {
                    game = rom.game
                }
            } catch {
                err = error
            }
        }
        
        if !urlReachable {
            game?.status = .alert
        }
        
        if let error = err {
            throw error
        } else {
            return game
        }
    }
    
    // MARK: - Accessors
    
    /// Returns the `Date` the game was last started, or `nil` if the game has never been played.
    var lastPlayed: Date? {
        return roms.sorted {
            guard let d1 = $0.lastPlayed,
                  let d2 = $1.lastPlayed
            else { return false }
            return d1.compare(d2) == .orderedDescending
        }.first?.lastPlayed
    }
    
    var autosaveForLastPlayedRom: OEDBSaveState? {
        return roms.sorted {
            guard let d1 = $0.lastPlayed,
                  let d2 = $1.lastPlayed
            else { return false }
            return d1.compare(d2) == .orderedDescending
        }.first?.autosaveState
    }
    
    var saveStateCount: Int {
        var count = 0
        for rom in roms {
            count += rom.saveStateCount
        }
        return count
    }
    
    var defaultROM: OEDBRom? {
        // TODO: If multiple roms are available we should select one based on version/revision and language
        return roms.first
    }
    
    var playCount: UInt {
        var count: UInt = 0
        for rom in roms {
            count += rom.playCount?.uintValue ?? 0
        }
        return count
    }
    
    var playTime: TimeInterval {
        var time: TimeInterval = 0
        for rom in roms {
            time += rom.playTime?.doubleValue ?? 0
        }
        return time
    }
    
    var displayName: String {
        get {
            if UserDefaults.standard.bool(forKey: OEDBGame.displayGameTitleKey) {
                return gameTitle ?? name
            } else {
                return name
            }
        }
        set {
            if UserDefaults.standard.bool(forKey: OEDBGame.displayGameTitleKey) {
                if gameTitle != nil {
                    gameTitle = newValue
                } else {
                    name = newValue
                }
            } else {
                name = newValue
            }
        }
    }
    
    var cleanDisplayName: String {
        let articlesDictionary = [
            "A "   : 2,
            "An "  : 3,
            "Das " : 4,
            "Der " : 4,
            // "Die " : 4, // Biased since some English titles start with Die
            "Gli " : 4,
            "L'"   : 2,
            "La "  : 3,
            "Las " : 4,
            "Le "  : 3,
            "Les " : 4,
            "Los " : 4,
            "The " : 4,
            "Un "  : 3,
        ]
        
        for (key, nmbr) in articlesDictionary {
            if displayName.hasPrefix(key) {
                return String(displayName.dropFirst(nmbr))
            }
        }
        
        return displayName
    }
    
    var filesAvailable: Bool {
        var result = true
        for rom in roms {
            if !rom.filesAvailable {
                result = false
                break
            }
        }
        
        if status == .downloading || status == .processing {
            return result
        }
        
        if !result {
            status = .alert
        } else if status == .alert {
            status = .ok
        }
        
        return result
    }
    
    // MARK: - Cover Art Database Sync / Info Lookup
    
    func requestCoverDownload() {
        if status == .alert || status == .ok {
            status = .processing
            save()
            libraryDatabase.startOpenVGDBSync()
        }
    }
    
    func cancelCoverDownload() {
        if status == .processing {
            status = .ok
            save()
        }
    }
    
    func requestInfoSync() {
        if status == .alert || status == .ok {
            status = .processing
            save()
            libraryDatabase.startOpenVGDBSync()
        }
    }
    
    // MARK: - ROM Downloading
    
    func requestROMDownload() {
        guard romDownload == nil else { return }
        
        status = .downloading
        
        guard let rom = defaultROM,
              let source = rom.source,
              let url = URL(string: source)
        else {
            DLog("Invalid URL to download!")
            return
        }
        
        let romDownload = Download(url: url)
        self.romDownload = romDownload
        romDownload.completionHandler = { url, error in
            if let url = url, error == nil {
                DLog("Downloaded to \(url)")
                rom.url = url
                
                let result: Bool
                do {
                    try result = rom.consolidateFiles()
                } catch {
                    DLog("\(error)")
                    result = false
                }
                if !result {
                    rom.url = nil
                }
                rom.save()
            }
            else {
                DLog("ROM download failed!")
                if let error = error {
                    DLog("\(error)")
                }
            }
            
            self.status = .ok
            self.romDownload = nil
            self.save()
        }
        
        romDownload.start()
    }
    
    func cancelROMDownload() {
        romDownload?.cancel()
        romDownload = nil
        status = .ok
        save()
    }
    
    // MARK: -
    
    @objc(setBoxImageByImage:)
    func setBoxImage(image: NSImage) {
        let dictionary = OEDBImage.prepareImage(with: image)
        let context = managedObjectContext
        context?.performAndWait {
            if let currentImage = boxImage {
                context?.delete(currentImage)
            }
            
            boxImage = OEDBImage.createImage(with: dictionary)
        }
    }
    
    @objc(setBoxImageByURL:)
    func setBoxImage(url: URL) {
        let url = url.standardizedFileURL.absoluteURL
        guard let dictionary = OEDBImage.prepareImage(with: url)
        else { return }
        
        let context = managedObjectContext
        context?.performAndWait {
            if let currentImage = boxImage {
                context?.delete(currentImage)
            }
            
            boxImage = OEDBImage.createImage(with: dictionary)
        }
    }
    
    // MARK: -
    
    override func awakeFromFetch() {
        if status == .downloading {
            status = .ok
        }
    }
    
    override func prepareForDeletion() {
        boxImage?.delete()
        
        romDownload?.cancel()
        romDownload = nil
    }
    
    @objc(deleteByMovingFile:keepSaveStates:)
    func delete(moveToTrash: Bool, keepSaveStates: Bool) {
        while let rom = roms.first {
            rom.delete(moveToTrash: moveToTrash, keepSaveStates: keepSaveStates)
            roms.remove(rom)
        }
        managedObjectContext?.delete(self)
    }
}

// MARK: - NSPasteboardWriting

extension OEDBGame: NSPasteboardWriting {
    
    func writableTypes(for pasteboard: NSPasteboard) -> [NSPasteboard.PasteboardType] {
        return [.game, .fileURL]
    }
    
    func pasteboardPropertyList(forType type: NSPasteboard.PasteboardType) -> Any? {
        switch type {
        case .game:
            return permanentIDURI.absoluteString
        case .fileURL:
            let url = defaultROM?.url?.absoluteURL as NSURL?
            return url?.pasteboardPropertyList(forType: .fileURL)
        default:
            return nil
        }
    }
}

// MARK: - NSPasteboardReading
/*
extension OEDBGame: NSPasteboardReading {
    
    static func readableTypes(for pasteboard: NSPasteboard) -> [NSPasteboard.PasteboardType] {
        return [.game]
    }
    
    static func readingOptions(forType type: NSPasteboard.PasteboardType, pasteboard: NSPasteboard) -> NSPasteboard.ReadingOptions {
        return .asString
    }
    
    convenience init?(pasteboardPropertyList propertyList: Any, ofType type: NSPasteboard.PasteboardType) {
        if type == .game,
           let context = OELibraryDatabase.default?.mainThreadContext,
           let propertyList = propertyList as? String,
           let uri = URL(string: propertyList) {
            //return OEDBGame.object(withURI: uri, in: context)
        }
        return nil
    }
}
*/
// MARK: - OECoverGridDataSourceItem

extension OEDBGame: OECoverGridDataSourceItem {
    
    func gridStatus() -> Int {
        return Int(status.rawValue)
    }
    
    func gridRating() -> UInt {
        return rating?.uintValue ?? 0
    }
    
    func setGridRating(_ newRating: UInt) {
        rating = newRating as NSNumber
        try? managedObjectContext?.save()
    }
    
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
    
    private class func artworkPlaceholder(aspectRatio ratio: CGFloat) -> NSImage {
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
    
    override func imageUID() -> String! {
        if let image = boxImage, image.isLocalImageAvailable, image.relativePath != nil {
            return image.uuid
        } else {
            return ":MissingArtwork(\(system?.coverAspectRatio ?? 0))"
        }
    }
    
    override func imageRepresentationType() -> String! {
        if let image = boxImage, image.isLocalImageAvailable, image.relativePath != nil {
            return IKImageBrowserNSURLRepresentationType
        } else {
            return IKImageBrowserNSImageRepresentationType
        }
    }
    
    override func imageRepresentation() -> Any! {
        if let image = boxImage, image.isLocalImageAvailable, image.relativePath != nil {
            return image.imageURL
        } else {
            return Self.artworkPlaceholder(aspectRatio: system?.coverAspectRatio ?? 0)
        }
    }
    
    override func imageTitle() -> String! {
        return displayName
    }
    
    override func imageSubtitle() -> String! {
        return nil
    }
}

// MARK: - OEListViewDataSourceItem

extension OEDBGame: OEListViewDataSourceItem {
    
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
    
    func listViewRating() -> NSNumber! {
        return rating
    }
    
    func listViewTitle() -> String! {
        return displayName
    }
    
    func listViewLastPlayed() -> String! {
        if let lastPlayed = lastPlayed {
            return Self.listViewDateFormatter.string(from: lastPlayed)
        } else {
            return ""
        }
    }
    
    func listViewConsoleName() -> String! {
        return NSLocalizedString(system?.name ?? "", comment: "")
    }
    
    func listViewSaveStateCount() -> NSNumber! {
        return saveStateCount > 0 ? saveStateCount as NSNumber : nil
    }
    
    func listViewPlayCount() -> NSNumber! {
        return playCount > 0 ? playCount as NSNumber : nil
    }
    
    func listViewPlayTime() -> String! {
        return playTime > 0 ? localizedStringFromElapsedTime(playTime) : ""
    }
    
    func setListViewRating(_ newRating: NSNumber!) {
        rating = newRating as NSNumber
        try? managedObjectContext?.save()
    }
    
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
@available(macOS 11.0, *)
extension OEDBGame {
    
    func dump(prefix: String = "---") {
        Logger.library.debug("\(prefix) Beginning of game dump")
        
        Logger.library.debug("\(prefix) Game name is \(self.name)")
        Logger.library.debug("\(prefix) title is \(self.gameTitle ?? "nil")")
        Logger.library.debug("\(prefix) rating is \(self.rating?.description ?? "nil")")
        Logger.library.debug("\(prefix) description is \(self.gameDescription ?? "nil")")
        Logger.library.debug("\(prefix) import date is \(self.importDate?.description ?? "nil")")
        Logger.library.debug("\(prefix) last info sync is \(self.lastInfoSync?.description ?? "nil")")
        Logger.library.debug("\(prefix) last played is \(self.lastPlayed?.description ?? "nil")")
        Logger.library.debug("\(prefix) status is \(self.status.rawValue)")
        
        Logger.library.debug("\(prefix) Number of ROMs for this game is \(self.roms.count)")
        
        roms.forEach { $0.dump(prefix: prefix + "-----") }
        
        Logger.library.debug("\(prefix) End of game dump\n\n")
    }
}
#endif
