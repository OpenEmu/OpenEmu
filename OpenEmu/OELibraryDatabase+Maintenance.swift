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
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES
// LOSS OF USE, DATA, OR PROFITS  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import Cocoa

extension OELibraryDatabase {
    
    func cleanupAutoSaveStates() {
        let context = mainThreadContext
        
        let allRoms = OEDBRom.allObjects(in: context) as! [OEDBRom]
        
        for rom in allRoms {
            let states = rom.saveStates.sorted {
                guard let d1 = $0.timestamp,
                      let d2 = $1.timestamp
                else { return false }
                return d1.compare(d2) == .orderedDescending
            }
            let autosaves = states.filter { $0.name.starts(with: OEDBSaveState.autosaveName) }
            
            var autosave: OEDBSaveState?
            for i in 0 ..< autosaves.count {
                let state = autosaves[i]
                if /* DISABLES CODE */ true { // TODO: fix -checkFilesAvailable
                    if autosave != nil {
                        state.name = NSLocalizedString("Recovered Auto Save", comment: "Recovered auto save name")
                        state.moveToDefaultLocation()
                    } else {
                        autosave = state
                    }
                } else {
                    state.delete()
                }
            }
            
            autosave?.moveToDefaultLocation()
        }
        try? context.save()
    }
    
    func cleanupSaveStates() {
        guard let database = OELibraryDatabase.default else { return }
        let context = mainThreadContext
        
        var allSaveStates = OEDBSaveState.allObjects(in: context) as! [OEDBSaveState]
        
        // remove invalid save states
        allSaveStates.forEach { $0.deleteAndRemoveFilesIfInvalid() }
        try? context.save()
        
        // add untracked save states
        let statesFolder = database.stateFolderURL
        let fm = FileManager.default
        let enumerator = fm.enumerator(at: statesFolder, includingPropertiesForKeys: nil)
        if let enumerator = enumerator {
            for element in enumerator {
                guard let url = element as? URL else {
                    continue
                }
                if url.pathExtension == OEDBSaveState.bundleExtension {
                    OEDBSaveState.createSaveState(byImportingBundleURL: url, into: context)
                }
            }
        }
        
        // remove invalid save states, again
        allSaveStates = OEDBSaveState.allObjects(in: context) as! [OEDBSaveState]
        allSaveStates.forEach { $0.deleteAndRemoveFilesIfInvalid() }
        try? context.save()
        
        // remove duplicates
        allSaveStates = OEDBSaveState.allObjects(in: context) as! [OEDBSaveState]
        allSaveStates = (allSaveStates as NSArray).sortedArray(using: [
            NSSortDescriptor(key: "rom.md5", ascending: true),
            NSSortDescriptor(key: "coreIdentifier", ascending: true),
            NSSortDescriptor(key: "timestamp", ascending: true),
        ]) as! [OEDBSaveState]
        var lastState: OEDBSaveState?
        for saveState in allSaveStates {
            if let lastState = lastState,
               lastState.rom == saveState.rom,
               lastState.timestamp == saveState.timestamp,
               lastState.coreIdentifier == saveState.coreIdentifier
            {
                let currentHash = try? FileManager.default.hashFile(at: saveState.dataFileURL)
                let previousHash = try? FileManager.default.hashFile(at: lastState.dataFileURL)
                
                if currentHash == previousHash {
                    if lastState.url == saveState.url {
                        lastState.delete()
                    } else {
                        lastState.deleteAndRemoveFiles()
                    }
                }
            }
            lastState = saveState
        }
        try? context.save()
        
        // move to default location
        allSaveStates = OEDBSaveState.allObjects(in: context) as! [OEDBSaveState]
        for saveState in allSaveStates {
            if !saveState.moveToDefaultLocation() {
                NSLog("SaveState is still corrupt!")
                DLog("\(saveState.url)")
            }
        }
    }
    
    // removes all image objects that are neither on disc nor have a source
    func removeUselessImages() {
        let context = mainThreadContext
        
        let request = OEDBImage.fetchRequest()
        let predicate = NSPredicate(format: "relativePath == nil and source == nil")
        request.predicate = predicate
        
        var images: [OEDBImage]
        do {
            images = try context.fetch(request) as? [OEDBImage] ?? []
        } catch {
            DLog("Could not execute fetch request: \(error)")
            return
        }
        
        images.forEach { $0.delete() }
        try? context.save()
        NSLog("Deleted \(images.count) images!")
    }
    
