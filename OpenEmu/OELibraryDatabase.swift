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

import Cocoa
import OSLog

@objc
extension OELibraryDatabase {
    
    // MARK: - Administration
    
    func disableSystemsWithoutPlugin() {
        let allSystems = OEDBSystem.allSystems(in: mainThreadContext)
        for system in allSystems {
            if system.plugin == nil {
                system.isEnabled = false
            }
        }
    }
    
    // MARK: - Queries
    
    var collections: [SidebarItem] {
        assert(Thread.isMainThread, "Only call on main thread!")
        
        let context = mainThreadContext
        let sortDescriptor = NSSortDescriptor(key: "name", ascending: true, selector: #selector(NSString.localizedStandardCompare))
        
        var collectionsArray: [SidebarItem] = []
        let allGamesCollections = OEDBAllGamesCollection.shared
        collectionsArray.append(allGamesCollections)
        
        let smartCollections = OEDBSmartCollection.allObjects(in: context, sortBy: [sortDescriptor], error: nil)
        collectionsArray.append(contentsOf: smartCollections as! [OEDBSmartCollection])
        
        let collections = OEDBCollection.allObjects(in: context, sortBy: [sortDescriptor], error: nil)
        collectionsArray.append(contentsOf: collections as! [OEDBCollection])
        
        return collectionsArray
    }
    
    func rom(forMD5Hash hashString: String) -> OEDBRom? {
        var result: [OEDBRom]?
        let context = mainThreadContext
        context.performAndWait {
            let fetchRequest = OEDBRom.fetchRequest()
            fetchRequest.fetchLimit = 1
            fetchRequest.includesPendingChanges = true
            
            let predicate = NSPredicate(format: "md5 == %@", hashString)
            fetchRequest.predicate = predicate
            
            do {
                result = try context.fetch(fetchRequest) as? [OEDBRom]
            } catch {
                os_log(.error, log: OE_LOG_LIBRARY, "Error executing fetch request to get rom by md5: %{public}@", error as NSError)
                return
            }
        }
        return result?.first as? OEDBRom
    }
    
    var lastPlayedRoms: [OEDBRom] {
        let numberOfRoms = NSDocumentController.shared.maximumRecentDocumentCount
        let fetchRequest = NSFetchRequest<NSFetchRequestResult>(entityName: OEDBRom.entityName)
        
        let predicate = NSPredicate(format: "lastPlayed != nil")
        let sortDesc = NSSortDescriptor(key: "lastPlayed", ascending: false)
        fetchRequest.sortDescriptors = [sortDesc]
        fetchRequest.predicate = predicate
        fetchRequest.fetchLimit = numberOfRoms
        
        let context = mainThreadContext
        let result = try? context.fetch(fetchRequest) as? [OEDBRom]
        return result ?? []
    }
    
    var lastPlayedRomsBySystem: [String : [OEDBRom]] {
        let roms = lastPlayedRoms
        var systemsSet = Set<OEDBSystem>(minimumCapacity: roms.count)
        for rom in roms {
            if let system = rom.game?.system {
                systemsSet.insert(system)
            }
        }
        
        let systems = Array(systemsSet)
        var result = [String : [OEDBRom]](minimumCapacity: systems.count)
        
        for system in systems {
            let romsForSystem = roms.filter {
                $0.game?.system == system
            }
            result[system.name] = romsForSystem
        }
        
        return result
    }
    
    // MARK: - Collection Editing
    
    func addNewCollection(_ name: String?) -> OEDBCollection {
        let context = mainThreadContext
        
        let name = name ?? {
            var name = NSLocalizedString("New Collection", comment: "Default collection name")
            
            let entityDescription = NSEntityDescription.entity(forEntityName: "AbstractCollection", in: context)
            let request = NSFetchRequest<NSFetchRequestResult>()
            request.entity = entityDescription
            request.fetchLimit = 1
            
            var numberSuffix = 0
            let baseName = name
            while (try? context.count(for: request)) != 0 {
                numberSuffix += 1
                name = "\(baseName) \(numberSuffix)"
                request.predicate = NSPredicate(format: "name == %@", name)
            }
            return name
        }()
        
        let collection = OEDBCollection.createObject(in: context)
        collection.name = name
        collection.save()
        
        return collection
    }
    
    func addNewSmartCollection(_ name: String?) -> OEDBSmartCollection {
        let context = mainThreadContext
        
        let name = name ?? {
            var name = NSLocalizedString("New Smart Collection", comment: "")
            
            let entityDescription = NSEntityDescription.entity(forEntityName: "AbstractCollection", in: context)
            let request = NSFetchRequest<NSFetchRequestResult>()
            request.entity = entityDescription
            request.fetchLimit = 1
            
            var numberSuffix = 0
            let baseName = name
            while (try? context.count(for: request)) != 0 {
                numberSuffix += 1
                name = "\(baseName) \(numberSuffix)"
                request.predicate = NSPredicate(format: "name == %@", name)
            }
            return name
        }()
        
        let collection = OEDBSmartCollection.createObject(in: context)
        collection.name = name
        collection.save()
        
        return collection
    }
    
    func addNewCollectionFolder(_ name: String?) -> OEDBCollectionFolder {
        let context = mainThreadContext
        
        let name = name ?? {
            var name = NSLocalizedString("New Folder", comment: "")
            
            let entityDescription = NSEntityDescription.entity(forEntityName: "AbstractCollection", in: context)
            let request = NSFetchRequest<NSFetchRequestResult>()
            request.entity = entityDescription
            request.fetchLimit = 1
            
            var numberSuffix = 0
            let baseName = name
            while (try? context.count(for: request)) != 0 {
                numberSuffix += 1
                name = "\(baseName) \(numberSuffix)"
                request.predicate = NSPredicate(format: "name == %@", name)
            }
            return name
        }()
        
        let collection = OEDBCollectionFolder.createObject(in: context)
        collection.name = name
        collection.save()
        
        return collection
    }
    
    // MARK: - Folders
    
    var databaseFolderURL: URL {
        let libraryFolderPath = UserDefaults.standard.string(forKey: OEDatabasePathKey)!
        let path = (libraryFolderPath as NSString).expandingTildeInPath
        return URL(fileURLWithPath: path, isDirectory: true)
    }
    
    var unsortedRomsFolderURL: URL {
        let unsortedFolderName = "unsorted"
        
        let result = romsFolderURL!.appendingPathComponent(unsortedFolderName, isDirectory: true)
        try? FileManager.default.createDirectory(at: result, withIntermediateDirectories: true)
        
        return result
    }
    
    @objc(romsFolderURLForSystem:) // OEImportOperation
    func romsFolderURL(for system: OEDBSystem) -> URL {
        let result = romsFolderURL!.appendingPathComponent(system.name, isDirectory: true)
        try? FileManager.default.createDirectory(at: result, withIntermediateDirectories: true)
        
        return result
    }
    
    var stateFolderURL: URL {
        if let urlString = UserDefaults.standard.string(forKey: OESaveStateFolderURLKey),
           let url = URL(string: urlString)
        {
            return url
        }
        
        let saveStateFolderName = "Save States"
        var result = try! FileManager.default.url(for: .applicationSupportDirectory, in: .userDomainMask, appropriateFor: nil, create: false)
        result.appendPathComponent("OpenEmu", isDirectory: true)
        result.appendPathComponent(saveStateFolderName, isDirectory: true)
        
        // In case one of the appended components is a symlink.
        result.resolveSymlinksInPath()
        
        // try? FileManager.default.createDirectory(at: result, withIntermediateDirectories: true)
        
        return result.standardized
    }
    
    func stateFolderURL(for system: OEDBSystem?) -> URL {
        let displayName = system?.plugin?.displayName ?? "Unkown System"
        
        let result = stateFolderURL.appendingPathComponent(displayName, isDirectory: true)
        try? FileManager.default.createDirectory(at: result, withIntermediateDirectories: true)
        
        return result
    }
    
    @nonobjc
    func stateFolderURL(for rom: OEDBRom) -> URL {
        var fileName = rom.fileName ??
                       rom.url?.lastPathComponent ??
                       rom.sourceURL?.lastPathComponent ??
                       "Unkown Game"
        
        fileName = (fileName as NSString).deletingPathExtension
        
        let result = stateFolderURL(for: rom.game?.system).appendingPathComponent(fileName, isDirectory: true)
        try? FileManager.default.createDirectory(at: result, withIntermediateDirectories: true)
        
        return result.standardized
    }
    
    var screenshotFolderURL: URL {
        if let urlString = UserDefaults.standard.string(forKey: OEScreenshotFolderURLKey),
           let url = URL(string: urlString)
        {
            return url
        }
        
        let screenshotFolderName = "Screenshots"
        var result = try! FileManager.default.url(for: .applicationSupportDirectory, in: .userDomainMask, appropriateFor: nil, create: false)
        result.appendPathComponent("OpenEmu", isDirectory: true)
        result.appendPathComponent(screenshotFolderName, isDirectory: true)
        
        try? FileManager.default.createDirectory(at: result, withIntermediateDirectories: true)
        
        return result.standardized
    }
    
    var coverFolderURL: URL {
        let coverFolderURL = databaseFolderURL.appendingPathComponent("Artwork", isDirectory: true)
        
        try? FileManager.default.createDirectory(at: coverFolderURL, withIntermediateDirectories: true)
        
        return coverFolderURL.standardized
    }
    
    var importQueueURL: URL {
        return databaseFolderURL.appendingPathComponent("Import Queue.db", isDirectory: false)
    }
}

// MARK: - Debug

#if DEBUG
extension OELibraryDatabase {
    
