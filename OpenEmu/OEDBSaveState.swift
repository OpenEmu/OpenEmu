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
import OpenEmuKit

@objc
final class OEDBSaveState: OEDBItem {
    
    // Preference keys
    static let useQuickSaveSlotsKey = "UseQuickSaveSlots"
    
    // Special name constants
    static let specialNamePrefix = "OESpecialState_"
    static let autosaveName = "\(specialNamePrefix)auto"
    static let quicksaveName = "\(specialNamePrefix)quick"
    
    // Required files
    static let bundleExtension = "oesavestate"
    private static let dataFile = "State"
    private static let screenshotFile = "ScreenShot"
    private static let infoPlistFile = "Info.plist"
    private static let latestVersion = "1.0"
    
    // Info.plist keys
    private static let infoVersionKey = "Version"
    private static let infoNameKey = "Name"
    private static let infoROMMD5Key = "ROM MD5"
    private static let infoCoreIdentifierKey = "Core Identifier"
    private static let infoCoreVersionKey = "Core Version"
    private static let infoTimestampKey = "Timestamp"
    
    // MARK: - CoreDataProperties
    
    @NSManaged var coreIdentifier: String
    @NSManaged var coreVersion: String?
    @NSManaged var location: String
    @NSManaged var name: String
    @NSManaged var timestamp: Date?
    // @NSManaged var userDescription: String?
    @NSManaged var rom: OEDBRom?
    
    // MARK: -
    
    @objc(URL)
    var url: URL {
        get {
            let saveStateDirectory = libraryDatabase.stateFolderURL
            return URL(string: location, relativeTo: saveStateDirectory)!
        }
        set {
            let saveStateDirectory = libraryDatabase.stateFolderURL
            let string = newValue.url(relativeTo: saveStateDirectory)!.relativeString
            // make sure we don't save trailing '/' for save state bundles
            location = string.removingTrailingSlash()
        }
    }
    
    var screenshotURL: URL {
        return Self.screenShotURL(withBundleURL: url)
    }
    
    var dataFileURL: URL {
        return Self.dataFileURL(withBundleURL: url)
    }
    
    private var infoPlistURL: URL {
        return Self.infoPlistURL(withBundleURL: url)
    }
    
    var systemIdentifier: String? {
        return rom?.game?.system?.systemIdentifier
    }
    
    // MARK: -
    
    override class var entityName: String { "SaveState" }
    
    // MARK: -
    
    override class func createObject(in context: NSManagedObjectContext) -> Self {
        let result = super.createObject(in: context)
        result.timestamp = Date()
        return result
    }
    
    private class func saveState(with url: URL, in context: NSManagedObjectContext) -> OEDBSaveState? {
        
        let saveStateDirectoryURL = context.libraryDatabase!.stateFolderURL
        
        // normalize URL for lookup
        let relativeURL = url.url(relativeTo: saveStateDirectoryURL)!
        let urlString = relativeURL.relativeString.removingTrailingSlash()
        
        // query core data
        let predicate = NSPredicate(format: "location == %@", urlString)
        let results = context.allObjects(ofType: Self.self, matching: predicate)
        if results.count > 1 {
            NSLog("WARNING: Found several save states with the same URL!")
        }
        
        return results.first
    }
    
    @discardableResult
    class func createSaveState(byImportingBundleURL bundleURL: URL, into context: NSManagedObjectContext, copy copyFlag: Bool = false) -> OEDBSaveState? {
        
        // Check if state is already in database
        if let state = saveState(with: bundleURL, in: context) {
            return state
        }
        
        // Check url extension
        let fileExtension = bundleURL.pathExtension.lowercased()
        if fileExtension != bundleExtension {
            DLog("SaveState \(bundleURL) has wrong extension (\(fileExtension))")
            return nil
        }
        
        // See if state and Info.plist files are available
        let dataFileURL = dataFileURL(withBundleURL: bundleURL)
        let infoPlistURL = infoPlistURL(withBundleURL: bundleURL)
        
        if (try? dataFileURL.checkResourceIsReachable()) != true {
            DLog("SaveState \(bundleURL) has no data file")
            return nil
        }
        if (try? infoPlistURL.checkResourceIsReachable()) != true {
            DLog("SaveState \(bundleURL) has no Info.plist file")
            return nil
        }
        
