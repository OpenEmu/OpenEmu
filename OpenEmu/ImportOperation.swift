// Copyright (c) 2022, OpenEmu Team
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
import OpenEmuKit
import OpenEmuSystem
import XADMaster

let OECopyToLibraryKey        = "copyToLibrary"
let OEAutomaticallyGetInfoKey = "automaticallyGetInfo"
let OEImportManualSystems     = "OEImportManualSystems"

// MARK: - Import Errors

let OEImportErrorDomainFatal      = "OEImportFatalDomain"
let OEImportErrorDomainResolvable = "OEImportResolvableDomain"
let OEImportErrorDomainSuccess    = "OEImportSuccessDomain"

enum OEImportErrorCode: Int {
    case alreadyInDatabase = -1
    case multipleSystems = 2
    case noSystem = 3
    case invalidFile = 4
    case additionalFiles = 5
    case noHash = 6
    case noGame = 7
    case disallowArchivedFile = 8
    case emptyFile = 9
    case alreadyInDatabaseFileUnreachable = 10
}

typealias ImportItemCompletionBlock = (NSManagedObjectID?) -> Void

final class ImportOperation: Operation, NSCopying {
    
    enum ExitStatus {
        case none, errorResolvable, errorFatal, success
    }
    
    /// Specifies the result of an alternative import operation that is tried before queueing.
    private enum ImportResult {
        /// The operation was handled by this request.
        case handled
        /// The operation was not handled by this request.
        case notHandled
        /// The operation was unsuccessful.
        case failed
    }
    
    var exploreArchives = true
    var isDisallowedArchiveWithMultipleFiles = false
    
    var url: URL
    var sourceURL: URL
    var extractedFileURL: URL?
    var collectionID: NSManagedObjectID?
    var romObjectID: NSManagedObjectID? {
        rom?.permanentID
    }
    
    var systemIdentifiers: [String] = []
    var romLocation: String?
    
    var error: Error?
    var exitStatus: ExitStatus = .none
    
    var completionHandler: ImportItemCompletionBlock?
    var importer: ROMImporter!
    
    var isChecked = false
    
    private var file: OEFile?
    
    private var fileName: String?
    private var archiveFileIndex = NSNotFound
    
    private var md5Hash: String?
    
    private var rom: OEDBRom?
    
    private var _shouldExit = false
    private var shouldExit: Bool {
        get {
            return _shouldExit || isCancelled
        }
        set {
            _shouldExit = newValue
        }
    }
    
    init(url: URL, sourceURL: URL) {
        self.url = url
        self.sourceURL = sourceURL
        super.init()
    }
    
    // MARK: - NSCopying
    
    func copy(with zone: NSZone? = nil) -> Any {
        let op = ImportOperation(url: url, sourceURL: sourceURL)
        
        op.exitStatus = .none
        op.error = nil
        
        op.exploreArchives = exploreArchives
        op.collectionID = collectionID
        op.rom = rom
        
        op.systemIdentifiers = systemIdentifiers
        op.completionHandler = completionHandler
        op.importer = importer
        
        op.isChecked = isChecked
        
        op.fileName = fileName
        op.extractedFileURL = extractedFileURL
        op.archiveFileIndex = archiveFileIndex
        op.md5Hash = md5Hash
        op.romLocation = romLocation
        
        op.isDisallowedArchiveWithMultipleFiles = isDisallowedArchiveWithMultipleFiles
        
        return op
    }
    
    // MARK: -
    
