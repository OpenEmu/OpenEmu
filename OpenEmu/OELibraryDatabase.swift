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

@objc extension OELibraryDatabase {
    static var locationDidChangeNotification: NSString {
        return Notification.Name.libraryLocationDidChange as NSString
    }
}

extension Notification.Name {
    static let libraryDidLoad = Notification.Name("OELibraryDidLoadNotificationName")
    static let libraryLocationDidChange = Notification.Name("OELibraryLocationDidChangeNotificationName")
}

@objc
@objcMembers
final class OELibraryDatabase: NSObject {
    
    enum Errors: LocalizedError {
        case folderNotFound
        
        var errorDescription: String? {
            switch self {
            case .folderNotFound:
                return NSLocalizedString("The OpenEmu Library could not be found.", comment: "")
            }
        }
    }
    
    static let databaseFileExtension = "storedata"
    static let databaseFileName = "Library.\(databaseFileExtension)"
    
    static let databasePathKey = "databasePath"
    static let defaultDatabasePathKey = "defaultDatabasePath"
    
    static let saveStateFolderURLKey = "saveStateFolder"
    static let screenshotFolderURLKey = "screenshotFolder"
    
    static let libraryRomsFolderURLKey = "romsFolderURL"
    
    static let libraryDatabaseUserInfoKey = "OELibraryDatabase"
    static let managedObjectContextHasDirectChangesUserInfoKey = "hasDirectChanges"
    
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
    
    private lazy var managedObjectModel: NSManagedObjectModel = {
        let modelURL = Bundle.main.url(forResource: "OEDatabase", withExtension: "momd")!
        let managedObjectModel = NSManagedObjectModel(contentsOf: modelURL)!
        return managedObjectModel
    }()
    
    private var undoManager: UndoManager? {
        nil //mainThreadContext.undoManager
    }
    
    private static let openVGDBSyncBatchSize = 5
    private lazy var syncThread: Thread = makeSyncThread()
    
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
                context.userInfo[Self.managedObjectContextHasDirectChangesUserInfoKey] as? Bool == true
        {
            context.userInfo[Self.managedObjectContextHasDirectChangesUserInfoKey] = false
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
        UserDefaults.standard.set(path, forKey: Self.databasePathKey)
        
        let romImporter = defaultDB.importer
        romImporter.loadQueue()
        
        DispatchQueue.main.asyncAfter(deadline: DispatchTime.now() + .seconds(2)) {
            defaultDB.startOpenVGDBSync()
            romImporter.start()
        }
    }
    
    private func loadPersistantStore() throws {
        let mom = managedObjectModel
        let coordinator = NSPersistentStoreCoordinator(managedObjectModel: mom)
        persistentStoreCoordinator = coordinator
        
        let url = databaseURL.appendingPathComponent(Self.databaseFileName, isDirectory: false)
        
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
        writerContext.userInfo[Self.libraryDatabaseUserInfoKey] = self
        writerContext.userInfo["name"] = "main"
        writerContext.undoManager = nil
        _writerContext = writerContext
        
        // Setup a moc for use on main thread
        let mainThreadContext = NSManagedObjectContext(concurrencyType: .mainQueueConcurrencyType)
        mainThreadContext.parent = writerContext
        mainThreadContext.name = "OELibraryDatabase.mainThread"
        mainThreadContext.userInfo[Self.libraryDatabaseUserInfoKey] = self
        mainThreadContext.userInfo["name"] = "UI"
        mainThreadContext.undoManager = nil
        _mainThreadContext = mainThreadContext
        
        // remeber last location as database path
        let path = (databaseURL.path as NSString).abbreviatingWithTildeInPath
        UserDefaults.standard.set(path, forKey: Self.databasePathKey)
        
        os_log(.debug, log: .library, "ROMs folder URL: %{public}@", romsFolderURL?.path ?? "nil")
    }
    
    private func createInitialItemsIfNeeded() {
        let context = mainThreadContext
        
        let smartCollections = context.allObjects(ofType: OEDBSmartCollection.self)
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
        context.userInfo[Self.libraryDatabaseUserInfoKey] = self
        
        return context
    }
    
