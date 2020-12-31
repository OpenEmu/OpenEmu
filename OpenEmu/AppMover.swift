//
//  AppMover.swift
//  AppMover
//
//  Created by Oskar Groth on 2019-12-20.
//  Updated by Stuart Carnie on 2020-12-31.
//  Copyright © 2019 Oskar Groth. All rights reserved.
//
//  https://github.com/OskarGroth/AppMover

import AppKit
import OpenEmuKit

public enum AppMover {
    
    /// Returns a boolean indicating whether the application has any quarantined files.
    /// - Returns: `true` if the app bundle is quarantined.
    public static func isAppQuarantined() -> Bool {
        let fm = FileManager.default
        
        let url = Bundle.main.bundleURL
        guard let e = fm.enumerator(at: url, includingPropertiesForKeys: []) else { return false }
        let res = e.lazy
            .compactMap {
                $0 as? URL
            }
            .first {
                (try? fm.hasExtendedAttribute("com.apple.quarantine", atPath: $0.path, traverseLink: true)) ?? false
            }
        
        return res != .none
    }
    
    /// Recursively removes the quarantine extended attribute from any files in the application bundle.
    public static func removeQuarantineAttribute() {
        let fm = FileManager.default
        
        let url = Bundle.main.bundleURL
        try? fm.removeExtendedAttribute("com.apple.quarantine", atPath: url.path, traverseLink: true)
        
        guard let paths = fm.enumerator(at: url, includingPropertiesForKeys: []) else { return }
        
        // Produce a lazy enumerator for files that are quarantined
        paths.lazy
            .compactMap { $0 as? URL }
            .filter {
                (try? fm.hasExtendedAttribute("com.apple.quarantine", atPath: $0.path, traverseLink: true)) ?? false
            }
            .forEach {
                try? fm.removeExtendedAttribute("com.apple.quarantine", atPath: $0.path, traverseLink: true)
            }
    }
    
    /// Verify if the app is running in the Applications folder and if not, alert the user
    /// they must move it.
    public static func moveIfNecessary() {
        let bundle = Bundle.main
        
        if shouldIgnore || bundle.isInstalled {
            return
        }
        
        guard let applications = preferredInstallDirectory() else { return }
        
        let srcUrl = bundle.bundleURL
        let dstUrl = applications.appendingPathComponent(srcUrl.lastPathComponent)
        
        let fm = FileManager.default
        let needDestAuth = fm.fileExists(atPath: dstUrl.path) && !fm.isWritableFile(atPath: dstUrl.path)
        let needAuth = needDestAuth || !fm.isWritableFile(atPath: applications.path)
        
        // Activate app -- work-around for focus issues related to "scary file from
        // internet" OS dialog.
        if !NSApp.isActive {
            NSApp.activate(ignoringOtherApps: true)
        }
        
        let alert: OEAlert = .moveToApplications(needAuth: needAuth)
        guard alert.runModal() == .alertFirstButtonReturn else {
            return
        }
        if needAuth {
            os_log(.info, log: OE_LOG_DEFAULT, "The destination is not writable; prompting for permissions")
            
            switch authorizedInstall(from: srcUrl, to: dstUrl) {
            case .canceled:
                return moveIfNecessary()
            case .failed:
                return NSApplication.shared.terminate(self)
            case .success:
                break
            }
        } else {
            do {
                if fm.fileExists(atPath: dstUrl.path) {
                    if isApplicationAtUrlRunning(dstUrl) {
                        NSWorkspace.shared.open(dstUrl)
                        return
                    } else {
                        try fm.trashItem(at: dstUrl, resultingItemURL: nil)
                    }
                }
                try fm.copyItem(at: srcUrl, to: dstUrl)
            } catch {
                os_log(.error, log: OE_LOG_DEFAULT, "Failed to move application bundle { error = %{public}@ }", error.localizedDescription)
                return
            }
        }
        
        // Remove the original application bundle
        try? fm.removeItem(at: srcUrl)
        
        relaunch(at: dstUrl.path)
        exit(0)
    }
    
    private enum InstallResult {
        case success, failed, canceled
    }
    
