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

class Download: NSObject {
    
    var progressHandler: ((Double) -> Bool)?
    var completionHandler: ((URL?, Error?) -> Void)?
    
    private(set) var destination: URL?
    private(set) var error: Error?
    private(set) dynamic var progress: Double = 0 {
        didSet {
            if let progressHandler = progressHandler,
               !progressHandler(progress) {
                downloadSession?.invalidateAndCancel()
                downloadSession = nil
            }
        }
    }
    
    private var url: URL
    private var downloadSession: URLSession?
    private var waitSemaphore = DispatchSemaphore(value: 0)
    
    init(url: URL) {
        self.url = url
        super.init()
    }
    
    func start() {
        assert(downloadSession == nil, "There shouldn't be a previous download session.")
        
        progress = 0
        
        let request = URLRequest(url: url, cachePolicy: .returnCacheDataElseLoad, timeoutInterval: 60)
        
        let downloadSession = URLSession(configuration: .default, delegate: self, delegateQueue: .main)
        downloadSession.sessionDescription = url.absoluteString
        self.downloadSession = downloadSession
        
        let downloadTask = downloadSession.downloadTask(with: request)
        
        DLog("Starting download (\(downloadSession.sessionDescription ?? ""))")
        
        downloadTask.resume()
        
        error = nil
        destination = nil
    }
    
    func cancel() {
        DLog("Cancelling download (\(downloadSession?.sessionDescription ?? ""))")
        downloadSession?.invalidateAndCancel()
    }
    
    func waitUntilCompleted() {
        _ = waitSemaphore.wait(timeout: .distantFuture)
    }
    
    private func callCompletionHandler() {
        
        waitSemaphore.signal()
        
        completionHandler?(destination, error)
        
        // Make sure we only call the completion handler once
        completionHandler = nil
    }
}

extension Download: URLSessionDownloadDelegate {
    
    func urlSession(_ session: URLSession, task: URLSessionTask, didCompleteWithError error: Error?) {
        
        DLog("Download (\(session.sessionDescription ?? "")) did complete: \(error?.localizedDescription ?? "no errors")")
        
        self.error = error
        
        callCompletionHandler()
        
        destination = nil
        
        downloadSession?.finishTasksAndInvalidate()
        downloadSession = nil
    }
    
    func urlSession(_ session: URLSession, downloadTask: URLSessionDownloadTask, didWriteData bytesWritten: Int64, totalBytesWritten: Int64, totalBytesExpectedToWrite: Int64) {
        
        let progress = Double(totalBytesWritten) / Double(totalBytesExpectedToWrite)
        
        // Try reducing the number of callbacks.
        if abs(progress - self.progress) > 0.01 {
            self.progress = progress
        }
    }
    
    func urlSession(_ session: URLSession, downloadTask: URLSessionDownloadTask, didFinishDownloadingTo location: URL) {
        
        DLog("Download (\(session.sessionDescription ?? "")) did finish downloading temporary data.")
        
        let temporaryDirectory = FileManager.default.temporaryDirectory
            .appendingPathComponent("org.openemu.OpenEmu", isDirectory: true)
            .appendingPathComponent(UUID().uuidString, isDirectory: true)
        let destination = temporaryDirectory.appendingPathComponent(url.lastPathComponent)
        
        try? FileManager.default.createDirectory(at: temporaryDirectory, withIntermediateDirectories: true, attributes: nil)
        
        try? FileManager.default.copyItem(at: location, to: destination)
        
        self.destination = destination
    }
}