        // Create new object
        let standardizedURL: URL
        if copyFlag {
            func makeTempURL(_ i: Int) -> URL {
                var fileName = "SaveState.\(bundleExtension)"
                if i != 0 {
                    fileName = "SaveState \(i).\(bundleExtension)"
                }
                return FileManager.default.temporaryDirectory.appendingPathComponent(fileName, isDirectory: true)
            }
            
            var temporaryURL: URL
            var i = 0
            repeat {
                temporaryURL = makeTempURL(i)
                i += 1
            } while FileManager.default.fileExists(atPath: temporaryURL.path)
            try? FileManager.default.copyItem(at: bundleURL, to: temporaryURL)
            
            standardizedURL = temporaryURL.standardized
        } else {
            standardizedURL = bundleURL.standardized
        }
        
        let state = createObject(in: context)
        state.url = standardizedURL
        
        // Try to read Info.plist
        if !state.readFromDisk() {
            DLog("SaveState \(bundleURL) seems invalid after further inspection!")
            state.delete()
            return nil
        }
        
        if !state.moveToDefaultLocation() {
            DLog("SaveState \(bundleURL) could not be moved!")
            state.delete()
            return nil
        }
        
        state.save()
        return state
    }
    
    class func createSaveState(named name: String, for rom: OEDBRom, core: OECorePlugin, withFile stateFileURL: URL, in context: NSManagedObjectContext) -> OEDBSaveState? {
        
        let dataFileURL = stateFileURL.standardized
        
        // Check supplied values
        if (try? dataFileURL.checkResourceIsReachable()) != true {
            DLog("State file does not exist!")
            return nil
        }
        
        if name.isEmpty {
            DLog("Invalid Save State name!")
            return nil
        }
        
        let temporaryURL = FileManager.default.temporaryDirectory
            .appendingPathComponent("org.openemu.openemu", isDirectory: true)
            .appendingPathComponent("SaveState.\(bundleExtension)", isDirectory: true)
        
        let saveState = OEDBSaveState.createObject(in: context)
        saveState.name = name
        saveState.rom = rom
        saveState.coreIdentifier = core.bundleIdentifier
        saveState.coreVersion = core.version
        saveState.url = temporaryURL
        
        do {
            try FileManager.default.createDirectory(at: temporaryURL, withIntermediateDirectories: true, attributes: nil)
        } catch {
            DLog("Could not create save state bundle!")
            DLog("\(error)")
            
            saveState.delete()
            return nil
        }
        
        if !saveState.writeToDisk() {
            DLog("Could not write Info.plist!")
            saveState.delete()
            return nil
        }
        
        if !saveState.moveToDefaultLocation() {
            DLog("Could not move save state to default location!")
            saveState.delete()
            return nil
        }
        
        if !saveState.replaceStateFileWithFile(at: dataFileURL) {
            DLog("Could not copy data file to bundle!")
            saveState.delete()
            return nil
        }
        
        saveState.save()
        return saveState
    }
    
    class func updateOrCreateState(with stateURL: URL, in context: NSManagedObjectContext) -> OEDBSaveState? {
        
        guard let saveState = createSaveState(byImportingBundleURL: stateURL, into: context)
        else { return nil }
        
        saveState.readFromDisk()
        saveState.moveToDefaultLocation()
        saveState.save()
        
        if !saveState.isValid {
            saveState.delete()
            return nil
        }
        
        return saveState
    }
    
    class func nameOfQuickSave(inSlot slot: Int) -> String {
        return slot == 0 ? quicksaveName : "\(quicksaveName)\(slot)"
    }
    
    private class func dataFileURL(withBundleURL url: URL) -> URL {
        return url.appendingPathComponent(dataFile, isDirectory: false)
    }
    
    private class func infoPlistURL(withBundleURL url: URL) -> URL {
        return url.appendingPathComponent(infoPlistFile, isDirectory: false)
    }
    
    private class func screenShotURL(withBundleURL url: URL) -> URL {
        return url.appendingPathComponent(screenshotFile, isDirectory: false)
    }
    