    @available(macOS 11.0, *)
    func dump(prefix: String = "***") {
        let subPrefix = prefix + "-----"
        os_log(.debug, log: OE_LOG_LIBRARY, "\(prefix) Beginning of database dump")
        
        os_log(.debug, log: OE_LOG_LIBRARY, "\(prefix) Database folder is \(self.databaseFolderURL.path)")
        os_log(.debug, log: OE_LOG_LIBRARY, "\(prefix) Number of collections is \(self.collections.count)")
        
        for collection in collections {
            //collection.dump(prefix: subPrefix)
            os_log(.debug, log: OE_LOG_LIBRARY, "\(subPrefix) Collection is \(collection.description)")
        }
        
        os_log(.debug, log: OE_LOG_LIBRARY, "\(prefix)")
        let systemCount = OEDBSystem.systemCount(in: mainThreadContext)
        os_log(.debug, log: OE_LOG_LIBRARY, "\(prefix) Number of systems is \(systemCount)")
        for system in OEDBSystem.allSystems(in: mainThreadContext) {
            system.dump(prefix: subPrefix)
        }
        
        os_log(.debug, log: OE_LOG_LIBRARY, "\(prefix)")
        os_log(.debug, log: OE_LOG_LIBRARY, "\(prefix) ALL ROMs")
        for rom in allROMsForDump() {
            rom.dump(prefix: subPrefix)
        }
        
        os_log(.debug, log: OE_LOG_LIBRARY, "\(prefix) end of database dump")
    }
    
    func allROMsForDump() -> [OEDBRom] {
        let fetchRequest = OEDBRom.fetchRequest()
        let result = try? mainThreadContext.fetch(fetchRequest) as? [OEDBRom]
        return result ?? []
    }
}
#endif
