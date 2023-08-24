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
import OpenEmuKit

extension Notification.Name {
    static let OEDBSystemAvailabilityDidChange = Notification.Name("OEDBSystemAvailabilityDidChangeNotification")
}

@objc
final class OEDBSystem: OEDBItem {
    
    private static let ErrorDomain = "OEDBSystemErrorDomain"
    private enum ErrorCode: Int {
        case enabledToggleFailed
    }
    
    // MARK: - CoreDataProperties
    
    @NSManaged private var enabled: NSNumber?
    @NSManaged var lastLocalizedName: String
    // @NSManaged var shortname: String
    @NSManaged var systemIdentifier: String
    @NSManaged var games: Set<OEDBGame>
    
    // MARK: -
    
    var isEnabled: Bool {
        get {
            return enabled as? Bool ?? false
        }
        set {
            enabled = newValue as NSNumber
            
            NotificationCenter.default.post(name: .OEDBSystemAvailabilityDidChange, object: self)
        }
    }
    
    var plugin: OESystemPlugin? {
        return OESystemPlugin.systemPlugin(forIdentifier: systemIdentifier)
    }
    
    var icon: NSImage? {
        return plugin?.systemIcon
    }
    
    var name: String {
        return plugin?.systemName ?? lastLocalizedName
    }
    
    // MARK: -
    
    override class var entityName: String { "System" }
    
    // MARK: -
    
    class func systemCount(in context: NSManagedObjectContext) -> Int {
        let request = Self.fetchRequest()
        
        var result = 0
        do {
            result = try context.count(for: request)
        } catch {
            DLog("Error: \(error)")
        }
        if result == NSNotFound {
            result = 0
        }
        return result
    }
    
    class func allSystems(in context: NSManagedObjectContext) -> [OEDBSystem] {
        let sortDescriptors = [NSSortDescriptor(key: "lastLocalizedName", ascending: true)]
        return context.allObjects(ofType: Self.self, sortedBy: sortDescriptors)
    }
    
    class func allSystemIdentifiers(in context: NSManagedObjectContext) -> [String] {
        return allSystems(in: context).map(\.systemIdentifier)
    }
    
    class func enabledSystems(in context: NSManagedObjectContext) -> [OEDBSystem] {
        let predicate = NSPredicate(format: "enabled = YES")
        let sortDescriptors = [NSSortDescriptor(key: "lastLocalizedName", ascending: true)]
        return context.allObjects(ofType: Self.self, matching: predicate, sortedBy: sortDescriptors)
    }
    
    class func systemsForFile(with fileURL: URL, in context: NSManagedObjectContext) -> [OEDBSystem] {
        if let file = try? OEFile(url: fileURL) {
            return systems(for: file, in: context)
        } else {
            return []
        }
    }
    
    class func systems(for file: OEFile, in context: NSManagedObjectContext) -> [OEDBSystem] {
        
        var theOneAndOnlySystemThatGetsAChanceToHandleTheFile: OESystemPlugin?
        var otherSystemsThatMightBeAbleToHandleTheFile: [OESystemPlugin] = []
        let fileExtension = file.fileExtension
        
        for systemPlugin in OESystemPlugin.allPlugins {
            guard let controller = systemPlugin.controller,
                  controller.canHandleFileExtension(fileExtension)
            else { continue }
            
            let fileSupport = controller.canHandle(file)
            if fileSupport == .yes {
                theOneAndOnlySystemThatGetsAChanceToHandleTheFile = systemPlugin
                break
            } else if fileSupport == .uncertain {
                otherSystemsThatMightBeAbleToHandleTheFile.append(systemPlugin)
            }
        }
        
        if let sys = theOneAndOnlySystemThatGetsAChanceToHandleTheFile {
            if let system = system(for: sys.systemIdentifier, in: context),
               system.isEnabled {
                return [system]
            } else {
                return []
            }
        } else {
            var validSystems: [OEDBSystem] = []
            for sys in otherSystemsThatMightBeAbleToHandleTheFile {
                if let system = system(for: sys.systemIdentifier, in: context),
                   system.isEnabled {
                    validSystems.append(system)
                }
            }
            return validSystems
        }
    }
    
    @nonobjc
    class func system(for plugin: OESystemPlugin, in context: NSManagedObjectContext) -> OEDBSystem {
        let systemIdentifier = plugin.systemIdentifier
        
        if let system = system(for: systemIdentifier, in: context) {
            return system
        }
        
        var system: OEDBSystem!
        context.performAndWait {
            system = OEDBSystem.createObject(in: context)
            system.systemIdentifier = systemIdentifier
            system.lastLocalizedName = system.name
            
            system.save()
        }
        
