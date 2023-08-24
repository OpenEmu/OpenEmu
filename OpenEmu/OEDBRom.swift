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
import OpenEmuSystem

extension OEDBRom {
    
    // OEGameCollectionViewController
    @objc(consolidateFilesWithError:)
    func _consolidateFiles(error: NSErrorPointer) -> Bool {
        do {
            return try consolidateFiles()
        } catch let err as NSError {
            error?.pointee = err
            return false
        }
    }
}

@objc
final class OEDBRom: OEDBItem {
    
    // MARK: - CoreDataProperties
    
    @NSManaged var archiveFileIndex: NSNumber?
    // @NSManaged var crc32: String?
    // @NSManaged var favorite: NSNumber?
    @NSManaged var fileName: String?
    @NSManaged var fileSize: NSNumber?
    @NSManaged var header: String?
    @NSManaged var lastPlayed: Date?
    @NSManaged var location: String?
    @NSManaged var md5: String?
    // @NSManaged var openVGDB: NSNumber?
    @NSManaged var playCount: NSNumber?
    @NSManaged var playTime: NSNumber?
    @NSManaged var serial: String?
    @NSManaged var source: String?
    @NSManaged var game: OEDBGame?
    @NSManaged var saveStates: Set<OEDBSaveState>
    @NSManaged var screenShots: Set<OEDBScreenshot>
    
    // MARK: -
    
    override class var entityName: String { "ROM" }
    
    // MARK: -
    
    @nonobjc
    class func rom(with url: URL, in context: NSManagedObjectContext) throws -> OEDBRom? {
        guard let library = context.libraryDatabase,
              let romFolderURL = library.romsFolderURL,
              let url = url.url(relativeTo: romFolderURL)
        else { return nil }
        
        let predicate = NSPredicate(format: "location == %@", url.relativeString)
        let fetchRequest = Self.fetchRequest()
        
        fetchRequest.fetchLimit = 1
        fetchRequest.includesPendingChanges = true
        fetchRequest.predicate = predicate
        
        return try context.fetch(fetchRequest).first as? OEDBRom
    }
    
    @nonobjc
    class func rom(withMD5HashString md5Hash: String, in context: NSManagedObjectContext) throws -> OEDBRom? {
        
        let predicate = NSPredicate(format: "md5 == %@", md5Hash.lowercased())
        let fetchRequest = Self.fetchRequest()
        
        fetchRequest.fetchLimit = 1
        fetchRequest.includesPendingChanges = true
        fetchRequest.predicate = predicate
        
        return try context.fetch(fetchRequest).first as? OEDBRom
    }
    
    // MARK: - Accessors
    
    @objc(URL)
    var url: URL? {
        get {
            if let romFolderURL = libraryDatabase.romsFolderURL,
               let location = location {
                return URL(string: location, relativeTo: romFolderURL)
            }
            return nil
        }
        set {
            if let romFolderURL = libraryDatabase.romsFolderURL {
                location = newValue?.url(relativeTo: romFolderURL)?.relativeString
            }
        }
    }
    
    var sourceURL: URL? {
        get {
            if let source = source {
                return URL(string: source)
            }
            return nil
        }
        set {
            source = newValue?.absoluteString
        }
    }
    
    /// Returns MD5 hash for rom. Calculates it if necessary so the method can take a long time to return, and might return `nil` if hash is not in DB and can not be calculated.
    var md5Hash: String? {
        var hash = md5
        if hash == nil {
            calculateHashes()
            hash = md5HashIfAvailable
        }
        return hash
    }
    
    /// Returns MD5 hash for rom if one was calculated before.
    var md5HashIfAvailable: String? {
        return md5
    }
    
    private func calculateHashes() {
        guard let url = url else { return }
        
        do {
            let isReachable = try url.checkResourceIsReachable()
            if !isReachable {
                // TODO: mark self as file missing
                return
            }
        } catch {
            // TODO: mark self as file missing
            DLog("\(error)")
            return
        }
        
        var md5Hash: String
        do {
            md5Hash = try FileManager.default.hashFile(at: url)
        } catch {
            // TODO: mark self as file missing
            DLog("\(error)")
            return
        }
        
        md5 = md5Hash.lowercased()
    }
    
    // MARK: -
    
    /// Returns count of save states.
    var saveStateCount: Int {
        return saveStates.count
    }
    