    func removeArtworkWithRemoteBacking() {
        let context = mainThreadContext
        
        let request = OEDBImage.fetchRequest()
        let predicate = NSPredicate(format: "source != nil")
        request.predicate = predicate
        
        var images: [OEDBImage]
        do {
            images = try context.fetch(request) as? [OEDBImage] ?? []
        } catch {
            DLog("Could not execute fetch request: \(error)")
            return
        }
        
        var count = 0
        for image in images {
            // make sure we only delete image files that can be downloaded automatically!
            if image.sourceURL != nil,
               let fileURL = image.imageURL
            {
                try? FileManager.default.removeItem(at: fileURL)
                image.relativePath = nil
                count += 1
            }
        }
        
        try? context.save()
        NSLog("Deleted \(count) image files!")
    }
    
    func syncGamesWithoutArtwork() {
        let context = mainThreadContext
        
        let request = OEDBGame.fetchRequest()
        let predicate = NSPredicate(format: "boxImage == nil")
        request.predicate = predicate
        
        var games: [OEDBGame]
        do {
            games = try context.fetch(request) as? [OEDBGame] ?? []
        } catch {
            DLog("Could not execute fetch request: \(error)")
            return
        }
        
        games.forEach { $0.requestInfoSync() }
        NSLog("Found \(games.count) games")
    }
    
    func downloadMissingArtwork() {
        let alert = OEAlert()
        alert.messageText = NSLocalizedString("While performing this operation OpenEmu will be unresponsive.", tableName: "Debug", comment: "")
        alert.defaultButtonTitle = NSLocalizedString("Continue", comment: "")
        alert.alternateButtonTitle = NSLocalizedString("Cancel", comment: "")
        if alert.runModal() != .alertFirstButtonReturn {
            return
        }
        
        let context = mainThreadContext
        
        let request = OEDBImage.fetchRequest()
        let predicate = NSPredicate(format: "source != nil")
        request.predicate = predicate
        
        var images: [OEDBImage]
        do {
            images = try context.fetch(request) as? [OEDBImage] ?? []
        } catch {
            DLog("Could not execute fetch request: \(error)")
            return
        }
        
        var count = 0
        for image in images {
            if !image.isLocalImageAvailable {
                autoreleasepool {
                    if let source = image.source,
                       let newInfo = OEDBImage.prepareImage(withURLString: source),
                       newInfo["relativePath"] != nil
                    {
                        image.width = Float(newInfo["width"] as! CGFloat)
                        image.height = Float(newInfo["height"] as! CGFloat)
                        image.relativePath = (newInfo["relativePath"] as! String)
                        image.format = Int16((newInfo["format"] as! NSBitmapImageRep.FileType).rawValue)
                        count += 1
                    }
                }
            }
            
            if count % 20 == 0 {
                try? context.save()
            }
        }
        
        try? context.save()
        NSLog("Downloaded \(count) image files!")
    }
    
    func removeUntrackedImageFiles() {
        guard let database = OELibraryDatabase.default else { return }
        let context = mainThreadContext
        
        let artworkDirectory = database.coverFolderURL
        
        guard let artworkFiles = try? FileManager.default.contentsOfDirectory(at: artworkDirectory, includingPropertiesForKeys: nil)
        else { return }
        
        var artwork = Set<URL>(artworkFiles)
        
        let request = OEDBImage.fetchRequest()
        let predicate = NSPredicate(format: "relativePath != nil")
        request.predicate = predicate
        
        var images: [OEDBImage]
        do {
            images = try context.fetch(request) as? [OEDBImage] ?? []
        } catch {
            DLog("Could not execute fetch request: \(error)")
            return
        }
        
        for image in images {
            if let url = image.imageURL {
                artwork.remove(url)
            }
        }
        
        for untrackedFile in artwork {
            try? FileManager.default.removeItem(at: untrackedFile)
        }
        
        NSLog("Removed \(artwork.count) unknown files from artwork directory")
    }
    
    func cleanupHashes() {
        let context = mainThreadContext
        
        let allRoms = OEDBRom.allObjects(in: context) as! [OEDBRom]
        
        for rom in allRoms {
            rom.md5 = rom.md5?.lowercased()
        }
        
        try? context.save()
    }
    
    func removeDuplicatedRoms() {
        let context = mainThreadContext
        
        var allRoms = OEDBRom.allObjects(in: context) as! [OEDBRom]
        allRoms.sort {
            guard let s1 = $0.md5,
                  let s2 = $1.md5
            else { return false }
            return s1.compare(s2) == .orderedAscending
        }
        var lastRom: OEDBRom?
        var romsToDelete: [OEDBRom] = []
        for rom in allRoms {
            if let lastRom = lastRom,
               rom.md5 == lastRom.md5
            {
                rom.saveStates.formUnion(lastRom.saveStates)
                romsToDelete.append(lastRom)
            }
            lastRom = rom
        }
        
        for rom in romsToDelete {
            rom.game?.delete(moveToTrash: false, keepSaveStates: true)
            rom.delete(moveToTrash: false, keepSaveStates: false)
        }
        
        try? context.save()
        NSLog("\(romsToDelete.count) roms deleted")
    }
    