    func makeWriterChildContext() -> NSManagedObjectContext {
        let context = NSManagedObjectContext(concurrencyType: .privateQueueConcurrencyType)
        context.parent = writerContext
        context.undoManager = nil
        context.mergePolicy = writerContext.mergePolicy
        context.userInfo[Self.libraryDatabaseUserInfoKey] = self
        
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
        
        let smartCollections = context.allObjects(ofType: OEDBSmartCollection.self, sortedBy: [sortDescriptor])
        collectionsArray.append(contentsOf: smartCollections)
        
        let collections = context.allObjects(ofType: OEDBCollection.self, sortedBy: [sortDescriptor])
        collectionsArray.append(contentsOf: collections)
        
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
        return result?.first
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
        let libraryFolderPath = UserDefaults.standard.string(forKey: Self.databasePathKey)!
        let path = (libraryFolderPath as NSString).expandingTildeInPath
        return URL(fileURLWithPath: path, isDirectory: true)
    }
    
    var romsFolderURL: URL? {
        if let persistentStore = persistentStoreCoordinator?.persistentStores.last,
           let metadata = persistentStoreCoordinator?.metadata(for: persistentStore),
           let urlString = metadata[Self.libraryRomsFolderURLKey] as? String
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
            metadata[Self.libraryRomsFolderURLKey] = "./" + urlString
        } else {
            metadata[Self.libraryRomsFolderURLKey] = url.absoluteString
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
        if let urlString = UserDefaults.standard.string(forKey: Self.saveStateFolderURLKey),
           let url = URL(string: urlString)
        {
            return url
        }
        
        let saveStateFolderName = "Save States"
        let result = URL.oeApplicationSupportDirectory.appendingPathComponent(saveStateFolderName, isDirectory: true)
            .resolvingSymlinksInPath() // In case one of the appended components is a symlink.
        
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
        if let urlString = UserDefaults.standard.string(forKey: Self.screenshotFolderURLKey),
           let url = URL(string: urlString)
        {
            return url
        }
        
        let screenshotFolderName = "Screenshots"
        let result = URL.oeApplicationSupportDirectory.appendingPathComponent(screenshotFolderName, isDirectory: true)
        
        try? FileManager.default.createDirectory(at: result, withIntermediateDirectories: true)
        
        return result.standardized
    }
    
    var coverFolderURL: URL {
        let coverFolderURL = databaseFolderURL.appendingPathComponent("Artwork", isDirectory: true)
        
        try? FileManager.default.createDirectory(at: coverFolderURL, withIntermediateDirectories: true)
        
        return coverFolderURL.standardized
    }
    
    var importQueueURL: URL {
        return databaseFolderURL.appendingPathComponent("ImportQueue.plist", isDirectory: false)
    }
    
    // MARK: - OpenVGDB Sync
    
    func startOpenVGDBSync() {
        objc_sync_enter(syncThread)
        defer { objc_sync_exit(syncThread) }
        
        if syncThread.isFinished {
            syncThread = makeSyncThread()
        }
    }
    
    private func makeSyncThread() -> Thread {
        let syncThread = Thread(block: { [weak self] in self?.openVGDBSyncThreadMain() })
        syncThread.name = "OEVGDBSync"
        syncThread.qualityOfService = .utility
        syncThread.start()
        
        return syncThread
    }
    
