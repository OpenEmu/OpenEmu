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

import Cocoa

@objc(OEApplicationDelegate)
protocol OpenEmuApplicationDelegateProtocol: NSApplicationDelegate {
    
    @objc(application:willBeginModalSessionForWindow:)
    optional func application(_ application: OpenEmuApplication, willBeginModalSessionForWindow window: NSWindow)
    
    @objc(application:didBeginModalSessionForWindow:)
    optional func application(_ application: OpenEmuApplication, didBeginModalSessionForWindow window: NSWindow)
    
    @objc(applicationWillEndModalSession:)
    optional func applicationWillEndModalSession(application: OpenEmuApplication)
    
    @objc(applicationDidEndModalSession:)
    optional func applicationDidEndModalSession(application: OpenEmuApplication)
    
    @objc(spotlightStatusDidChangeForApplication:)
    optional func spotlightStatusDidChange(for application: OpenEmuApplication)
}

@objc(OEApplication)
class OpenEmuApplication: NSApplication {
    
    var isSpotlightFrontmost = false
    
    override func beginModalSession(for window: NSWindow) -> NSApplication.ModalSession {
        
        if let delegate = delegate as? OpenEmuApplicationDelegateProtocol {
            delegate.application?(self, willBeginModalSessionForWindow: window)
        }
        
        let modalSession = super.beginModalSession(for: window)
        
        if let delegate = delegate as? OpenEmuApplicationDelegateProtocol {
            delegate.application?(self, didBeginModalSessionForWindow: window)
        }
        
        return modalSession
    }
    
    override func endModalSession(_ session: NSApplication.ModalSession) {
        
        if let delegate = delegate as? OpenEmuApplicationDelegateProtocol {
            delegate.applicationWillEndModalSession?(application: self)
        }
        
        super.endModalSession(session)
        
        if let delegate = delegate as? OpenEmuApplicationDelegateProtocol {
            delegate.applicationDidEndModalSession?(application: self)
        }
    }
    
    override func sendEvent(_ event: NSEvent) {
        
        super.sendEvent(event)
        
        guard event.type.rawValue == 21 else {
            return
        }
        
        switch event.subtype.rawValue {
            
        case 16384:
            isSpotlightFrontmost = true
            
        case -32768:
            isSpotlightFrontmost = false
            
        default:
            return
        }
        
        if let delegate = delegate as? OpenEmuApplicationDelegateProtocol {
            delegate.spotlightStatusDidChange?(for: self)
        }
    }
}

var OEApp: OpenEmuApplication {
    return NSApp as! OpenEmuApplication
}
