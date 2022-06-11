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

@objc
@objcMembers
class OELibraryDatabase: OELibraryDatabaseObjC {
    
    enum Errors: LocalizedError {
        case folderNotFound
        case noModelToGenerateStoreFrom
        
        var errorDescription: String? {
            switch self {
            case .folderNotFound:
                return NSLocalizedString("The OpenEmu Library could not be found.", comment: "")
            case .noModelToGenerateStoreFrom:
                return NSLocalizedString("No model to generate a store from.", comment: "")
            }
        }
    }
    
    @objc(defaultDatabase)
    private(set) static var `default`: OELibraryDatabase?
    
    private var databaseURL: URL
    
    private var _writerContext: NSManagedObjectContext?
    var writerContext: NSManagedObjectContext {
        _writerContext!
    }
    private var _mainThreadContext: NSManagedObjectContext?
    var mainThreadContext: NSManagedObjectContext {
        _mainThreadContext!
    }
    
    private(set) lazy var importer = ROMImporter(database: self)
    
    // Exposed for library migration
    var persistentStoreCoordinator: NSPersistentStoreCoordinator?
    
    private lazy var managedObjectModel: NSManagedObjectModel? = {
        let modelURL = Bundle.main.url(forResource: "OEDatabase", withExtension: "momd")!
        if let managedObjectModel = NSManagedObjectModel(contentsOf: modelURL) {
            return managedObjectModel
        } else {
            return nil
        }
    }()
    
    private var undoManager: UndoManager? {
        nil //mainThreadContext.undoManager
    }
    
    // MARK: -
    