    func cancelCoverArtSync() {
        let context = mainThreadContext
        
        let request = OEDBGame.fetchRequest()
        let predicate = NSPredicate(format: "status == \(OEDBGame.Status.processing.rawValue)")
        request.predicate = predicate
        
        var games: [OEDBGame]
        do {
            games = try context.fetch(request) as? [OEDBGame] ?? []
        } catch {
            DLog("Could not execute fetch request: \(error)")
            return
        }
        
        games.forEach { $0.status = .ok }
        try? context.save()
        NSLog("Cancelled cover art download for \(games.count) games")
    }
    
    func sanityCheck() {
        let context = mainThreadContext
        
        var counts = (0, 0)
        
        NSLog("= START SANITY CHECK =")
        
        var allRoms = OEDBRom.allObjects(in: context) as! [OEDBRom]
        
        // Look for roms without games
        counts.0 = 0
        for rom in allRoms {
            if rom.game == nil {
                counts.0 += 1
            }
        }
        
        if counts.0 != 0 {
            NSLog("Found \(counts.0) roms without game!")
        }
        
        // Look for roms referencing the same file
        allRoms.sort {
            guard let s1 = $0.location,
                  let s2 = $1.location
            else { return false }
            return s1.compare(s2) == .orderedAscending
        }
        counts.0 = 0
        var lastRom: OEDBRom?
        for rom in allRoms {
            if rom.location == lastRom?.location,
               rom.location != nil,
               rom.location != ""
            {
                counts.0 += 1
            }
            lastRom = rom
        }
        if counts.0 != 0 {
            NSLog("Found \(counts.0) duplicated roms!")
        }
        
        // Look for roms with same hash
        allRoms.sort {
            guard let s1 = $0.md5,
                  let s2 = $1.md5
            else { return false }
            return s1.compare(s2) == .orderedAscending
        }
        counts.0 = 0
        counts.1 = 0
        lastRom = nil
        for rom in allRoms {
            if rom.md5 == lastRom?.md5,
               rom.md5 != nil,
               rom.md5 != ""
            {
                counts.0 += 1
            }
            if rom.md5?.lowercased() != rom.md5 {
                counts.1 += 1
            }
            lastRom = rom
        }
        if counts.0 != 0 {
            NSLog("Found \(counts.0) duplicated roms!")
        }
        if counts.1 != 0 {
            NSLog("Found \(counts.1) roms with wrong case in hash!")
        }
        
        // Look for games without roms
        let allGames = OEDBGame.allObjects(in: context) as! [OEDBGame]
        
        counts.0 = 0
        for game in allGames {
            if game.roms.count == 0 {
                counts.0 += 1
            }
        }
        
        if counts.0 != 0 {
            NSLog("Found \(counts.0) games without rom!")
        }
        
        
        // Look for save states without rom
        let allStates = OEDBSaveState.allObjects(in: context) as! [OEDBSaveState]
        counts.0 = 0
        counts.1 = 0
        for state in allStates {
            if state.rom == nil {
                counts.0 += 1
            }
            if !state.isValid {
                counts.1 += 1
            }
        }
        if counts.0 != 0 {
            NSLog("Found \(counts.0) save states without rom!")
        }
        if counts.1 != 0 {
            NSLog("Found \(counts.1) invalid save states!")
        }
        
        
        // Look for images without game
        let allImages = OEDBImage.allObjects(in: context) as! [OEDBImage]
        counts.0 = 0
        for image in allImages {
            if image.Box == nil {
                counts.0 += 1
            }
        }
        if counts.0 != 0 {
            NSLog("Found \(counts.0) images without game!")
        }
        
        // Look for images without source
        counts.0 = 0
        counts.1 = 0
        for image in allImages {
            if image.relativePath == nil || image.relativePath == "" {
                counts.0 += 1
            }
            else if image.image == nil {
                counts.1 += 1
            }
        }
        if counts.0 != 0 {
            NSLog("Found \(counts.0) images without local path!")
        }
        if counts.1 != 0 {
            NSLog("Found \(counts.1) invalid images!")
        }
        
        NSLog("= Done =")
    }
}
