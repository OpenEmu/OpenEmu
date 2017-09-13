/*
 Copyright (c) 2017, OpenEmu Team
 
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

/* XML cheats database format:
     
     <?xml version="1.0" encoding="UTF-8"?>
     <systems>
        <system id="openemu.system.nes">
            <game title="Some Game">
                <hashes>
                    <hash md5="9e107d9d372bb6826bd81d3542a419d6" />
                </hashes>
                <cheats>
                    <cheat code="048E:88" type="Action Replay" description="Invincibility" />
                    <cheat code="010F4ED8+01424FD8" type="GameShark" description="Infinite Money" />
                    <cheat code="69C4-AF6C+62C4-A7DC" type="Game Genie" description="Infinite time" />
                </cheats>
            </game>
        </system>
     ...
     </systems>
*/

@objc(OECheats)
@objcMembers
class Cheats: NSObject, XMLParserDelegate {
    
    private(set) var allCheats = [NSMutableDictionary]()
    private var didFindMD5Hash = false
    private let md5Hash: String
    
    init(md5Hash: String) {
        
        self.md5Hash = md5Hash
        
        super.init()
        
        findCheats()
    }
    
    private func findCheats() {
        
        // TODO: Read cheats database from server instead of bundling with the app for easy updating.
        let cheatsDatabaseFilename = "cheats-database.xml"
        let cheatsDatabasePath = (Bundle.main.resourcePath! as NSString).appendingPathComponent(cheatsDatabaseFilename)
        
        let xml = try! Data(contentsOf: URL(fileURLWithPath: cheatsDatabasePath))
        
        let parser = XMLParser(data: xml)
        parser.delegate = self
        
        parser.parse()
    }
    
    // MARK: - XMLParserDelegate
    
    func parser(_ parser: XMLParser, didStartElement elementName: String, namespaceURI: String?, qualifiedName qName: String?, attributes attributeDict: [String : String] = [:]) {
        
        // Parse until we find our MD5 hash.
        if elementName == "hash" && attributeDict["md5"]! as String == md5Hash {
            
            didFindMD5Hash = true
            
        // Parse cheats where MD5 hash was found.
        } else if didFindMD5Hash && elementName == "cheat" {
          
            allCheats.append(NSMutableDictionary(dictionary:[
                "code": attributeDict["code"]!,
                "type": attributeDict["type"]!,
                "description": attributeDict["description"]!,
                "enabled": false
            ]))
        }
    }
    
    func parser(_ parser: XMLParser, didEndElement elementName: String, namespaceURI: String?, qualifiedName qName: String?) {
        
        // Stop parsing after all cheats have been found for MD5.
        if didFindMD5Hash && elementName == "cheats" {
            parser.abortParsing()
        }
    }
}
