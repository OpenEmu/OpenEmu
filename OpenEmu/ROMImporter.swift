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

extension Notification.Name {
    static let ROMImporterDidStart  = NSNotification.Name("OEROMImporterDidStart")
    static let ROMImporterDidCancel = NSNotification.Name("OEROMImporterDidCancel")
    static let ROMImporterDidPause  = NSNotification.Name("OEROMImporterDidPause")
    static let ROMImporterDidFinish = NSNotification.Name("OEROMImporterDidFinish")
    static let ROMImporterChangedItemCount = NSNotification.Name("OEROMImporterChangedItemCount")
    static let ROMImporterStoppedProcessingItem = NSNotification.Name("OEROMImporterStoppedProcessingItem")
}

@objc(OEROMImporter)
final class ROMImporter: NSObject {
    
    enum UserInfoKey: String {
        case itemKey = "OEROMImporterItemKey"
    }
    
    @objc(OEImporterStatus)
    enum Status: Int {
        case stopped, running, paused
    }
    
    @objc
    private(set) weak var database: OELibraryDatabase!
    private(set) var operationQueue: OperationQueue!
    var delegate: ROMImporterDelegate?
    @objc
    private(set) var status: Status = .stopped
    @objc
    private(set) var context: NSManagedObjectContext?
    