    convenience init?(url: URL, in importer: ROMImporter) {
        var isReachable: Bool
        do {
            isReachable = try url.checkResourceIsReachable()
        } catch {
            DLog("Could not reach url at \(url)")
            DLog("\(error)")
            return nil
        }
        if !isReachable {
            DLog("Could not reach url at \(url)")
            return nil
        }
        
        // Ignore hidden or package files
        if let resourceValues = try? url.resourceValues(forKeys: [.isPackageKey, .isHiddenKey]),
           resourceValues.isPackage! || resourceValues.isHidden!
        {
            DLog("Item is hidden file or package directory at \(url)")
            // Check for .oesavestate files and copy them directly (not going through importer queue)
            Self.tryImportSaveState(at: url)
            
            do {
                try PluginDocument.importCorePlugin(at: url)
                try PluginDocument.importSystemPlugin(at: url)
            } catch {
                NSApp.presentError(error)
            }
            
            return nil
        }
        
        // Try to import .oeshaderplugin
        switch Self.tryShader(at: url) {
        case .handled:
            return nil
        case .failed:
            return nil
        case .notHandled:
            break
        }
        
        // Check for PlayStation .sbi subchannel data files and copy them directly (not going through importer queue)
        switch Self.trySBIFile(at: url) {
        case .handled:
            return nil
        case .failed:
            return nil
        case .notHandled:
            break
        }
        
        // Ignore text files that are .md
        if Self.isTextFile(at: url) {
            return nil
        }
        
        if Self.isInvalidExtension(at: url) {
            return nil
        }
        
        self.init(url: url, sourceURL: url)
        self.importer = importer
    }
    
    private static func tryImportSaveState(at url: URL) {
        let pathExtension = url.pathExtension.lowercased()
        
        if pathExtension == OEDBSaveState.bundleExtension,
           let db = OELibraryDatabase.default {
            OEDBSaveState.createSaveState(byImportingBundleURL: url, into: db.mainThreadContext, copy: true)
        }
    }
    
    @discardableResult
    static func importShaderPlugin(at url: URL) -> Bool {
        return tryShader(at: url) == .handled
    }
    
    @discardableResult
    private static func tryShader(at url: URL) -> ImportResult {
        let pathExtension = url.pathExtension.lowercased()
        guard  pathExtension == "oeshaderplugin" else {
            return .notHandled
        }
       
        os_log(.info, log: .import, "File seems to be a shader plugin at %{public}@", url.path);
        
        let fileManager = FileManager.default
        let filename = (url.lastPathComponent as NSString).deletingPathExtension
        let shadersPath = URL.oeApplicationSupportDirectory
            .appendingPathComponent("Shaders", isDirectory: true)
        let destination = shadersPath.appendingPathComponent(filename, isDirectory: true)
        
        if OEShaderStore.shared.systemShaderNames.contains(filename) {
            // ignore customer shaders with the same name
            os_log(.error, log: .import, "Custom shader name '%{public}@' collides with system shader", filename)
            return .notHandled
        }
        
        if fileManager.fileExists(atPath: destination.path) {
            // lets remove it first
            do {
                try fileManager.trashItem(at: destination, resultingItemURL: nil)
            } catch {
                os_log(.error, log: .import, "Could not remove existing directory '%{public}@' before copying shader: %{public}@", destination.path, error.localizedDescription)
                return .notHandled
            }
        }
        
        do {
            try fileManager.createDirectory(at: destination, withIntermediateDirectories: true, attributes: nil)
        } catch {
            os_log(.error, log: .import, "Could not create directory '%{public}@' before copying shader: %{public}@", destination.path, error.localizedDescription)
            return .notHandled
        }
        
        if let archive = XADArchive.oe_archiveForFile(at: url),
           archive.extract(to: destination.path) {
            OEShaderStore.shared.reload()
        } else {
            os_log(.error, log: .import, "Error extracting shader plugin: %{public}@", url.path)
            return .notHandled
        }
        
        return .handled
    }
    
