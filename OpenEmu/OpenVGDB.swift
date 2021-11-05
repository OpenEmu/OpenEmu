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

extension Notification.Name {
    static let GameInfoHelperWillUpdate = NSNotification.Name("OEGameInfoHelperWillUpdateNotificationName")
    static let GameInfoHelperDidChangeUpdateProgress = NSNotification.Name("OEGameInfoHelperDidChangeUpdateProgressNotificationName")
    static let GameInfoHelperDidUpdate = NSNotification.Name("OEGameInfoHelperDidUpdateNotificationName")
}

final class OpenVGDB: NSObject {
    
    static let versionKey = "OpenVGDBVersion"
    static let updateCheckKey = "OpenVGDBUpdatesChecked"
    static let updateIntervalKey = "OpenVGDBUpdateInterval"
    
    private static let fileName = "openvgdb"
    private static let downloadURL = URL(string: "https://github.com/OpenVGDB/OpenVGDB/releases/download/")!
    private static let updateURL = URL(string: "https://api.github.com/repos/OpenVGDB/OpenVGDB/releases?page=1&per_page=1")!
    
    private static let initializeDefaults: Void = {
        let onceADayInterval = 60 * 60 * 24 * 1
        UserDefaults.standard.register(defaults: [
            OpenVGDB.versionKey : "",
            OpenVGDB.updateCheckKey : Date(timeIntervalSince1970: 0),
            OpenVGDB.updateIntervalKey : onceADayInterval,
        ])
    }()
    
    private(set) var downloadProgress: Double = 0
    private(set) var downloadVersion: String?
    private(set) var isUpdating = false
    
    private var downloadSession: URLSession?
    
    private var database: SQLiteDatabase?
    
    var isAvailable: Bool {
        return database != nil
    }
    
    static let shared: OpenVGDB = {
        
        let sharedHelper = OpenVGDB()
        
        let databaseURL = sharedHelper.databaseFileURL
        
        if (try? databaseURL.checkResourceIsReachable()) != true {
            DispatchQueue.global(qos: .utility).async {
                let defaults = UserDefaults.standard
                defaults.removeObject(forKey: OpenVGDB.updateCheckKey)
                defaults.removeObject(forKey: OpenVGDB.versionKey)
                
                sharedHelper.checkForUpdates { url, version in
                    if let url = url, let version = version {
                        sharedHelper.install(with: url, version: version)
                    }
                }
            }
        }
        else {
            do {
                sharedHelper.database = try SQLiteDatabase(url: databaseURL)
            } catch {
                NSApp.presentError(error)
            }
            
            // check for updates
            DispatchQueue.global(qos: .utility).async {
                let defaults = UserDefaults.standard
                let lastUpdateCheck = defaults.object(forKey: OpenVGDB.updateCheckKey) as! Date
                let updateInterval = defaults.double(forKey: OpenVGDB.updateIntervalKey)
                
                if Date().timeIntervalSince(lastUpdateCheck) > updateInterval {
                    NSLog("Check for updates (\(Date().timeIntervalSince(lastUpdateCheck)) > \(updateInterval))")
                    
                    sharedHelper.checkForUpdates { url, version in
                        if let url = url, let version = version {
                            sharedHelper.install(with: url, version: version)
                        }
                    }
                }
            }
        }
        
        return sharedHelper
    }()
    
    override init() {
        super.init()
        Self.initializeDefaults
    }
    
    var databaseFileURL: URL {
        let applicationSupport = (try? FileManager.default.url(for: .applicationSupportDirectory, in: .userDomainMask, appropriateFor: nil, create: true)) ?? FileManager.default.temporaryDirectory
        return applicationSupport.appendingPathComponent("OpenEmu/openvgdb.sqlite")
    }
    