    private(set) var numberOfProcessedItems = 0
    private var _totalNumberOfItems = 0
    private(set) var totalNumberOfItems: Int {
        get {
            return _totalNumberOfItems
        }
        set {
            _totalNumberOfItems = newValue
            postNotification(name: .ROMImporterChangedItemCount)
            delegateResponds(to: #selector(ROMImporterDelegate.romImporterChangedItemCount(_:)), block: { self.delegate?.romImporterChangedItemCount?(self) })
        }
    }
    
    @objc
    init(database: OELibraryDatabase) {
        super.init()
        
        self.database = database
        
        let queue = OperationQueue()
        queue.maxConcurrentOperationCount = 1
        queue.name = "org.openemu.importqueue"
        operationQueue = queue
        
        let initializeMOCOp = BlockOperation(block: { [weak self] in
            let context = database.makeChildContext()
            context.name = "OEROMImporter"
            // An OEDBSystem object's `enabled` property could become out of sync (always enabled=true) in ROMImporter child context without this.
            context.automaticallyMergesChangesFromParent = true
            self?.context = context
        })
        queue.addOperations([initializeMOCOp], waitUntilFinished: true)
        queue.isSuspended = true
    }
    
    // MARK: - State
    
    @objc
    @discardableResult
    func saveQueue() -> Bool {
        operationQueue.isSuspended = true
        
        let url = database.importQueueURL
        
        // remove last saved queue if any
        try? FileManager.default.removeItem(at: url)
        
        if let queueData = data(forOperationQueue: operationQueue.operations) {
            do {
                try queueData.write(to: url, options: [.atomic])
                return true
            } catch {
                return false
            }
        }
        return true
    }
    
    fileprivate func data(forOperationQueue queue: [Operation]) -> Data? {
        // only pick OEImportOperations
        let operations = queue.filter { operation in
            if let operation = operation as? OEImportOperation {
                return !operation.isFinished && !operation.isCancelled
            } else {
                return false
            }
        }
        if !operations.isEmpty {
            return try? NSKeyedArchiver.archivedData(withRootObject: operations, requiringSecureCoding: true)
        }
        return nil
    }
    
    fileprivate func operationQueue(from data: Data) -> [OEImportOperation]? {
        let classes = [NSArray.self, OEImportOperation.self]
        let operations = try? NSKeyedUnarchiver.unarchivedObject(ofClasses: classes, from: data) as? [OEImportOperation]
        return operations
    }
    
    @objc
    @discardableResult
    func loadQueue() -> Bool {
        let url = database.importQueueURL
        
        // read previously stored data
        guard let queueData = try? Data(contentsOf: url) else { return false }
        
        // remove file if reading was successfull
        try? FileManager.default.removeItem(at: url)
        
        if let operations = operationQueue(from: queueData),
           !operations.isEmpty {
            numberOfProcessedItems = 0
            totalNumberOfItems = operations.count
            operationQueue.addOperations(operations, waitUntilFinished: false)
            return true
        }
        return false
    }
    
    // MARK: - Importing items
    
    @objc(importItemsAtURL:intoCollectionWithID:withCompletionHandler:)
    @discardableResult
    func importItem(at url: URL, intoCollectionWith collectionID: NSManagedObjectID? = nil, withCompletionHandler handler: OEImportItemCompletionBlock? = nil) -> Bool {
        
        // check operation queue for items that have already import the same url
        let item = operationQueue.operations.first { item in
            if let item = item as? OEImportOperation,
               item.url == url {
                return true
            } else {
                return false
            }
        }
        if item == nil,
           let item = OEImportOperation(url: url, in: self) {
            item.completionHandler = handler
            item.collectionID = collectionID
            
            addOperation(item)
            return true
        }
        return false
    }
    
    @objc(importItemsAtURLs:intoCollectionWithID:withCompletionHandler:)
    @discardableResult
    func importItems(at urls: [URL], intoCollectionWith collectionID: NSManagedObjectID? = nil, withCompletionHandler handler: OEImportItemCompletionBlock? = nil) -> Bool {
        
        var success = false
        for url in urls {
            success = importItem(at: url, intoCollectionWith: collectionID, withCompletionHandler: handler) || success
        }
        return success
    }
    
    // MARK: -
    
    @objc
    func addOperation(_ operation: OEImportOperation) {
        if operation.completionBlock == nil {
            operation.completionBlock = completionHandler(for: operation)
        }
        
        operationQueue.addOperation(operation)
        totalNumberOfItems += 1
        start()
        
        postNotification(name: .ROMImporterChangedItemCount)
        delegateResponds(to: #selector(ROMImporterDelegate.romImporterChangedItemCount(_:)), block: { self.delegate?.romImporterChangedItemCount?(self) })
    }
    
    func rescheduleOperation(_ operation: OEImportOperation) {
        let copy = operation.copy() as! OEImportOperation
        copy.completionBlock = completionHandler(for: copy)
        
        operationQueue.addOperation(copy)
        
        numberOfProcessedItems -= 1
    }
    
    private func completionHandler(for operation: OEImportOperation) -> () -> Void {
        let importer = self
        return {
            let state = operation.exitStatus
            if state == .success {
                importer.numberOfProcessedItems += 1
            } else if state == .errorFatal {
                importer.numberOfProcessedItems += 1
            } else if state == .errorResolvable {
            }
            
            if let block = operation.completionHandler {
                // save so changes propagate to other stores
                try? importer.context?.save()
                
                DispatchQueue.main.asyncAfter(deadline: DispatchTime(uptimeNanoseconds: 1)) {
                    block(operation.romObjectID)
                }
            }
            
            self.postNotification(name: .ROMImporterStoppedProcessingItem, userInfo: [UserInfoKey.itemKey : operation])
            self.delegateResponds(to: #selector(ROMImporterDelegate.romImporter(_:stoppedProcessingItem:)), block: { self.delegate?.romImporter?(self, stoppedProcessingItem: operation) })
            
            if importer.operationQueue.operationCount == 0 {
                importer.finish()
            }
            
            operation.completionHandler = nil
        }
    }
    
    // MARK: - Controlling Import
    
    @objc
    func start() {
        DLog("\(operationQueue.operationCount != 0 && status != .running)")
        
        if operationQueue.operationCount != 0 && status != .running {
            status = .running
            operationQueue.isSuspended = false
            postNotification(name: .ROMImporterDidStart)
            delegateResponds(to: #selector(ROMImporterDelegate.romImporterDidStart(_:)), block: { self.delegate?.romImporterDidStart?(self) })
        }
    }
    
    func pause() {
        DLog("\(status == .running)")
        
        if status == .running {
            status = .paused
            operationQueue.isSuspended = true
            postNotification(name: .ROMImporterDidPause)
            delegateResponds(to: #selector(ROMImporterDelegate.romImporterDidPause(_:)), block: { self.delegate?.romImporterDidPause?(self) })
        }
    }
    
    func togglePause() {
        if status == .paused {
            DLog("start")
            start()
        } else if status == .running {
            DLog("pause")
            pause()
        } else {
            DLog("nothing")
        }
    }
    
    func cancel() {
        DLog("cancel")
        
        status = .stopped
        operationQueue.cancelAllOperations()
        
        numberOfProcessedItems = 0
        totalNumberOfItems = 0
        operationQueue.isSuspended = true
        
        postNotification(name: .ROMImporterDidCancel)
        delegateResponds(to: #selector(ROMImporterDelegate.romImporterDidCancel(_:)), block: { self.delegate?.romImporterDidCancel?(self) })
    }
    
    func finish() {
        DLog("Finish")
        
        status = .stopped
        operationQueue.cancelAllOperations()
        
        numberOfProcessedItems = 0
        totalNumberOfItems = 0
        operationQueue.isSuspended = true
        
        postNotification(name: .ROMImporterDidFinish)
        delegateResponds(to: #selector(ROMImporterDelegate.romImporterDidFinish(_:)), block: { self.delegate?.romImporterDidFinish?(self) })
    }
    
    // MARK: -
    
    private func delegateResponds(to selector: Selector, block: @escaping () -> Void) {
        if delegate?.responds(to: selector) ?? false {
            if Thread.isMainThread {
                block()
            } else {
                DispatchQueue.main.sync(execute: block)
            }
        }
    }
    
    private func postNotification(name: NSNotification.Name, userInfo: [AnyHashable : Any]? = nil) {
        let block = {
            NotificationCenter.default.post(name: name, object: self, userInfo: userInfo)
        }
        
        if Thread.isMainThread {
            block()
        } else {
            DispatchQueue.main.sync(execute: block)
        }
    }
}

// MARK: - Importer Delegate

@objc protocol ROMImporterDelegate: NSObjectProtocol {
    @objc optional func romImporterDidStart(_ importer: ROMImporter)
    @objc optional func romImporterDidCancel(_ importer: ROMImporter)
    @objc optional func romImporterDidPause(_ importer: ROMImporter)
    @objc optional func romImporterDidFinish(_ importer: ROMImporter)
    @objc optional func romImporterChangedItemCount(_ importer: ROMImporter)
    @objc optional func romImporter(_ importer: ROMImporter, startedProcessingItem item: OEImportOperation)
    @objc optional func romImporter(_ importer: ROMImporter, changedProcessingPhaseOfItem item: OEImportOperation)
    @objc optional func romImporter(_ importer: ROMImporter, stoppedProcessingItem item: OEImportOperation)
}

#if DEBUG
extension ROMImporter {
    
    func _data(forOperationQueue queue: [Operation]) -> Data? {
        return data(forOperationQueue: queue)
    }
    
    func _operationQueue(from data: Data) -> [OEImportOperation]? {
        return operationQueue(from: data)
    }
}
#endif