    @discardableResult
    private static func trySBIFile(at url: URL) -> ImportResult {
        let copyToLibrary = UserDefaults.standard.bool(forKey: OECopyToLibraryKey)
        
        let pathExtension = url.pathExtension.lowercased()
        guard pathExtension == "sbi" && copyToLibrary else {
            return .notHandled
        }
        
        // Check 4-byte SBI header
        let header = try? FileHandle(forReadingFrom: url)
        header?.seek(toFileOffset: 0x0)
        let sbiHeaderBuffer = header?.readData(ofLength: 4)
        let expectedSBIHeader = Data(bytes: "SBI\0", count: 4)
        header?.closeFile()
        let bytesFound = sbiHeaderBuffer == expectedSBIHeader
        
        if !bytesFound {
            return .notHandled // TODO: This should return an error and present it to the user
        }
        
        DLog("File seems to be a SBI file at \(url)")
        
        let fileManager = FileManager.default
        let sbiFilename = url.lastPathComponent
        let sbiExtensionlessFilename = (sbiFilename as NSString).deletingPathExtension as NSString
        // RegEx pattern match the parentheses e.g. " (Disc 1)" and trim sbiExtensionlessFilename string.
        let sbiFolderName = sbiExtensionlessFilename.replacingOccurrences(of: "\\ \\(Disc.*\\)", with: "", options: .regularExpression, range: NSRange(location: 0, length: sbiExtensionlessFilename.length))
        // Get the PlayStation system's Game Library folder URL.
        let database = OELibraryDatabase.default!
        let context  = database.mainThreadContext
        let system = OEDBSystem.system(for: "openemu.system.psx", in: context)!
        let systemFolderURL = database.romsFolderURL(for: system)
        // Copy SBI to subfolder where (presumably) the user also imported their game files - SBI files must be in the same folder as the game.
        // NOTE: This is for convenience and cannot be perfect now that games for disc-supporting systems are copied into a unique subfolder in the system's folder - users may have to manually import.
        // Does not handle SBI-requiring M3U games well (FF VIII, FF IX, Galerians and Parasite Eve II), unless the M3U is named as the basename SBI (e.g. Final Fantasy IX (Europe).m3u and Final Fantasy IX (Europe) (Disc 1).sbi).
        let sbiSubFolderURL = systemFolderURL.appendingPathComponent(sbiFolderName, isDirectory: true)
        let destination = sbiSubFolderURL.appendingPathComponent(sbiFilename, isDirectory: false)
        
        do {
            try fileManager.createDirectory(at: sbiSubFolderURL, withIntermediateDirectories: true)
        } catch {
            DLog("Could not create directory before copying SBI file at \(url)")
            DLog("\(error)");
            // TODO: This should return an error and present it to the user
        }
        
        do {
            try fileManager.copyItem(at: url, to: destination)
        } catch {
            DLog("Could not copy SBI file \(url) to \(destination)")
            DLog("\(error)");
            // TODO: This should return an error and present it to the user
        }
        
        return .handled
    }
    
    private static func isTextFile(at url: URL) -> Bool {
        let pathExtension = url.pathExtension.lowercased()
        
        if pathExtension == "md" {
            // Read 1k of the file and look for null bytes
            let sampleSize = 1024
            let sampleBuffer = UnsafeMutablePointer<Int8>.allocate(capacity: sampleSize)
            let path = (url.path as NSString).fileSystemRepresentation
            if let f = fopen(path, "r") {
                let bytesRead = fread(sampleBuffer, MemoryLayout<Int8>.size, sampleSize, f)
                fclose(f)
                if memchr(sampleBuffer, 0, bytesRead) == nil {
                    return true
                }
            }
        }
        
        return false
    }
    