        return system
    }
    
    class func system(for identifier: String, in context: NSManagedObjectContext) -> OEDBSystem? {
        let predicate = NSPredicate(format: "systemIdentifier == %@", identifier)
        
        let request = Self.fetchRequest()
        request.predicate = predicate
        request.fetchLimit = 1
        
        var result: [OEDBSystem]?
        do {
            result = try context.fetch(request) as? [OEDBSystem]
        } catch {
            DLog("Error: \(error)")
        }
        
        return result?.first
    }
    
    class func header(for file: OEFile, forSystem identifier: String) -> String? {
        let systemPlugin = OESystemPlugin.systemPlugin(forIdentifier: identifier)
        let header = systemPlugin?.controller.headerLookup(for: file)
        return header
    }
    
    class func serial(for file: OEFile, forSystem identifier: String) -> String? {
        let systemPlugin = OESystemPlugin.systemPlugin(forIdentifier: identifier)
        let serial = systemPlugin?.controller.serialLookup(for: file)
        return serial
    }
    
    // MARK: -
    
    /// Toggles the system's `enabled` status. If toggling the status would cause an unwanted situation (e.g., there would be no systems enabled), returns `false` with an appropriate error message to present to the user.
    func toggleEnabled() throws {
        let context = libraryDatabase.mainThreadContext
        let enabled = isEnabled
        
        // Make sure at least one system would still be enabled.
        if enabled && Self.enabledSystems(in: context).count == 1 {
            let localizedDescription = NSLocalizedString("At least one System must be enabled", comment: "")
            let error = NSError(domain: OEDBSystem.ErrorDomain,
                                code: OEDBSystem.ErrorCode.enabledToggleFailed.rawValue,
                                userInfo: [NSLocalizedDescriptionKey : localizedDescription])
            throw error
        }
        
        // Make sure only systems with a valid plugin get enabled.
        if !enabled && plugin == nil {
            let localizedDescription = String(format: NSLocalizedString("%@ could not be enabled because its plugin was not found.", comment: ""), name)
            let error = NSError(domain: OEDBSystem.ErrorDomain,
                                code: OEDBSystem.ErrorCode.enabledToggleFailed.rawValue,
                                userInfo: [NSLocalizedDescriptionKey : localizedDescription])
            throw error
        }
        
        // Toggle enabled status and save.
        isEnabled = !enabled
        save()
    }
    
    /// Convenience method for attempting to toggle the system's `enabled` status and automatically presenting a modal alert if the toggle fails. Returns `true` if the toggle succeeded.
    @discardableResult
    func toggleEnabledAndPresentError() -> Bool {
        do {
            try toggleEnabled()
        } catch {
            let alert = OEAlert()
            alert.messageText = error.localizedDescription
            alert.defaultButtonTitle = NSLocalizedString("OK", comment: "")
            alert.runModal()
            
            return false
        }
        
        return true
    }
    
    var coverAspectRatio: CGFloat {
        if let aspectRatio = plugin?.coverAspectRatio,
           aspectRatio != 0 {
            return aspectRatio
        }
        
        // in case system plugin has been removed, return a default value
        return 1.365385
    }
}

extension OEDBSystem: SidebarItem {
    var sidebarIcon: NSImage? { icon }
    var sidebarName: String { name }
    var sidebarID: String? { systemIdentifier }
    var isEditableInSidebar: Bool { false }
    var hasSubCollections: Bool { false }
}

extension OEDBSystem: GameCollectionViewItemProtocol {
    var collectionViewName: String { name }
    var isCollectionEditable: Bool { true }
    var shouldShowSystemColumnInListView: Bool { false }
    
    var fetchPredicate: NSPredicate {
        NSPredicate(format: "system == %@", self)
    }
    var fetchLimit: Int { 0 }
    var fetchSortDescriptors: [NSSortDescriptor] { [] }
}

// MARK: - Debug

#if DEBUG
@available(macOS 11.0, *)
extension OEDBSystem {
    
    func dump(prefix: String = "---") {
        Logger.library.debug("\(prefix) Beginning of system dump")
        
        Logger.library.debug("\(prefix) System last localized name is \(self.lastLocalizedName)")
        Logger.library.debug("\(prefix) system identifier is \(self.systemIdentifier)")
        Logger.library.debug("\(prefix) enabled? \(self.isEnabled)")
        
        Logger.library.debug("\(prefix) Number of games in this system is \(self.games.count)")
        
        games.forEach { $0.dump(prefix: prefix + "-----") }
        
        Logger.library.debug("\(prefix) End of system dump\n\n")
    }
}
#endif
