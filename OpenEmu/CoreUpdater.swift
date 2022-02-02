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

import Foundation
import OpenEmuKit
import Sparkle

final class CoreUpdater: NSObject {
    
    enum Errors: Error {
        case noDownloadableCoreForIdentifierError
        case newCoreCheckAlreadyPendingError
    }
    
    static let shared = CoreUpdater()
    
    @objc dynamic private(set) var coreList: [CoreDownload] = []
    
    var completionHandler: ((_ plugin: OECorePlugin?, Error?) -> Void)?
    var coreIdentifier: String?
    var alert: OEAlert?
    var coreDownload: CoreDownload?
    
    private var coresDict: [String : CoreDownload] = [:]
    private var autoInstall = false
    private var lastCoreListURLTask: URLSessionDataTask?
    private var pendingUserInitiatedDownloads: Set<CoreDownload> = []
    
    override init() {
        super.init()
        
        for plugin in OECorePlugin.allPlugins {
            let download = CoreDownload(plugin: plugin)
            let bundleID = plugin.bundleIdentifier.lowercased()
            coresDict[bundleID] = download
        }
        
        updateCoreList()
    }
    
    private func updateCoreList() {
        willChangeValue(forKey: #keyPath(coreList))
        coreList = coresDict.values.sorted {
            $0.name.localizedStandardCompare($1.name) == .orderedAscending
        }
        didChangeValue(forKey: #keyPath(coreList))
    }
    
    @objc func checkForUpdates() {
        guard Thread.isMainThread else {
            performSelector(onMainThread: #selector(checkForUpdates), with: nil, waitUntilDone: false)
            return
        }
        
        for plugin in OECorePlugin.allPlugins {
            
            if let appcastURLString = plugin.infoDictionary["SUFeedURL"] as? String,
               let updater = SUUpdater(for: plugin.bundle) {
                updater.delegate = self
                updater.feedURL = URL(string: appcastURLString)
                
                // Core updates are silently installed on launch, so ensure there is no annoying update prompt from Sparkle
                updater.automaticallyChecksForUpdates = true
                updater.automaticallyDownloadsUpdates = true
                
                updater.resetUpdateCycle()
                updater.checkForUpdateInformation()
            }
        }
    }
    
    func checkForUpdatesAndInstall() {
        autoInstall = true
        checkForUpdates()
    }
    
    func checkForNewCores(completionHandler handler: ((_ error: Error?) -> Void)? = nil) {
        guard lastCoreListURLTask == nil else {
            handler?(Errors.newCoreCheckAlreadyPendingError)
            return
        }
        
        let coreListURL = URL(string: Bundle.main.infoDictionary!["OECoreListURL"] as! String)!
        
        lastCoreListURLTask = URLSession.shared.dataTask(with: coreListURL) {data, response , error in
            DispatchQueue.main.async {
                guard let data = data else {
                    handler?(error)
                    self.lastCoreListURLTask = nil
                    return
                }
                
                if let coreList = try? XMLDocument(data: data, options: []),
                   let coreNodes = try? coreList.nodes(forXPath: "/cores/core") as? [XMLElement] {
                    for coreNode in coreNodes {
                        guard
                            let coreID = coreNode.attribute(forName: "id")?.stringValue,
                            self.coresDict[coreID] == nil,
                            let coreName = coreNode.attribute(forName: "name")?.stringValue,
                            let systemNodes = try? coreNode.nodes(forXPath: "./systems/system") as? [XMLElement],
                            let appcastURLString = coreNode.attribute(forName: "appcastURL")?.stringValue,
                            let appcastURL = URL(string: appcastURLString)
                        else { continue }
                        
                        let download = CoreDownload()
                        download.name = coreName
                        download.bundleIdentifier = coreID
                        
                        var systemNames: [String] = []
                        var systemIdentifiers: [String] = []
                        
                        for systemNode in systemNodes {
                            if let systemName = systemNode.stringValue {
                                systemNames.append(systemName)
                            }
                            if let systemIdentifier = systemNode.attribute(forName: "id")?.stringValue {
                                systemIdentifiers.append(systemIdentifier)
                            }
                        }
                        
                        download.systemNames = systemNames
                        download.systemIdentifiers = systemIdentifiers
                        download.canBeInstalled = true
                        
                        let appcast = CoreAppcast(url: appcastURL)
                        
                        DispatchQueue.main.async {
                            do {
                                try appcast.fetch {
                                    download.appcastItem = appcast.items.first { $0.isSupported }
                                    download.delegate = self
                                    
                                    if download == self.coreDownload {
                                        download.start()
                                    }
                                    
                                    self.updateCoreList()
                                }
                            } catch {
                                NSLog("\(error)")
                            }
                        }
                        
                        self.coresDict[coreID] = download
                    }
                }
                
                self.updateCoreList()
                
                handler?(nil)
                self.lastCoreListURLTask = nil
            }
        }
        
        lastCoreListURLTask?.resume()
    }
    
    func cancelCheckForNewCores() {
        lastCoreListURLTask?.cancel()
        lastCoreListURLTask = nil
    }
    
    // MARK: - Installing with OEAlert
    
    func installCore(for game: OEDBGame, withCompletionHandler handler: @escaping (_ plugin: OECorePlugin?, _ error: Error?) -> Void) {
        
        let systemIdentifier = game.system?.systemIdentifier ?? ""
        var validPlugins = coreList.filter { $0.systemIdentifiers.contains(systemIdentifier) }
        
        if !validPlugins.isEmpty {
            let download: CoreDownload
            
            if validPlugins.count == 1 {
                download = validPlugins.first!
            } else {
                // Sort by core name alphabetically to match our automatic core picker behavior
                validPlugins.sort {
                    $0.name.localizedStandardCompare($1.name) == .orderedAscending
                }
                
                // Check if a core is set as default in AppDelegate
                var didFindDefaultCore = false
                var foundDefaultCoreIndex = 0
                
                for (index, plugin) in validPlugins.enumerated() {
                    let sysID = "defaultCore.\(systemIdentifier)"
                    if let userDef = UserDefaults.standard.string(forKey: sysID),
                       userDef.caseInsensitiveCompare(plugin.bundleIdentifier) == .orderedSame {
                        didFindDefaultCore = true
                        foundDefaultCoreIndex = index
                        break
                    }
                }
                
                // Use default core plugin for this system, otherwise just use first found from the sorted list
                if didFindDefaultCore {
                    download = validPlugins[foundDefaultCoreIndex]
                } else {
                    download = validPlugins.first!
                }
            }
            
            let coreName = download.name
            let message = String(format: NSLocalizedString("OpenEmu uses 'Cores' to emulate games. You need the %@ Core to play %@", comment: ""), coreName, game.displayName ?? "")
            installCore(with: download, message: message, completionHandler: handler)
        }
        else {
            handler(nil, Errors.noDownloadableCoreForIdentifierError)
        }
    }
    
    func installCore(for state: OEDBSaveState, withCompletionHandler handler: @escaping (_ plugin: OECorePlugin?, _ error: Error?) -> Void) {
        
        let coreID = state.coreIdentifier?.lowercased() ?? ""
        if let download = coresDict[coreID] {
            let coreName = download.name
            let message = String(format: NSLocalizedString("To launch the save state %@ you will need to install the '%@' Core", comment: ""), state.displayName ?? "", coreName)
            installCore(with: download, message: message, completionHandler: handler)
        } else {
            // TODO: create proper error saying that no core is available for the state
            handler(nil, Errors.noDownloadableCoreForIdentifierError)
        }
    }
    
    func installCore(with download: CoreDownload, message: String, completionHandler handler: @escaping (_ plugin: OECorePlugin?, _ error: Error?) -> Void) {
        
        let alert = OEAlert()
        alert.messageText = NSLocalizedString("Missing Core", comment: "")
        alert.informativeText = message
        alert.defaultButtonTitle = NSLocalizedString("Install", comment: "")
        alert.alternateButtonTitle = NSLocalizedString("Cancel", comment: "")
        alert.setDefaultButtonAction(#selector(startInstall), andTarget: self)
        
        coreIdentifier = coresDict.first(where: { $1 == download })?.key
        completionHandler = handler
        
        self.alert = alert
        
        if alert.runModal() == .alertSecondButtonReturn {
            handler(nil, NSError(domain: NSCocoaErrorDomain, code: NSUserCancelledError))
        }
        
        completionHandler = nil
        coreDownload = nil
        coreIdentifier = nil
        
        self.alert = nil
    }
    
    func installCore(with download: CoreDownload, completionHandler handler: @escaping (_ plugin: OECorePlugin?, _ error: Error?) -> Void) {
        
        let alert = OEAlert()
        
        coreIdentifier = coresDict.first(where: { $1 == download })?.key
        completionHandler = handler
        self.alert = alert
        
        alert.performBlockInModalSession {
            self.startInstall()
        }
        alert.runModal()
        
        completionHandler = nil
        coreDownload = nil
        coreIdentifier = nil
        
        self.alert = nil
    }
    
    // MARK: -
    
    @objc func cancelInstall() {
        coreDownload?.cancel()
        completionHandler = nil
        coreDownload = nil
        alert?.close(withResult: .alertSecondButtonReturn)
        alert = nil
        coreIdentifier = nil
    }
    
    @objc func startInstall() {
        alert?.messageText = NSLocalizedString("Downloading and Installing Core…", comment: "")
        alert?.informativeText = ""
        alert?.defaultButtonTitle = ""
        alert?.setAlternateButtonAction(#selector(cancelInstall), andTarget: self)
        alert?.showsProgressbar = true
        alert?.progress = 0
        
        guard
            let coreID = coreIdentifier,
            let pluginDL = coresDict[coreID]
        else {
            alert?.messageText = NSLocalizedString("Error!", comment: "")
            alert?.informativeText = NSLocalizedString("The core could not be downloaded. Try installing it from the Cores preferences.", comment: "")
            alert?.defaultButtonTitle = NSLocalizedString("OK", comment: "")
            alert?.alternateButtonTitle = ""
            alert?.setDefaultButtonAction(#selector(OEAlert.buttonAction(_:)), andTarget: alert)
            alert?.showsProgressbar = false
            
            return
        }
        
        coreDownload = pluginDL
        coreDownload?.start()
    }
    
    func failInstallWithError(_ error: Error?) {
        alert?.close(withResult: .alertFirstButtonReturn)
        
        completionHandler?(OECorePlugin(bundleIdentifier: coreIdentifier), error)
        
        alert = nil
        coreIdentifier = nil
        completionHandler = nil
    }
    
    func finishInstall() {
        alert?.close(withResult: .alertFirstButtonReturn)
        
        completionHandler?(OECorePlugin(bundleIdentifier: coreIdentifier), nil)
        
        alert = nil
        coreIdentifier = nil
        completionHandler = nil
    }
    
    // MARK: - Other user-initiated (= with error reporting) downloads
    
    func installCoreInBackgroundUserInitiated(_ download: CoreDownload) {
        assert(download.delegate === self, "download \(download)'s delegate is not the singleton CoreUpdater!?")
        
        pendingUserInitiatedDownloads.insert(download)
        
        download.start()
    }
}

// MARK: - CoreDownload Delegate

private var CoreDownloadProgressContext = 0

extension CoreUpdater: CoreDownloadDelegate {
    
    func coreDownloadDidStart(_ download: CoreDownload) {
        updateCoreList()
        
        download.addObserver(self, forKeyPath: #keyPath(CoreDownload.progress), options: [.new, .old, .initial, .prior], context: &CoreDownloadProgressContext)
    }
    
    func coreDownloadDidFinish(_ download: CoreDownload) {
        updateCoreList()
        
        download.removeObserver(self, forKeyPath: #keyPath(CoreDownload.progress), context: &CoreDownloadProgressContext)
        
        if download == coreDownload {
            finishInstall()
        }
        
        pendingUserInitiatedDownloads.remove(download)
    }
    
    func coreDownloadDidFail(_ download: CoreDownload, withError error: Error?) {
        updateCoreList()
        
        if download == coreDownload {
            failInstallWithError(error)
        }
        
        if pendingUserInitiatedDownloads.contains(download),
           let error = error {
            NSApp.presentError(error)
        }
        
        pendingUserInitiatedDownloads.remove(download)
    }
    
    override func observeValue(forKeyPath keyPath: String?, of object: Any?, change: [NSKeyValueChangeKey : Any]?, context: UnsafeMutableRawPointer?) {
        
        guard context == &CoreDownloadProgressContext else {
            return super.observeValue(forKeyPath: keyPath, of: object, change: change, context: context)
        }
        
        if let object = object as? CoreDownload,
           object == coreDownload {
            alert?.progress = coreDownload!.progress
        }
    }
}

// MARK: - SUUpdater Delegate

extension CoreUpdater: SUUpdaterDelegate {
    
    func updater(_ updater: SUUpdater, didFindValidUpdate item: SUAppcastItem) {
        
        for plugin in OECorePlugin.allPlugins {
            guard updater == SUUpdater(for: plugin.bundle) else {
                continue
            }
            
            let coreID = plugin.bundleIdentifier.lowercased()
            let download = coresDict[coreID]
            download?.hasUpdate = true
            download?.appcastItem = CoreAppcastItem(url: item.fileURL, version: item.versionString, minOSVersion: item.minimumSystemVersion)
            download?.delegate = self
            
            if autoInstall {
                download?.start()
            }
            
            break
        }
        
        updateCoreList()
    }
}

private final class CoreAppcast {
    
    let url: URL
    var items: [CoreAppcastItem] = []
    
    init(url: URL) {
        self.url = url
    }
    
    func fetch(completionHandler handler: (() -> Void)? = nil) throws {
        
        let items: [XMLElement]
        do {
            let appcast = try XMLDocument(contentsOf: url, options: [])
            items = try appcast.nodes(forXPath: "/rss/channel/item") as! [XMLElement]
        } catch {
            throw error
        }
        
        for item in items {
            if let enclosure = item.elements(forName: "enclosure").first,
               let fileURL = enclosure.attribute(forName: "url")?.stringValue,
               let url = URL(string: fileURL),
               let version = enclosure.attribute(forName: "sparkle:version")?.stringValue,
               let minOSVersion = item.elements(forName: "sparkle:minimumSystemVersion").first?.stringValue
            {
                self.items.append(CoreAppcastItem(url: url, version: version, minOSVersion: minOSVersion))
            }
        }
        
        handler?()
    }
}

struct CoreAppcastItem {
    
    var version: String
    var fileURL: URL
    var minimumSystemVersion: String
    
    init(url: URL, version: String, minOSVersion: String) {
        fileURL = url
        self.version = version
        minimumSystemVersion = minOSVersion
    }
    
    var isSupported: Bool {
        return SUStandardVersionComparator.default().compareVersion(minimumSystemVersion, toVersion: Self.osVersionString) != .orderedDescending
    }
    
    private static let osVersionString: String = {
        let osVersion = ProcessInfo.processInfo.operatingSystemVersion
        return "\(osVersion.majorVersion).\(osVersion.minorVersion).\(osVersion.patchVersion)"
    }()
}