    private static func isInvalidExtension(at url: URL) -> Bool {
        let pathExtension = url.pathExtension.lowercased()
        
        // Ignore unsupported file extensions
        var validExtensions = OESystemPlugin.supportedTypeExtensions
        
        // Hack fix for #2031
        // TODO: Build set for extensions from all BIOS file types?
        validExtensions.insert("img")
        
        // TODO:
        // The Archived Game document type lists all supported archive extensions, e.g. zip
        if let bundleInfo = Bundle.main.infoDictionary,
           let docTypes = bundleInfo["CFBundleDocumentTypes"] as? [[String : Any]],
           let docType = docTypes.first(where: { $0["CFBundleTypeName"] as? String == "Archived Game" }),
           let extensions = docType["CFBundleTypeExtensions"] as? [String]
        {
            validExtensions.formUnion(extensions)
        }
        
        if !url.isDirectory {
            if !pathExtension.isEmpty,
               !validExtensions.contains(pathExtension)
            {
                DLog("File has unsupported extension (\(pathExtension) at \(url)")
                return true
            }
        }
        
        return false
    }
    
    // MARK: -
    
    override func main() {
        autoreleasepool {
            importer.context?.performAndWait {
                if shouldExit { return }
                
                performImportStepCheckDirectory()
                if shouldExit { return }
                
                performImportStepCheckArchiveFile()
                if shouldExit { return }
                
                performImportStepHash()
                if shouldExit { return }
                
                performImportStepCheckHash()
                if shouldExit { return }
                
                performImportStepParseFile()
                if shouldExit { return }
                
                performImportStepDetermineSystem()
                if shouldExit { return }
                
                performImportStepOrganize()
                if shouldExit { return }
                
                performImportStepCreateCoreDataObjects()
            }
        }
    }
    
    private func exit(with status: ExitStatus, error: Error?) {
        let context = importer.context!
        
        if status == .success,
           let rom = rom,
           let game = rom.game
        {
            if let collectionID = collectionID,
               let collection = OEDBCollection.object(with: collectionID, in: context)
            {
                if !collection.isDeleted {
                    game.collections.insert(collection)
                }
            }
            
            // start sync thread
            if game.status == .processing {
                let database = importer.database
                DispatchQueue.global(qos: .default).asyncAfter(deadline: DispatchTime.now() + .seconds(1)) {
                    database?.startOpenVGDBSync()
                }
            }
        }
        
        if status != .errorFatal {
            try? context.save()
            
            let parentContext = context.parent
            parentContext?.perform {
                try? parentContext?.save()
            }
        }
        
        if status != .errorResolvable,
           let extractedFileURL = extractedFileURL {
            try? FileManager.default.removeItem(at: extractedFileURL)
        }
        
        shouldExit = true
        self.error = error
        exitStatus = status
    }
    
    // MARK: - Importing
    
    private func performImportStepCheckDirectory() {
        // Check if file at URL is a directory
        // if so, add new items for each item in the directory
        if url.isDirectory {
            let propertyKeys: [URLResourceKey] = [.isPackageKey, .isHiddenKey]
            let options: FileManager.DirectoryEnumerationOptions = [.skipsSubdirectoryDescendants, .skipsPackageDescendants, .skipsHiddenFiles]
            
            let directoryEnumerator = FileManager.default.enumerator(at: url, includingPropertiesForKeys: propertyKeys, options: options)
            
            while isCancelled == false,
                  let subURL = directoryEnumerator?.nextObject() as? URL {
                let importer = importer!
                if let subItem = ImportOperation(url: subURL, in: importer) {
                    subItem.completionHandler = completionHandler
                    subItem.collectionID = collectionID
                    importer.addOperation(subItem)
                }
            }
            
            exit(with: .success, error: nil)
        }
    }
    