    static private func authorizedInstall(from sourceURL: URL, to destinationURL: URL) -> InstallResult {
        guard destinationURL.representsBundle,
              destinationURL.isValid,
              sourceURL.isValid
        else { return .failed }
        
        guard
            let srcPath = sourceURL.withUnsafeFileSystemRepresentation({ $0 == nil ? nil : String(cString: $0!) }),
            let dstPath = destinationURL.withUnsafeFileSystemRepresentation({ $0 == nil ? nil : String(cString: $0!) })
        else { return .failed }
        
        let deleteCommand = "rm -rf '\(srcPath)'"
        let copyCommand = "cp -pR '\(srcPath)' '\(dstPath)'"
        guard
            let script = NSAppleScript(source: "do shell script \"\(deleteCommand) && \(copyCommand)\" with administrator privileges")
        else { return .failed }
        
        var error: NSDictionary?
        script.executeAndReturnError(&error)
        if let error = error {
            if error[NSAppleScript.errorNumber] as? Int16 == -128 {
                return .canceled
            }
            
            if let errNum = error[NSAppleScript.errorNumber] as? Int16,
               let errStr = error[NSAppleScript.errorMessage] as? String {
                os_log(.error, log: OE_LOG_DEFAULT, "Executing the authorizedInstall AppleScript failed with an error. { errorNumber = %d, errorMessage = %{public}@",
                       errNum, errStr)
            }
            
            return .failed
        }
        
        return .success
    }
    
    static private func preferredInstallDirectory() -> URL? {
        let fm = FileManager.default
        let dirs = fm.urls(for: .applicationDirectory, in: .allDomainsMask)
        // Find Applications dir with the most apps that isn't system protected
        return dirs
            .map { $0.resolvingSymlinksInPath() }
            .filter { url in
                guard
                    let attr = try? fm.attributesOfItem(atPath: url.path),
                    let type = attr[.type] as? FileAttributeType
                else { return false }
                
                return type == .typeDirectory && url.path != "/System/Applications"
            }
            .sorted { left, right in
                return left.numberOfFilesInDirectory < right.numberOfFilesInDirectory
            }
            .last
    }
    
    static private func isApplicationAtUrlRunning(_ url: URL) -> Bool {
        let url = url.standardized
        return NSWorkspace
            .shared
            .runningApplications
            .contains {
                $0.bundleURL?.standardized == url
            }
    }
    
    static private func relaunch(at path: String) {
        let pid = ProcessInfo.processInfo.processIdentifier
        
        /// This script performs the following actions:
        ///  1. Recursively remove the quarantine extended attribute from the application bundle;
        ///  2. waits for application to terminate; and
        ///  3. relaunches at the specified path.
        
        let script = "(/usr/bin/xattr -d -r com.apple.quarantine \"\(path)\" ; while /bin/kill -0 \(pid) >&/dev/null; do /bin/sleep 0.1; done; /usr/bin/open \"\(path)\") &"
        
        let task = Process()
        task.executableURL = URL(fileURLWithPath: "/bin/sh")
        task.arguments = ["-c", script]
        try? task.run()
    }
    
    /// shouldIgnore returns true for any conditions that should suppress the
    /// Move to Applications Folder alert, including if a debugger is attached
    /// or the bundle is already in a path that contains Applications or DerivedData.
    static private var shouldIgnore: Bool {
        if OEXPCCDebugSupport.debuggerAttached {
            return true
        }
        
        let safe = ["/Applications/", "/DerivedData/"]
        return safe.contains { Bundle.main.bundlePath.contains($0) }
    }
}

fileprivate extension URL {
    
    var representsBundle: Bool {
        pathExtension == "app"
    }
    
    var isValid: Bool {
        !path.trimmingCharacters(in: .whitespaces).isEmpty
    }
    
    var numberOfFilesInDirectory: Int {
        (try? FileManager.default.contentsOfDirectory(atPath: path))?.count ?? 0
    }
    
}

fileprivate extension Bundle {
    var isInstalled: Bool {
        FileManager.default
            .urls(for: .applicationDirectory, in: .allDomainsMask)
            .contains {
                bundlePath.hasPrefix($0.path)
            }
            || bundlePath.contains("/Applications/")
    }
}