    init(url: URL) {
        databaseURL = url
        super.init()
        
        let nc = NotificationCenter.default
        nc.addObserver(self, selector: #selector(applicationWillTerminate), name: NSApplication.willTerminateNotification, object: NSApp)
        nc.addObserver(self, selector: #selector(managedObjectContextDidSave), name: .NSManagedObjectContextDidSave, object: nil)
    }
    
    deinit {
        NotificationCenter.default.removeObserver(self)
        
        os_log(.debug, log: .library, "Destroying library database")
    }
    
    @objc func applicationWillTerminate(_ notification: Notification) {
        importer.saveQueue()
        
        do {
            try writerContext.save()
        } catch {
            os_log(.error, log: .library, "Could not save database: %{public}@", error as NSError)
            NSApp.presentError(error)
        }
    }
    
    @objc func managedObjectContextDidSave(_ notification: Notification) {
        guard let context = notification.object as? NSManagedObjectContext else { return }
        if context == mainThreadContext {
            // Write changes to disk (in background)
            let writerContext = writerContext
            writerContext.perform {
                try? writerContext.save()
            }
        }
        else if context == writerContext,
                context.userInfo[OEManagedObjectContextHasDirectChangesKey] as? Bool == true
        {
            context.userInfo[OEManagedObjectContextHasDirectChangesKey] = false
            let mainThreadContext = mainThreadContext
            mainThreadContext.perform {
                mainThreadContext.mergeChanges(fromContextDidSave: notification)
                try? mainThreadContext.save()
            }
        }
    }
    
    // MARK: -
    
    static func load(from url: URL) throws {
        os_log(.info, log: .library, "Load library database from '%{public}@'", url.path)
        
        var isDir = ObjCBool(false)
        if !FileManager.default.fileExists(atPath: url.path, isDirectory: &isDir) || !isDir.boolValue {
            os_log(.error, log: .library, "Library database folder not found")
            throw Errors.folderNotFound
        }
        
        let defaultDB = OELibraryDatabase(url: url)
        `default` = defaultDB
        
        do {
            try defaultDB.loadPersistantStore()
        } catch {
            `default` = nil
            throw error
        }
        defaultDB.loadManagedObjectContext()
        defaultDB.createInitialItemsIfNeeded()
        
        let path = (defaultDB.databaseURL.path as NSString).abbreviatingWithTildeInPath
        UserDefaults.standard.set(path, forKey: OEDatabasePathKey)
        
        let romImporter = defaultDB.importer
        romImporter.loadQueue()
        
        DispatchQueue.main.asyncAfter(deadline: DispatchTime.now() + .seconds(2)) {
            defaultDB.startOpenVGDBSync()
            romImporter.start()
        }
    }
    
    private func loadPersistantStore() throws {
        guard let mom = managedObjectModel else {
            os_log(.error, log: .library, "No model to generate a store from")
            throw Errors.noModelToGenerateStoreFrom
        }
        let coordinator = NSPersistentStoreCoordinator(managedObjectModel: mom)
        persistentStoreCoordinator = coordinator
        
        let url = databaseURL.appendingPathComponent(OEDatabaseFileName, isDirectory: false)
        
        let options = [
            NSMigratePersistentStoresAutomaticallyOption : false,
            NSInferMappingModelAutomaticallyOption       : false,
        ]
        
        do {
            try coordinator.addPersistentStore(ofType: NSSQLiteStoreType, configurationName: nil, at: url, options: options)
        } catch {
            persistentStoreCoordinator = nil
            throw error
        }
    }
    
    private func loadManagedObjectContext() {
        // Setup a private managed object context
        let writerContext = NSManagedObjectContext(concurrencyType: .privateQueueConcurrencyType)
        writerContext.name = "OELibraryDatabase.writer"
        writerContext.mergePolicy = NSMergePolicy(merge: .mergeByPropertyObjectTrumpMergePolicyType)
        writerContext.retainsRegisteredObjects = true
        writerContext.persistentStoreCoordinator = persistentStoreCoordinator
        writerContext.userInfo[OELibraryDatabaseUserInfoKey] = self
        writerContext.userInfo["name"] = "main"
        writerContext.undoManager = nil
        _writerContext = writerContext
        
        // Setup a moc for use on main thread
        let mainThreadContext = NSManagedObjectContext(concurrencyType: .mainQueueConcurrencyType)
        mainThreadContext.parent = writerContext
        mainThreadContext.name = "OELibraryDatabase.mainThread"
        mainThreadContext.userInfo[OELibraryDatabaseUserInfoKey] = self
        mainThreadContext.userInfo["name"] = "UI"
        mainThreadContext.undoManager = nil
        _mainThreadContext = mainThreadContext
        
        // remeber last location as database path
        let path = (databaseURL.path as NSString).abbreviatingWithTildeInPath
        UserDefaults.standard.set(path, forKey: OEDatabasePathKey)
        
        os_log(.debug, log: .library, "ROMs folder URL: %{public}@", romsFolderURL?.path ?? "nil")
    }
    
    private func createInitialItemsIfNeeded() {
        let context = mainThreadContext
        
        let smartCollections = OEDBSmartCollection.allObjects(in: context)
        if !smartCollections.isEmpty {
            return
        }
        
        let recentlyAdded = OEDBSmartCollection.createObject(in: context)
        recentlyAdded.name = "Recently Added"
        recentlyAdded.save()
    }
    
    // MARK: - Creating child contexts
    
    func makeChildContext() -> NSManagedObjectContext {
        let context = NSManagedObjectContext(concurrencyType: .privateQueueConcurrencyType)
        context.parent = mainThreadContext
        context.undoManager = nil
        context.mergePolicy = mainThreadContext.mergePolicy
        context.userInfo[OELibraryDatabaseUserInfoKey] = self
        
        return context
    }
    
    func makeWriterChildContext() -> NSManagedObjectContext {
        let context = NSManagedObjectContext(concurrencyType: .privateQueueConcurrencyType)
        context.parent = writerContext
        context.undoManager = nil
        context.mergePolicy = writerContext.mergePolicy
        context.userInfo[OELibraryDatabaseUserInfoKey] = self
        
        return context
    }
    
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
                os_log(.error, log: .library, "Error executing fetch request to get rom by md5: %{public}@", error as NSError)
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
    
    var romsFolderURL: URL? {
        if let persistentStore = persistentStoreCoordinator?.persistentStores.last,
           let metadata = persistentStoreCoordinator?.metadata(for: persistentStore),
           let urlString = metadata[OELibraryRomsFolderURLKey] as? String
        {
            if !urlString.contains("file://") {
                return URL(string: urlString, relativeTo: databaseFolderURL)
            } else {
                return URL(string: urlString)
            }
        } else {
            let result = databaseFolderURL.appendingPathComponent("roms", isDirectory: true)
            try? FileManager.default.createDirectory(at: result, withIntermediateDirectories: true)
            setRomsFolderURL(result)
            
            return result
        }
    }
    
    func setRomsFolderURL(_ url: URL) {
        guard let persistentStore = persistentStoreCoordinator?.persistentStores.last,
              var metadata = persistentStore.metadata
        else { return }
        
        let databaseFolderURL = databaseFolderURL
        
        if url.isSubpath(of: databaseFolderURL) {
            let urlString = (url.absoluteString as NSString).substring(from: (databaseFolderURL.absoluteString as NSString).length)
            metadata[OELibraryRomsFolderURLKey] = "./" + urlString
        } else {
            metadata[OELibraryRomsFolderURLKey] = url.absoluteString
        }
        
        // Using the instance method sets the metadata for the current store in memory, while
        // using the class method writes to disk immediately. Calling both seems redundant
        // but is the only way I found that works.
        //
        // Also see discussion at http://www.cocoabuilder.com/archive/cocoa/295041-setting-not-saving-nspersistentdocument-metadata-changes-file-modification-date.html
        persistentStoreCoordinator?.setMetadata(metadata, for: persistentStore)
        try? NSPersistentStoreCoordinator.setMetadata(metadata, forPersistentStoreOfType: persistentStore.type, at: persistentStore.url!)
        writerContext.perform {
            try? self.writerContext.save()
        }
    }
    
    var unsortedRomsFolderURL: URL {
        let unsortedFolderName = "unsorted"
        
        let result = romsFolderURL!.appendingPathComponent(unsortedFolderName, isDirectory: true)
        try? FileManager.default.createDirectory(at: result, withIntermediateDirectories: true)
        
        return result
    }
    
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
        os_log(.debug, log: .library, "\(prefix) Beginning of database dump")
        
        os_log(.debug, log: .library, "\(prefix) Database folder is \(self.databaseFolderURL.path)")
        os_log(.debug, log: .library, "\(prefix) Number of collections is \(self.collections.count)")
        
        for collection in collections {
            //collection.dump(prefix: subPrefix)
            os_log(.debug, log: .library, "\(subPrefix) Collection is \(collection.description)")
        }
        
        os_log(.debug, log: .library, "\(prefix)")
        let systemCount = OEDBSystem.systemCount(in: mainThreadContext)
        os_log(.debug, log: .library, "\(prefix) Number of systems is \(systemCount)")
        for system in OEDBSystem.allSystems(in: mainThreadContext) {
            system.dump(prefix: subPrefix)
        }
        
        os_log(.debug, log: .library, "\(prefix)")
        os_log(.debug, log: .library, "\(prefix) ALL ROMs")
        for rom in allROMsForDump() {
            rom.dump(prefix: subPrefix)
        }
        
        os_log(.debug, log: .library, "\(prefix) end of database dump")
    }
    
    func allROMsForDump() -> [OEDBRom] {
        let fetchRequest = OEDBRom.fetchRequest()
        let result = try? mainThreadContext.fetch(fetchRequest) as? [OEDBRom]
        return result ?? []
    }
}
#endif