    private func shouldDisallowArchive(_ archive: XADArchive) -> Bool {
        let context = importer.context!
        let enabledSystems = OEDBSystem.enabledSystems(in: context)
        var enabledExtensions: Set<String> = []
        let arcadeSystem = OEDBSystem.system(for: "openemu.system.arcade", in: context)
        let isArcadeEnabled = arcadeSystem?.isEnabled ?? false
        
        // Get extensions from all enabled systems.
        for system in enabledSystems {
            // Ignore Arcade file extensions (zip, 7z, chd).
            if system.systemIdentifier == "openemu.system.arcade" {
                continue
            }
            if let extensions = system.plugin?.supportedTypeExtensions {
                enabledExtensions.formUnion(extensions)
            }
        }
        
        // When Arcade is enabled, remove conflicting extensions found in Arcade ROMs.
        if isArcadeEnabled {
            enabledExtensions.subtract(["bin", "rom", "a26", "a52", "cas", "col", "com", "int", "p00", "prg"])
        }
        
        // Get extensions from files in archive.
        var archiveExtensions: Set<String> = []
        for i in 0 ..< archive.numberOfEntries() {
            if let name = archive.name(ofEntry: i) as? NSString {
                let ext = name.pathExtension.lowercased()
                archiveExtensions.insert(ext)
            }
        }
        
        // Check if extensions in archive are found.
        enabledExtensions.formIntersection(archiveExtensions)
        if !enabledExtensions.isEmpty {
            return true
        }
        
        return false
    }
    
    private func performImportStepCheckArchiveFile() {
        if exploreArchives == false {
            return
        }
        DLog("")
        let ext = url.pathExtension.lowercased()
        
        // nds and some isos might be recognized as compressed archives by XADArchive
        // but we don't ever want to extract anything from those files
        // Exclusions added here also need added to GameInfoHelper and OpenEmuHelperApp
        if ext == "nds" || ext == "iso" {
            return
        }
        
        guard let archive = XADArchive.oe_archiveForFile(at: url) else { return }
        let formatName = archive.formatName()
        
        // XADArchive file detection is not exactly the best
        // ignore some formats
        if formatName == "MacBinary" ||
           formatName == "LZMA_Alone" {
            return
        }
        
        // disable multi-rom archives
        if archive.numberOfEntries() > 1 {
            // Check if archive contains known extensions, otherwise is assumed Arcade.
            isDisallowedArchiveWithMultipleFiles = shouldDisallowArchive(archive)
            return
        }
        
        for i in  0 ..< archive.numberOfEntries() {
            if archive.entryHasSize(i) && archive.size(ofEntry: i) == 0 ||
               archive.entryIsEncrypted(i) ||
               archive.entryIsDirectory(i) ||
               archive.entryIsArchive(i)
            {
                DLog("Entry \(i) is either empty, or a directory or encrypted or iteself an archive")
                continue
            }
            
            let folder = ArchiveHelper.temporaryDirectoryForDecompressionOfFile(at: url)
            var extractionDst = folder.appendingPathComponent(archive.name(ofEntry: i))
            if extractionDst.pathExtension.isEmpty,
               !url.pathExtension.isEmpty
            {
                // this won't do. Re-add the archive's extension in case it's .smc or the like
                extractionDst = extractionDst.appendingPathComponent(url.pathExtension)
            }
            
            let fm = FileManager.default
            if !fm.fileExists(atPath: extractionDst.path) {
                if !archive.oe_extractEntry(i, as: extractionDst.path, deferDirectories: true, dataFork: true, resourceFork: false) {
                    NSLog("exception handeled")
                }
                
                // exception is caught but handler does not execute, so check if extraction worked
                if extractionDst.fileSize == 0 {
                    try? fm.removeItem(at: folder)
                    DLog("unpack failed")
                    continue
                }
            }
            
            extractedFileURL = extractionDst
            archiveFileIndex = Int(i)
            exploreArchives = false
        }
    }
    
    private func performImportStepHash() {
        if md5Hash != nil {
            return
        }
        
        DLog("")
        let url = extractedFileURL ?? url
        
        do {
            let md5 = try FileManager.default.hashFile(at: url)
            
            md5Hash = md5.lowercased()
            
            if BIOSFile.checkIfBIOSFileAndImport(at: url, withMD5: md5Hash!) {
                DLog("File seems to be a BIOS at \(url)");
                exit(with: .none, error: nil)
            }
        } catch {
            DLog("unable to hash file, this is probably a fatal error")
            DLog("\(error)")
            
            let error = NSError(domain: OEImportErrorDomainFatal, code: OEImportErrorCode.noHash.rawValue)
            exit(with: .errorFatal, error: error)
        }
    }
    