    // MARK: - Handling Bundle & Files
    
    @discardableResult
    func writeToDisk() -> Bool {
        let infoPlist = NSMutableDictionary()
        
        let md5Hash = rom?.md5HashIfAvailable
        
        if name.isEmpty {
            DLog("Save state is corrupted! Name is missing.")
            return false
        }
        if coreIdentifier.isEmpty {
            DLog("Save state is corrupted! Core Identifier is invalid.")
            return false
        }
        if md5Hash == nil || md5Hash!.isEmpty {
            DLog("Save state is corrupted! MD5Hash or rom are missing.")
            return false
        }
        
        infoPlist[Self.infoNameKey] = name
        infoPlist[Self.infoCoreIdentifierKey] = coreIdentifier
        infoPlist[Self.infoROMMD5Key] = md5Hash
        infoPlist[Self.infoVersionKey] = Self.latestVersion
        
        if let coreVersion = coreVersion {
            infoPlist[Self.infoCoreVersionKey] = coreVersion
        }
        if let timestamp = timestamp {
            infoPlist[Self.infoTimestampKey] = timestamp
        }
        
        do {
            try infoPlist.write(to: infoPlistURL)
        } catch {
            DLog("Unable to write Info.plist file!")
            return false
        }
        
        return true
    }
    
    @discardableResult
    func readFromDisk() -> Bool {
        // Read values from Info.plist
        guard let infoPlist = try? NSDictionary(contentsOf: infoPlistURL, error: ()) else {
            DLog("Could not read Info.plist file! This state should be deleted!")
            return false
        }
        // First values are mandatory
        let name = infoPlist[Self.infoNameKey] as? String
        let romMD5 = infoPlist[Self.infoROMMD5Key] as? String
        let coreIdentifier = infoPlist[Self.infoCoreIdentifierKey] as? String
        // These values are kind of optional
        let coreVersion = infoPlist[Self.infoCoreVersionKey] as? String
        let timestamp = infoPlist[Self.infoTimestampKey] as? Date
        var version = infoPlist[Self.infoVersionKey] as? String
        
        // make sure we have a version (shouldn't be a problem),
        // and this OE version can handle it
        if version == nil || version!.isEmpty {
            version = "1.0"
        }
        
        if version!.compare(Self.latestVersion) == .orderedDescending {
            DLog("This version of OpenEmu only supports save states up to version \(Self.latestVersion). SaveState uses version \(version!) format")
            return false
        }
        
        // in the future, we can start differentiating save state versions here
        
        // Check values for sanity
        guard let name = name, !name.isEmpty else {
            DLog("Info.plist does not contain a valid name!")
            return false
        }
        
        guard let romMD5 = romMD5, !romMD5.isEmpty else {
            DLog("Info.plist does not contain a valid rom reference!")
            return false
        }
        
        guard let coreIdentifier = coreIdentifier, !coreIdentifier.isEmpty else {
            DLog("Info.plist does not contain a valid core reference!")
            return false
        }
        
        // Check additional files (data)
        if (try? dataFileURL.checkResourceIsReachable()) != true {
            DLog("Data file is missing!")
            return false
        }
        
        // Make sure the rom file is available
        let rom: OEDBRom?
        do {
            rom = try OEDBRom.rom(withMD5HashString: romMD5, in: managedObjectContext!)
        } catch {
            DLog("Could not find ROM with MD5 hash \(romMD5)")
            DLog("\(error)")
            return false
        }
        
        guard let rom = rom else {
            return false
        }
        
        // Set mandatory values
        self.name = name
        self.rom = rom
        self.coreIdentifier = coreIdentifier
        
        // Set optional values
        if let coreVersion = coreVersion {
            self.coreVersion = coreVersion
        }
        if let timestamp = timestamp {
            self.timestamp = timestamp
        }
        
        return true
    }
    
    // MARK: - Management
    
    var filesAvailable: Bool {
        if (try? url.checkResourceIsReachable()) != true {
            DLog("bundle missing: \(url.path)")
            return false
        }
        if (try? dataFileURL.checkResourceIsReachable()) != true {
            DLog("state missing: \(dataFileURL.path)")
            return false
        }
        if (try? infoPlistURL.checkResourceIsReachable()) != true {
            DLog("info.plist missing: \(dataFileURL.path)")
            return false
        }
        return true
    }
    
