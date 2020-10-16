// Copyright (c) 2020, OpenEmu Team
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
class GameDocumentController: NSDocumentController {
    
    class override var shared: GameDocumentController {
        return super.shared as! GameDocumentController
    }
    
    var gameDocuments = [OEGameDocument]()
    var reviewingUnsavedDocuments = false
    
    override func addDocument(_ document: NSDocument) {
        
        if let document = document as? OEGameDocument {
            gameDocuments.append(document)
        }
        
        super.addDocument(document)
    }
    
    override func removeDocument(_ document: NSDocument) {
        
        if let document = document as? OEGameDocument {
            gameDocuments.remove(at: gameDocuments.firstIndex(of: document)!)
        }
        
        super.removeDocument(document)
    }
    
    private func sendDelegateCallback(toTarget target: AnyObject, selector: Selector, documentController: NSDocumentController, didReviewAll: Bool, contextInfo: UnsafeMutableRawPointer?) {
        
        guard let method = class_getInstanceMethod(Swift.type(of: target), selector) else {
            return
        }
        
        let implementation = method_getImplementation(method)
        
        typealias Function = @convention(c) (AnyObject, Selector, NSDocumentController, Bool, UnsafeMutableRawPointer?) -> Void
        let function = unsafeBitCast(implementation, to: Function.self)
        
        function(target, selector, documentController, didReviewAll, contextInfo)
    }
    
    override func reviewUnsavedDocuments(withAlertTitle title: String?, cancellable: Bool, delegate: Any?, didReviewAllSelector: Selector?, contextInfo: UnsafeMutableRawPointer?) {
        
        
        
        guard !reviewingUnsavedDocuments else {
            sendDelegateCallback(toTarget: delegate as AnyObject, selector: didReviewAllSelector!, documentController: self, didReviewAll: false, contextInfo: contextInfo)
            return
        }
        reviewingUnsavedDocuments = true
        
        guard !gameDocuments.isEmpty else {
            reviewingUnsavedDocuments = false
            super.reviewUnsavedDocuments(withAlertTitle: title, cancellable: cancellable, delegate: delegate, didReviewAllSelector: didReviewAllSelector, contextInfo: contextInfo)
            return
        }
        
        if OEAlert.quitApplication().runModal() == .alertFirstButtonReturn {
            closeAllDocuments(withDelegate: delegate, didCloseAllSelector: didReviewAllSelector, contextInfo: contextInfo)
        } else {
            sendDelegateCallback(toTarget: delegate as AnyObject, selector: didReviewAllSelector!, documentController: self, didReviewAll: false, contextInfo: contextInfo)
        }
        
        reviewingUnsavedDocuments = false
    }
    
    override func closeAllDocuments(withDelegate delegate: Any?, didCloseAllSelector: Selector?, contextInfo: UnsafeMutableRawPointer?) {
        
        guard !gameDocuments.isEmpty else {
            super.closeAllDocuments(withDelegate: delegate, didCloseAllSelector: didCloseAllSelector, contextInfo: contextInfo)
            return
        }
        
        let documents = gameDocuments
        var remainingDocuments = documents.count
        
        for document in documents {
            
            document.canClose { document, shouldClose in
                
                remainingDocuments -= 1
                
                if shouldClose {
                    document!.close()
                }
                
                guard remainingDocuments == 0 else {
                    return
                }
                
                if !self.gameDocuments.isEmpty {
                    self.sendDelegateCallback(toTarget: delegate as AnyObject, selector: didCloseAllSelector!, documentController: self, didReviewAll: false, contextInfo: contextInfo)
                } else {
                    super.closeAllDocuments(withDelegate: delegate, didCloseAllSelector: didCloseAllSelector, contextInfo: contextInfo)
                }
            }
        }
    }
    
    fileprivate func setUpGameDocument(_ document: OEGameDocument, display displayDocument: Bool, fullScreen: Bool, completionHandler: ((OEGameDocument?, NSError?) -> Void)?) {
        
        addDocument(document)
        
        document.setupGame { success, error in
            
            if success {
                
                if displayDocument {
                    document.showInSeparateWindow(inFullScreen: fullScreen)
                }
                
                completionHandler?(document, nil)
                
            } else {
                completionHandler?(nil, error as NSError?)
            }
            NotificationCenter.default.post(name: .OEGameDocumentSetupDidFinish, object: nil)
        }
    }
    
    override func openDocument(withContentsOf url: URL, display displayDocument: Bool, completionHandler: @escaping (NSDocument?, Bool, Error?) -> Void) {
        
        super.openDocument(withContentsOf: url, display: false) { document, documentWasAlreadyOpen, error in
            
            if let document = document as? OEGameDocument {
                let fullScreen = UserDefaults.standard.bool(forKey: OEFullScreenGameWindowKey)
                self.setUpGameDocument(document, display: true, fullScreen: fullScreen, completionHandler: nil)
            }
            
            if let error = error as? OEGameDocumentErrorCodes, error.code == OEGameDocumentErrorCodes.importRequiredError  {
                completionHandler(nil, false, nil)
                return
            }
            
            completionHandler(document, documentWasAlreadyOpen, error)
            
            self.clearRecentDocuments(nil)
        }
    }
    
    override func openGameDocument(with game: OEDBGame?, display displayDocument: Bool, fullScreen: Bool, completionHandler: @escaping (OEGameDocument?, Error?) -> Void) {
        do {
            let document = try OEGameDocument(game: game, core: nil)
            setUpGameDocument(document, display: displayDocument, fullScreen: fullScreen, completionHandler: completionHandler)
        } catch {
            completionHandler(nil, error)
        }
    }
    
    override func openGameDocument(with rom: OEDBRom, display displayDocument: Bool, fullScreen: Bool, completionHandler: @escaping (OEGameDocument?, Error?) -> Void) {
        do {
            let document = try OEGameDocument(rom: rom, core: nil)
            setUpGameDocument(document, display: displayDocument, fullScreen: fullScreen, completionHandler: completionHandler)
        } catch {
            completionHandler(nil, error)
        }
    }
    
    override func openGameDocument(with saveState: OEDBSaveState, display displayDocument: Bool, fullScreen: Bool, completionHandler: @escaping (OEGameDocument?, Error?) -> Void) {
        do {
            let document = try OEGameDocument(saveState: saveState)
            setUpGameDocument(document, display: displayDocument, fullScreen: fullScreen, completionHandler: completionHandler)
        } catch {
            completionHandler(nil, error)
        }
    }
}

extension NSNotification.Name {
    static let OEGameDocumentSetupDidFinish = NSNotification.Name("OEGameDocumentSetupDidFinish")
}