    private func performImportStepCheckHash() {
        DLog("")
        var error: Error
        let md5 = md5Hash!
        let context = importer.context!
        
        var rom: OEDBRom?
        
        do {
            rom = try OEDBRom.rom(withMD5HashString: md5, in: context)
        } catch let err {
            error = err
        }
        
        if let rom = rom {
            let romURL = rom.url
            self.rom = rom
            romLocation = rom.game?.system?.lastLocalizedName
            
            let isReachable = (try? romURL?.checkResourceIsReachable()) ?? false
            if !isReachable {
                DLog("rom file not available")
                error = NSError(domain: OEImportErrorDomainFatal, code: OEImportErrorCode.alreadyInDatabaseFileUnreachable.rawValue)
            }
            else {
                error = NSError(domain: OEImportErrorDomainFatal, code: OEImportErrorCode.alreadyInDatabase.rawValue)
            }
            
            exit(with: .errorFatal, error: error)
        }
    }
    
    private func performImportStepParseFile() {
        /* Parsing M3U+CUE/BIN code walk
         
         1. +[OEFile fileWithURL:error:]
         2. -[OEDiscDescriptor initWithFileURL:error:]
         3. -[OEFile initWithFileURL:error:]
         4. -[OEM3UFile _setUpFileReferencesWithError:]
         5. -[OEDiscDescriptor _fileContentWithError:]
         5a. After -[OEDiscDescriptor _fileContentWithError:] returns, -[OEM3UFile _setUpFileReferencesWithError:] loops through the fileContent string and calls +[OEFile fileWithURL:error:] for each line, thus repeating steps 1-5 but for class OECUESheet. -[OEDiscDescriptor _validateFileURLs:withError:] will be called for the referenced BIN file during *each* CUE iteration
         6. Finally, -[OEDiscDescriptor _validateFileURLs:withError:] called to check the referenced CUE's array
         */
        let url = extractedFileURL ?? url
        
        var file: OEFile
        do {
            file = try OEFile(url: url)
        } catch {
            DLog("Error while parsing file: \(error)")
            exit(with: .errorFatal, error: error)
            return
        }
        
        if file.fileSize == 0 {
            let error = NSError(domain: OEImportErrorDomainFatal, code: OEImportErrorCode.emptyFile.rawValue)
            exit(with: .errorFatal, error: error)
            return
        }
        
        self.file = file
    }
    