    var isValid: Bool {
        return filesAvailable && rom != nil
    }
    
    @discardableResult
    func replaceStateFileWithFile(at stateFileURL: URL) -> Bool {
        do {
            try FileManager.default.removeItem(at: dataFileURL)
        } catch {
            DLog("Could not delete previous state file!")
            DLog("\(error)")
        }
        do {
            try FileManager.default.moveItem(at: stateFileURL, to: dataFileURL)
        } catch {
            DLog("Could not copy new state file")
            DLog("\(error)")
            return false
        }
        return true
    }
    
    func deleteAndRemoveFiles() {
        try? FileManager.default.trashItem(at: url, resultingItemURL: nil)
        delete()
        save()
    }
    
    /// Returns `true` if it was invalid.
    @discardableResult
    func deleteAndRemoveFilesIfInvalid() -> Bool {
        let invalid = !isValid
        if invalid {
            deleteAndRemoveFiles()
        }
        return invalid
    }
    
    @discardableResult
    func moveToDefaultLocation() -> Bool {
        guard let rom = rom else { return false }
        let saveStateDirectoryURL = libraryDatabase.stateFolderURL(for: rom)
        let currentURL = url
        
        let desiredName = URL.validFilename(from: displayName)
        var desiredFileName = "\(desiredName).\(Self.bundleExtension)"
        var url = saveStateDirectoryURL.appendingPathComponent(desiredFileName, isDirectory: true)
        
        // check if save state is already where it's supposed to be
        if url.absoluteURL.standardized == currentURL.absoluteURL.standardized {
            return true
        }
        
        // check if url is already taken, determine unique url if so
        if (try? url.checkResourceIsReachable()) == true {
            var count = 1
            repeat {
                desiredFileName = "\(desiredName) \(count).\(Self.bundleExtension)"
                url = saveStateDirectoryURL.appendingPathComponent(desiredFileName, isDirectory: true)
                count += 1
            } while url.standardized != currentURL.standardized && (try? url.checkResourceIsReachable()) == true
        }
        
        // only proceed if the location has changed
        if url.absoluteURL.standardized == currentURL.absoluteURL.standardized {
            return true
        }
        
        do {
            try FileManager.default.moveItem(at: currentURL, to: url)
        } catch {
            DLog("Could not move save state to new location!")
            return false
        }
        
        self.url = url
        save()
        
        return true
    }
    
    override func willSave() {
        if hasChanges && !isDeleted {
            writeToDisk()
        }
    }
    
    // MARK: - Data Accessors
    
    /// Should be used instead of `name` if the string is to be displayed to the user.
    var displayName: String {
        if !isSpecialState {
            return name
        }
        else if name == Self.autosaveName {
            return NSLocalizedString("Auto Save State", comment: "Autosave state display name")
        }
        else if name == Self.quicksaveName {
            return NSLocalizedString("Quick Save State", comment: "Quicksave state display name")
        }
        else if name.hasPrefix(Self.quicksaveName) {
            return .localizedStringWithFormat(NSLocalizedString("Quick Save, Slot %@", comment: "Quicksave state display name with slot"), String(name.dropFirst(Self.quicksaveName.count)))
        }
        return name
    }
    
    var isSpecialState: Bool {
        return name.hasPrefix(Self.specialNamePrefix)
    }
}

// MARK: - NSPasteboardWriting

extension OEDBSaveState: NSPasteboardWriting {
    
    func writableTypes(for pasteboard: NSPasteboard) -> [NSPasteboard.PasteboardType] {
        return [.fileURL]
    }
    
    func pasteboardPropertyList(forType type: NSPasteboard.PasteboardType) -> Any? {
        if type == .fileURL {
            return (url.absoluteURL as NSURL).pasteboardPropertyList(forType: type)
        }
        return nil
    }
}

private extension String {
    
    func removingTrailingSlash() -> String {
        if hasSuffix("/") {
            return "\(dropLast())"
        } else {
            return self
        }
    }
}
