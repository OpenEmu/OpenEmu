/*
 Copyright (c) 2016, OpenEmu Team

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the OpenEmu Team nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

import Foundation

extension NSDocumentController {
    
    var currentGameDocument: OEGameDocument? {
        
        if let document = currentDocument as? OEGameDocument {
            return document
        }
        
        for document in NSApplication.shared.orderedDocuments {
            if let document = document as? OEGameDocument {
                return document
            }
        }
        
        return nil
    }
    
    @objc(openGameDocumentWithGame:display:fullScreen:completionHandler:)
    func openGameDocument(with game: OEDBGame?, display displayDocument: Bool, fullScreen: Bool, completionHandler: @escaping (OEGameDocument?, Error?) -> Void) {
        fatalError("Method must be implemented by a subclass.")
    }
    
    @objc(openGameDocumentWithRom:display:fullScreen:completionHandler:)
    func openGameDocument(with rom: OEDBRom, display displayDocument: Bool, fullScreen: Bool, completionHandler: @escaping (OEGameDocument?, Error?) -> Void) {
        fatalError("Method must be implemented by a subclass.")
    }
    
    @objc(openGameDocumentWithSaveState:display:fullScreen:completionHandler:)
    func openGameDocument(with saveState: OEDBSaveState, display displayDocument: Bool, fullScreen: Bool, completionHandler: @escaping (OEGameDocument?, Error?) -> Void) {
        fatalError("Method must be implemented by a subclass.")
    }
}