    func performImportStepDetermineSystem() {
        DLog("URL: \(sourceURL)")
        if rom != nil {
            return
        }
        
        let context = importer.context!
        
        // see if systemidentifiers are set already (meaning that user determined system)
        var validSystems: [OEDBSystem]
        if !systemIdentifiers.isEmpty {
            var systems: [OEDBSystem] = []
            for id in systemIdentifiers  {
                if let system = OEDBSystem.system(for: id, in: context) {
                    systems.append(system)
                }
            }
            validSystems = systems
        }
        else if UserDefaults.standard.bool(forKey: OEImportManualSystems) {
            validSystems = OEDBSystem.allSystems(in: context)
        }
        else {
            validSystems = OEDBSystem.systems(for: file!, in: context)
        }
        
        if validSystems.isEmpty {
            DLog("Could not get valid systems")
            if extractedFileURL != nil {
                DLog("Try again with zip itself")
                extractedFileURL = nil
                file = nil
                performImportStepParseFile()
                if shouldExit {
                    return
                }
                
                performImportStepDetermineSystem()
            }
            else {
                let error: Error
                // Unless for Arcade, compressed archives must not contain multiple files.
                if isDisallowedArchiveWithMultipleFiles {
                    error = NSError(domain: OEImportErrorDomainFatal, code: OEImportErrorCode.disallowArchivedFile.rawValue)
                } else {
                    error = NSError(domain: OEImportErrorDomainFatal, code: OEImportErrorCode.noSystem.rawValue)
                }
                exit(with: .errorFatal, error: error)
            }
            return
        }
        else if validSystems.count == 1 {
            let systemIdentifier = validSystems.first!.systemIdentifier
            systemIdentifiers = [systemIdentifier]
            
            // Optical disc media or non-Arcade archives with multiple files must not import compressed.
            // Stops false positives importing into Arcade.
            let system = OEDBSystem.system(for: systemIdentifier, in: context)
            if (extractedFileURL != nil && system?.plugin?.systemMedia.contains("OESystemMediaOpticalDisc") ?? false || isDisallowedArchiveWithMultipleFiles)
            {
                let error = NSError(domain: OEImportErrorDomainFatal, code: OEImportErrorCode.disallowArchivedFile.rawValue)
                exit(with: .errorFatal, error: error)
            }
        }
        else {
            // Found multiple valid systems after checking extension and system specific canHandleFile:
            var systemIDs: [String] = []
            for system in validSystems {
                let systemIdentifier = system.systemIdentifier
                systemIDs.append(systemIdentifier)
            }
            systemIdentifiers = systemIDs
            
            let userInfo = [NSLocalizedDescriptionKey : "Aaargh, too many systems. You need to choose one!"]
            let error = NSError(domain: OEImportErrorDomainResolvable, code: OEImportErrorCode.multipleSystems.rawValue, userInfo: userInfo)
            exit(with: .errorResolvable, error: error)
        }
    }
    
    private func performImportStepOrganize() {
        let copyToLibrary = UserDefaults.standard.bool(forKey: OECopyToLibraryKey)
        
        if !copyToLibrary {
            // There is nothing to do in this method if we do not have to copy or move the file.
            return
        }
        
        var url = url
        
        if extractedFileURL != nil {
            var originalFile: OEFile
            do {
                originalFile = try OEFile(url: url)
            } catch {
                DLog("Failed to create file from original archive.")
                exit(with: .errorFatal, error: error)
                return
            }
            
            file = originalFile
        }
        
        let context = importer.context!
        
        // Unlock rom file so we can rename the copy directly
        var romFileLocked = false
        if let resourceValues = try? url.resourceValues(forKeys: [.isUserImmutableKey]),
           resourceValues.isUserImmutable! {
            romFileLocked = true
            var resourceValues = URLResourceValues()
            resourceValues.isUserImmutable = false
            try? url.setResourceValues(resourceValues)
        }
        
        // Copy to system sub-folder in library folder if it's not already there
        let database = importer.database!
        
        if copyToLibrary,
           !url.isSubpath(of: database.romsFolderURL!)
        {
            var system: OEDBSystem
            if let rom = rom {
                DLog("using rom object")
                system = rom.game!.system!
            }
            else {
                assert(systemIdentifiers.count == 1, "System should have been detected at an earlier import stage")
                let systemIdentifier = systemIdentifiers.first!
                system = OEDBSystem.system(for: systemIdentifier, in: context)!
            }
            
            let fullName  = url.lastPathComponent
            let ext = (fullName as NSString).pathExtension
            let baseName  = (fullName as NSString).deletingPathExtension
            
            var systemFolder = database.romsFolderURL(for: system)
            // Copy game to subfolder in system's folder if system supports discs with descriptor file
            if system.plugin?.supportsDiscsWithDescriptorFile ?? false {
                systemFolder = systemFolder.appendingPathComponent(baseName, isDirectory: true)
                
                systemFolder = systemFolder.uniqueURL { triesCount in
                    let newName = "\(baseName) \(triesCount)"
                    return systemFolder.deletingLastPathComponent().appendingPathComponent(newName, isDirectory: true)
                }
                
                try? FileManager.default.createDirectory(at: systemFolder, withIntermediateDirectories: true)
            }
            
            var romURL = systemFolder.appendingPathComponent(fullName, isDirectory: false)
            
            if romURL == url {
                self.url = romURL
                return
            }
            
            romURL = romURL.uniqueURL { triesCount in
                let newName = "\(baseName) \(triesCount).\(ext)"
                return systemFolder.appendingPathComponent(newName)
            }
            
            var copiedFile: OEFile
            do {
                copiedFile = try file!.copyingFile(to: romURL)
            } catch {
                DLog("Could not copy rom to library")
                exit(with: .errorFatal, error: error)
                return
            }
            
            // Lock original file again
            if romFileLocked {
                var resourceValues = URLResourceValues()
                resourceValues.isUserImmutable = true
                try? url.setResourceValues(resourceValues)
            }
            
            url = romURL
            self.url = url
            self.file = copiedFile
        }
    }
    