    /// Checks for updates and passes URL of new release and version if any newer DB is found.
    func checkForUpdates(withHandler handler: @escaping (_ newURL: URL?, _ newVersion: String?) -> Void) {
        
        DLog("")
        
        var request = URLRequest(url: OpenVGDB.updateURL, cachePolicy: .reloadIgnoringCacheData, timeoutInterval: 30)
        request.setValue("OpenEmu", forHTTPHeaderField: "User-Agent")
        
        let task = URLSession.shared.dataTask(with: request) { result, response, error in
            
            if let result = result {
                let releases = try? JSONSerialization.jsonObject(with: result, options: .allowFragments) as? [AnyHashable]
                if let releases = releases {
                    let defaults = UserDefaults.standard
                    
                    let currentVersion = defaults.string(forKey: OpenVGDB.versionKey)!
                    var nextVersion = currentVersion
                    
                    defaults.set(Date(), forKey: OpenVGDB.updateCheckKey)
                    
                    for release in releases {
                        if let release = release as? [AnyHashable : Any],
                           let tagName = release["tag_name"] as? String,
                           tagName.compare(nextVersion) != .orderedSame {
                            nextVersion = tagName
                        }
                    }
                    
                    if nextVersion != currentVersion {
                        let url = OpenVGDB.downloadURL.appendingPathComponent("\(nextVersion)/\(OpenVGDB.fileName).zip")
                        
                        DLog("Updating OpenVGDB version from \(!currentVersion.isEmpty ? currentVersion : "(none)") to \(nextVersion).")
                        
                        handler(url, nextVersion)
                        
                        return
                    } else {
                        DLog("OpenVGDB not updated.")
                    }
                }
            }
            
            handler(nil, nil)
        }
        
        task.resume()
    }
    
    func cancelUpdate() {
        DLog("Cancelling OpenVGDB download.")
        
        downloadSession?.invalidateAndCancel()
    }
    
    func install(with url: URL, version versionTag: String) {
        
        DispatchQueue.main.async {
            self.isUpdating = true
            
            NotificationCenter.default.post(name: .GameInfoHelperWillUpdate, object: self)
            
            self.downloadProgress = 0
            self.downloadVersion = versionTag
            
            let request = URLRequest(url: url)
            
            let downloadSession = URLSession(configuration: .default, delegate: self, delegateQueue: .main)
            self.downloadSession = downloadSession
            
            let downloadTask = downloadSession.downloadTask(with: request)
            
            DLog("Starting OpenVGDB download.")
            
            downloadTask.resume()
        }
    }
    
    func executeQuery(_ sql: String) throws -> [[String : Any]] {
        guard let database = database else { return [] }
        do {
            return try database.executeQuery(sql)
        } catch {
            throw error
        }
    }
}

extension OpenVGDB: URLSessionDownloadDelegate {
    
    func urlSession(_ session: URLSession, task: URLSessionTask, didCompleteWithError error: Error?) {
        
        DLog("OpenVGDB download did complete: \(error?.localizedDescription ?? "no errors")")
        
        isUpdating = false
        downloadProgress = 0
        
        downloadSession?.finishTasksAndInvalidate()
        downloadSession = nil
        
        postDidUpdateNotification()
    }
    
    func urlSession(_ session: URLSession, downloadTask: URLSessionDownloadTask, didWriteData bytesWritten: Int64, totalBytesWritten: Int64, totalBytesExpectedToWrite: Int64) {
        
        downloadProgress = Double(totalBytesWritten) / Double(totalBytesExpectedToWrite)
        
        NotificationCenter.default.post(name: .GameInfoHelperDidChangeUpdateProgress, object: self)
    }
    
    func urlSession(_ session: URLSession, downloadTask: URLSessionDownloadTask, didFinishDownloadingTo location: URL) {
        
        DLog("OpenVGDB download did finish downloading temporary data.")
        
        let url = databaseFileURL
        let databaseFolder = url.deletingLastPathComponent()
        
        OEDecompressFileInArchiveAtPathToDirectory(location.path, databaseFolder.path)
        
        DLog("OpenVGDB extracted to database folder.")
        
        database = try? SQLiteDatabase(url: url)
        if database != nil {
            UserDefaults.standard.set(downloadVersion, forKey: OpenVGDB.versionKey)
        }
        
        isUpdating = false
        downloadProgress = 1
        downloadVersion = nil
        
        postDidUpdateNotification()
    }
    
    private func postDidUpdateNotification() {
        DispatchQueue.main.async {
            NotificationCenter.default.post(name: .GameInfoHelperDidUpdate, object: self)
        }
    }
}