    var normalSaveStates: [OEDBSaveState] {
        let predicate = NSPredicate(format: "NOT (name beginswith[c] %@)", OEDBSaveState.specialNamePrefix)
        
        return saveStates.filter { predicate.evaluate(with: $0) }
    }
    
    @objc(normalSaveStatesByTimestampAscending:)
    // Returns save states ordered by timestamp.
    func normalSaveStatesByTimestamp(ascending ascFlag: Bool) -> [OEDBSaveState] {
        return normalSaveStates.sorted {
            guard let d1 = $0.timestamp,
                  let d2 = $1.timestamp
            else { return false }
            return ascFlag ? d1.compare(d2) == .orderedDescending : d1.compare(d2) == .orderedAscending
        }
    }
    
    var autosaveState: OEDBSaveState? {
        let predicate = NSPredicate(format: "name beginswith[c] %@", OEDBSaveState.autosaveName)
        
        return saveStates.filter { predicate.evaluate(with: $0) }.first
    }
    
    var quickSaveStates: [OEDBSaveState] {
        let predicate = NSPredicate(format: "name beginswith[c] %@", OEDBSaveState.quicksaveName)
        
        return saveStates.filter { predicate.evaluate(with: $0) }
    }
    
    func quickSaveState(inSlot num: Int) -> OEDBSaveState? {
        let quickSaveName = OEDBSaveState.nameOfQuickSave(inSlot: num)
        let predicate = NSPredicate(format: "name beginswith[c] %@", quickSaveName)
        
        return saveStates.filter { predicate.evaluate(with: $0) }.first
    }
    
    func saveState(withName string: String) -> OEDBSaveState? {
        let predicate = NSPredicate(format: "name == %@", string)
        
        return saveStates.filter { predicate.evaluate(with: $0) }.first
    }
    
    // MARK: -
    
    func removeMissingStates() {
        var needsSave = false
        for state in saveStates {
            needsSave = needsSave || state.deleteAndRemoveFilesIfInvalid()
        }
        if needsSave {
            save()
        }
    }
    
    // MARK: -
    
    func incrementPlayCount() {
        var currentCount = playCount?.intValue ?? 0
        currentCount += 1
        playCount = currentCount as NSNumber
    }
    
    func addTimeIntervalToPlayTime(_ timeInterval: TimeInterval) {
        var currentPlayTime = TimeInterval(playTime?.doubleValue ?? 0)
        currentPlayTime += timeInterval
        playTime = currentPlayTime as NSNumber
    }
    
    /// Sets rom’s `lastPlayed` to now.
    func markAsPlayedNow() {
        lastPlayed = Date()
    }
    
    // MARK: -
    
    var filesAvailable: Bool {
        return (try? url?.checkResourceIsReachable()) ?? false
    }
    
    func delete(moveToTrash: Bool, keepSaveStates: Bool) {
        
        if moveToTrash,
           let url = url,
           let romsFolderURL = libraryDatabase.romsFolderURL,
           url.isSubpath(of: romsFolderURL) {
            var count = 1
            if archiveFileIndex != nil,
               let location = location {
                let predicate = NSPredicate(format: "location == %@", location)
                let fetchRequest = Self.fetchRequest()
                fetchRequest.predicate = predicate
                count = (try? managedObjectContext?.count(for: fetchRequest)) ?? count
            }
            
            if count == 1 {
                if let file = try? OEFile(url: url) {
                    var willDeleteSubFolder = false
                    
                    // Delete game in subfolder in system's folder if system supports discs with descriptor file
                    if let systemPlugin = game?.system?.plugin,
                       systemPlugin.supportsDiscsWithDescriptorFile {
                        let truncatedFolderPath = file.fileURL.deletingLastPathComponent().deletingLastPathComponent().absoluteString
                        let isFileInSubFolder = truncatedFolderPath != libraryDatabase.romsFolderURL?.absoluteString
                        
                        // Games of systems that support discs are now copied to subfolders with their referenced files, so delete the whole subfolder. Else, handle legacy case.
                        if isFileInSubFolder {
                            willDeleteSubFolder = true
                        }
                    }
                    
                    if willDeleteSubFolder {
                        NSWorkspace.shared.recycle([file.fileURL.deletingLastPathComponent()])
                    } else {
                        NSWorkspace.shared.recycle(file.allFileURLs)
                    }
                }
            } else {
                DLog("Keeping file, other roms depent on it!")
            }
        }
        
        if !keepSaveStates {
            if saveStateCount > 0,
               let statesFolderURL = saveStates.first?.url.deletingLastPathComponent(),
               let file = try? OEFile(url: statesFolderURL) {
                NSWorkspace.shared.recycle(file.allFileURLs)
            }
        }
        
        managedObjectContext?.delete(self)
    }
    