    @objc private func openVGDBSyncThreadMain() {
        let romKeys = [#keyPath(OEDBRom.md5), #keyPath(OEDBRom.url), #keyPath(OEDBRom.header), #keyPath(OEDBRom.serial), #keyPath(OEDBRom.archiveFileIndex)]
        let gameKeys = [#keyPath(OEDBGame.permanentID)]
        let systemKeys = [#keyPath(OEDBSystem.systemIdentifier)]
        
        let request = OEDBGame.fetchRequest()
        let predicate = NSPredicate(format: "status == %d", OEDBGame.Status.processing.rawValue)
        request.fetchLimit = Self.openVGDBSyncBatchSize
        request.predicate = predicate
        
        let context = mainThreadContext
        
        var count = 0
        context.performAndWait {
            count = (try? context.count(for: request)) ?? 0
        }
        
        while count != 0 {
            var games: [[String : Any]] = []
            
            context.performAndWait {
                let gamesObjects = (try? context.fetch(request)) as? [OEDBGame] ?? []
                for game in gamesObjects {
                    guard let rom = game.defaultROM,
                          let system = game.system
                    else { continue }
                    
                    let gameInfo = game.dictionaryWithValues(forKeys: gameKeys)
                    let romInfo = rom.dictionaryWithValues(forKeys: romKeys)
                    let systemInfo = system.dictionaryWithValues(forKeys: systemKeys)
                    
                    var info = gameInfo
                    info.merge(romInfo) { (old, _) in return old }
                    info.merge(systemInfo) { (old, _) in return old }
                    
                    games.append(info)
                }
            }
            
            for i in 0 ..< games.count {
                let gameInfo = games[i]
                
                var result = GameInfoHelper.shared.gameInfo(withDictionary: gameInfo)
                
                // Trim the gameTitle for imported m3u's so they look nice
                if let resultGameTitle = result["gameTitle"] as? NSString,
                   let gameInfoURL = gameInfo[#keyPath(OEDBRom.url)] as? URL,
                   gameInfoURL.pathExtension.lowercased() == "m3u"
                {
                    // RegEx pattern match the parentheses e.g. " (Disc 1)" and update dictionary with trimmed gameTitle string
                    let newGameTitle = resultGameTitle.replacingOccurrences(of: "\\ \\(Disc.*\\)", with: "", options: .regularExpression, range: NSRange(location: 0, length: resultGameTitle.length))
                    
                    result["gameTitle"] = newGameTitle
                }
                
                let objectID = gameInfo[#keyPath(OEDBGame.permanentID)] as! NSManagedObjectID
                var dict: [String : Any] = ["objectID" : objectID, #keyPath(OEDBGame.status) : OEDBGame.Status.ok.rawValue]
                
                if !result.isEmpty {
                    dict.merge(result) { (old, _) in return old }
                }
                
                if let boxImageURLString = dict["boxImageURL"] as? String,
                   let boxImageURL = URL(string: boxImageURLString),
                   let image = OEDBImage.prepareImage(with: boxImageURL) {
                    dict["image"] = image
                }
                
                Thread.sleep(forTimeInterval: 0.5)
                
                games[i] = dict
            }
            
            var previousBoxImages: [NSManagedObjectID] = []
            
            context.performAndWait {
                for i in 0 ..< games.count {
                    var gameInfo = games[i]
                    let objectID = gameInfo.removeValue(forKey: "objectID") as! NSManagedObjectID
                    let imageDictionary = gameInfo.removeValue(forKey: "image") as? [String : Any]
                    
                    let md5 = gameInfo.removeValue(forKey: "md5") as? String
                    let serial = gameInfo.removeValue(forKey: "serial") as? String
                    let header = gameInfo.removeValue(forKey: "header") as? String
                    
                    gameInfo.removeValue(forKey: "boxImageURL")
                    
                    let game = OEDBGame.object(with: objectID, in: context)
                    game?.setValuesForKeys(gameInfo)
                    
                    if let imageDictionary = imageDictionary,
                       let game = game {
                        let image = OEDBImage.createImage(with: imageDictionary)
                        if let previousImage = game.boxImage {
                            previousBoxImages.append(previousImage.permanentID)
                        }
                        game.boxImage = image
                    }
                    
                    guard let rom = game?.defaultROM else { continue }
                    
                    if rom.md5 == nil,
                       let md5 = md5 {
                        rom.md5 = md5.lowercased()
                    }
                    if rom.serial == nil,
                       let serial = serial {
                        rom.md5 = serial
                    }
                    if rom.header == nil,
                       let header = header {
                        rom.header = header
                    }
                }
                
                try? context.save()
                count = (try? context.count(for: request)) ?? 0
            }
            
            context.perform {
                for objectID in previousBoxImages {
                    if let item = OEDBImage.object(with: objectID, in: context) {
                        context.delete(item)
                    }
                }
                try? context.save()
            }
        }
    }
}

// MARK: - Debug

#if DEBUG
extension OELibraryDatabase {
    
    @available(macOS 11.0, *)
    func dump(prefix: String = "***") {
        let subPrefix = prefix + "-----"
        Logger.library.debug("\(prefix) Beginning of database dump")
        
        Logger.library.debug("\(prefix) Database folder is \(self.databaseFolderURL.path)")
        Logger.library.debug("\(prefix) Number of collections is \(self.collections.count)")
        
        for collection in collections {
            //collection.dump(prefix: subPrefix)
            Logger.library.debug("\(subPrefix) Collection is \(collection.description)")
        }
        
        Logger.library.debug("\(prefix)")
        let systemCount = OEDBSystem.systemCount(in: mainThreadContext)
        Logger.library.debug("\(prefix) Number of systems is \(systemCount)")
        for system in OEDBSystem.allSystems(in: mainThreadContext) {
            system.dump(prefix: subPrefix)
        }
        
        Logger.library.debug("\(prefix)")
        Logger.library.debug("\(prefix) ALL ROMs")
        for rom in allROMsForDump() {
            rom.dump(prefix: subPrefix)
        }
        
        Logger.library.debug("\(prefix) end of database dump")
    }
    
    func allROMsForDump() -> [OEDBRom] {
        let fetchRequest = OEDBRom.fetchRequest()
        let result = try? mainThreadContext.fetch(fetchRequest) as? [OEDBRom]
        return result ?? []
    }
}
#endif