    private func performImportStepCreateCoreDataObjects() {
        DLog("")
        
        let context = importer.context!
        var rom = rom
        
        if rom == nil {
            rom = OEDBRom.createObject(in: context)
        }
        
        guard let rom = rom else {
            let error = NSError(domain: OEImportErrorDomainFatal, code: OEImportErrorCode.invalidFile.rawValue)
            exit(with: .errorFatal, error: error)
            return
        }
        
        rom.url = url
        if let fileName = fileName {
            rom.fileName = fileName
        }
        if let md5 = md5Hash {
            rom.md5 = md5.lowercased()
        }
        
        // Check if system plugin for ROM implemented headerLookupForFile: and serialLookupForFile:
        assert(systemIdentifiers.count == 1, "System should have been detected at an earlier import stage")
        let systemIdentifier = systemIdentifiers.last!
        
        let lookupURL = extractedFileURL ?? rom.url!
        var file: OEFile
        do {
            file = try OEFile(url: lookupURL)
        } catch {
            exit(with: .errorFatal, error: error)
            return
        }
        
        let headerFound = OEDBSystem.header(for: file, forSystem: systemIdentifier)
        let serialFound = OEDBSystem.serial(for: file, forSystem: systemIdentifier)
        
        if headerFound != nil {
            rom.header = headerFound
        }
        
        if serialFound != nil {
            rom.serial = serialFound
        }
        
        if archiveFileIndex != NSNotFound {
            rom.archiveFileIndex = archiveFileIndex as NSNumber
        }
        var game = rom.game
        if game == nil {
            let system = OEDBSystem.system(for: systemIdentifier, in: context)
            if system != nil {
                // Ensure single-file Arcade archives containing a known file suffix (bin, rom) retain their archived name.
                if systemIdentifier == "openemu.system.arcade" {
                    extractedFileURL = nil
                }
                
                let url = extractedFileURL ?? rom.url!
                let gameTitleWithSuffix = url.lastPathComponent
                let gameTitleWithoutSuffix = (gameTitleWithSuffix as NSString).deletingPathExtension
                
                game = OEDBGame.createObject(in: context)
                game!.name = gameTitleWithoutSuffix
                game!.system = system
                game!.importDate = Date()
            }
            else {
                let userInfo = [NSLocalizedDescriptionKey : "No system! Someone must have deleted it!"]
                let error = NSError(domain: OEImportErrorDomainFatal, code: OEImportErrorCode.noSystem.rawValue, userInfo: userInfo)
                exit(with: .errorFatal, error: error)
                return
            }
        }
        
        if let game = game {
            rom.game = game
            
            if UserDefaults.standard.bool(forKey: OEAutomaticallyGetInfoKey) {
                game.status = .processing
            }
            
            self.rom = rom
            
            exit(with: .success, error: nil)
        }
        else {
            rom.delete()
            self.rom = nil
            
            let error = NSError(domain: OEImportErrorDomainFatal, code: OEImportErrorCode.noGame.rawValue)
            exit(with: .errorFatal, error: error)
        }
    }
}