    @nonobjc
    @discardableResult
    func consolidateFiles() throws -> Bool {
        guard
            var url = url,
            let romsFolderURL = libraryDatabase.romsFolderURL
        else { return true }
        
        if let isReachable = try? url.checkResourceIsReachable(),
           isReachable,
           !url.isSubpath(of: romsFolderURL) {
            var romFileLocked = false
            if let attributes = try? url.resourceValues(forKeys: [.isUserImmutableKey]),
               let immutable = attributes.isUserImmutable,
               immutable {
                romFileLocked = true
                var resourceValues = URLResourceValues()
                resourceValues.isUserImmutable = false
                try? url.setResourceValues(resourceValues)
            }
            
            let fullName = url.lastPathComponent as NSString
            let pathExtension = fullName.pathExtension
            let baseName = fullName.deletingPathExtension
            
            var unsortedFolder: URL
            if let system = game?.system {
                unsortedFolder = libraryDatabase.romsFolderURL(for: system)
            } else {
                unsortedFolder = libraryDatabase.unsortedRomsFolderURL
            }
            // Copy game to subfolder in system's folder if system supports discs with descriptor file
            if let systemPlugin = game?.system?.plugin,
               systemPlugin.supportsDiscsWithDescriptorFile {
                unsortedFolder = unsortedFolder.appendingPathComponent(baseName, isDirectory: true)
                
                unsortedFolder = unsortedFolder.uniqueURL { triesCount in
                    let newName = "\(baseName) \(triesCount)"
                    return unsortedFolder.deletingLastPathComponent().appendingPathComponent(newName, isDirectory: true)
                }
                
                try? FileManager.default.createDirectory(at: unsortedFolder, withIntermediateDirectories: true, attributes: nil)
            }
            var romURL = unsortedFolder.appendingPathComponent(fullName as String, isDirectory: false)
            romURL = romURL.uniqueURL { triesCount in
                let newName = "\(baseName) \(triesCount).\(pathExtension)"
                return unsortedFolder.appendingPathComponent(newName)
            }
            
            do {
                let file = try? OEFile(url: url)
                let copiedFile = try file?.copyingFile(to: romURL)
                if copiedFile != nil {
                    // Lock original file again
                    if romFileLocked {
                        var resourceValues = URLResourceValues()
                        resourceValues.isUserImmutable = true
                        try? url.setResourceValues(resourceValues)
                    }
                    
                    self.url = romURL
                    DLog("New URL: \(romURL)")
                }
            } catch {
                return false
            }
        }
        return true
    }
}

// MARK: - Debug

#if DEBUG
@available(macOS 11.0, *)
extension OEDBRom {
    
    func dump(prefix: String = "---") {
        Logger.library.debug("\(prefix) Beginning of ROM dump")
        
        Logger.library.debug("\(prefix) ROM location is \(self.location?.description ?? "nil")")
        // Logger.library.debug("\(prefix) favorite? \(self.isFavorite)")
        Logger.library.debug("\(prefix) MD5 is \(self.md5?.description ?? "nil")")
        Logger.library.debug("\(prefix) last played is \(self.lastPlayed?.description ?? "nil")")
        Logger.library.debug("\(prefix) file size is \(self.fileSize?.description ?? "nil")")
        Logger.library.debug("\(prefix) play count is \(self.playCount?.description ?? "nil")")
        Logger.library.debug("\(prefix) play time is \(self.playTime?.description ?? "nil")")
        Logger.library.debug("\(prefix) ROM is linked to a game? \(self.game != nil ? "YES" : "NO")")
        
        Logger.library.debug("\(prefix) Number of save states for this ROM is \(self.saveStateCount)")
        
        Logger.library.debug("\(prefix) End of ROM dump\n\n")
    }
}
#endif
