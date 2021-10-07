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

final class LibraryMigrator: NSObject {
    
    private var storeURL: URL
    
    init(storeURL: URL) {
        self.storeURL = storeURL.appendingPathComponent(OEDatabaseFileName)
        super.init()
    }
    
    func runMigration() throws {
        let modelURL = Bundle.main.url(forResource: "OEDatabase", withExtension: "momd")!
        let destinationModel = NSManagedObjectModel(contentsOf: modelURL)
        
        let modelNames = [
            "OEDatabase",
            "OEDatabase 0.2",
            "OEDatabase 0.3",
            "OEDatabase 0.4",
            "OEDatabase 0.5",
            "OEDatabase 1.0",
            "OEDatabase 1.1",
            "OEDatabase 1.2",
            "OEDatabase 1.3",
        ]
        
        do {
            try ALIterativeMigrator.iterativeMigrateURL(storeURL, ofType: NSSQLiteStoreType, to: destinationModel, orderedModelNames: modelNames)
        } catch {
            DLog("Error migrating to latest model: \(error)\n \((error as NSError).userInfo)")
            throw error
        }
        
        let sourceMetadata = try? NSPersistentStoreCoordinator.metadataForPersistentStore(ofType: NSSQLiteStoreType, at: storeURL)
        
        if var versions = sourceMetadata?[NSStoreModelVersionIdentifiersKey] as? [String] {
            versions.sort { $0.caseInsensitiveCompare($1) == .orderedAscending }
            
            let sourceVersion = versions.last
            if sourceVersion?.compare("1.3") == .orderedAscending {
                UserDefaults.standard.set(true, forKey: OEDBScreenshot.importRequiredKey)
            }
        }
        
        DLog("Migration Done")
    }
}
