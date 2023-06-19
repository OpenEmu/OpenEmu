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
import OpenEmuSystem

final class GameInfoHelper {
    
    static let shared = GameInfoHelper()
    
    var database: OpenVGDB? {
        return OpenVGDB.shared.isAvailable ? OpenVGDB.shared : nil
    }
    
    func gameInfo(withDictionary gameInfo: [String : Any]) -> [String : Any] {
        
        DispatchQueue(label: "org.openemu.OpenEmu.GameInfoHelper").sync {
            
            guard let database = database else {
                return [:]
            }
            
            lazy var resultDict: [String : Any] = [:]
            
            let systemIdentifier = gameInfo["systemIdentifier"] as! String
            var header = gameInfo["header"] as? String
            var serial = gameInfo["serial"] as? String
            let md5 = gameInfo["md5"] as? String
            let url = gameInfo["URL"] as? URL
            let archiveFileIndex = gameInfo["archiveFileIndex"] as? NSNumber
            
            var isSystemWithHashlessROM = hashlessROMCheck(forSystem: systemIdentifier)
            var isSystemWithROMHeader = headerROMCheck(forSystem: systemIdentifier)
            var isSystemWithROMSerial = serialROMCheck(forSystem: systemIdentifier)
            var headerSize = sizeOfROMHeader(forSystem: systemIdentifier)
            
            let DBMD5Key = "romHashMD5"
            let DBROMExtensionlessFileNameKey = "romExtensionlessFileName"
            let DBROMHeaderKey = "romHeader"
            let DBROMSerialKey = "romSerial"
            
            var key: String?
            var value: String?
            
            let determineQueryParams: (() -> Void) = {
                
                if value != nil {
                    return
                }
                
                // check if the system is 'hashless' in the db and instead match by filename (Arcade)
                if isSystemWithHashlessROM,
                   let url = url {
                    key = DBROMExtensionlessFileNameKey
                    value = (url.lastPathComponent as NSString).deletingPathExtension.lowercased()
                }
                // check if the system has headers in the db and instead match by header
                else if isSystemWithROMHeader {
                    key = DBROMHeaderKey
                    value = header?.uppercased()
                }
                // check if the system has serials in the db and instead match by serial
                else if isSystemWithROMSerial {
                    key = DBROMSerialKey
                    value = serial?.uppercased()
                }
                // if rom has no header we can use the hash we calculated at import
                else if headerSize == 0, let md5 = md5 {
                    key = DBMD5Key
                    value = md5.uppercased()
                }
            }
            
            determineQueryParams()
            
            if value == nil,
               let url = url {
                
                var removeFile = false
                var romURL: URL
                if let archiveFileIndex = archiveFileIndex as? Int,
                   let archiveURL = ArchiveHelper.decompressFileInArchive(at: url, atIndex: archiveFileIndex) {
                    romURL = archiveURL
                    removeFile = true
                } else {
                    // rom is no archive, use original file URL
                    romURL = url
                }
                
                var file: OEFile
                do {
                    file = try OEFile(url: romURL)
                } catch {
                    return [:]
                }
                
                let headerFound = OEDBSystem.header(for: file, forSystem: systemIdentifier)
                let serialFound = OEDBSystem.serial(for: file, forSystem: systemIdentifier)
                
                if headerFound == nil && serialFound == nil {
                    
                    if let md5 = try? FileManager.default.hashFile(at: romURL, fileOffset: Int(headerSize)) {
                        key = DBMD5Key
                        value = md5.uppercased()
                        resultDict["md5"] = value
                    }
                }
                else {
                    if let headerFound = headerFound {
                        header = headerFound
                        resultDict["header"] = headerFound
                    }
                    if let serialFound = serialFound {
                        serial = serialFound
                        resultDict["serial"] = serialFound
                    }
                    
                    determineQueryParams()
                }
                
                if removeFile {
                    try? FileManager.default.removeItem(at: romURL)
                }
            }
            
            if value == nil {
                // Still nothing to look up, force determineQueryParams to use hashes
                isSystemWithHashlessROM = false
                isSystemWithROMHeader = false
                isSystemWithROMSerial = false
                headerSize = 0
                
                determineQueryParams()
            }
            
            guard let key = key, let value = value else {
                return [:]
            }
            
            let sql = """
                SELECT DISTINCT releaseTitleName as 'gameTitle', releaseCoverFront as 'boxImageURL', releaseDescription as 'gameDescription', regionName as 'region'\
                FROM ROMs rom LEFT JOIN RELEASES release USING (romID) LEFT JOIN REGIONS region on (regionLocalizedID=region.regionID)\
                WHERE \(key) = '\(value)'
            """
            
            let results = (try? database.executeQuery(sql)) ?? []
            var result: [String : Any]?
            
            if results.count > 1 {
                
                // the database holds multiple regions for this rom (probably WORLD rom)
                // so we pick the preferred region if it's available or just any if not
                var preferredRegion = OELocalizationHelper.shared.regionName
                // TODO: Associate regionName's in the database with -[OELocalizationHelper regionName]'s
                if preferredRegion == "North America" {
                    preferredRegion = "USA"
                }
                
                if let dict = results.first(where: { $0["region"] as? String == preferredRegion }) {
                    result = dict
                }
                
                // preferred region not found, just pick one
                if result == nil {
                    result = results.last
                }
            }
            else {
                result = results.last
            }
            
            if var result = result {
                // remove the region key so the result can be directly passed to OEDBGame
                result.removeValue(forKey: "region")
                resultDict.merge(result) { (_, new) in new }
            }
            
            return resultDict
        }
    }
    
    func hashlessROMCheck(forSystem system: String) -> Bool {
        guard let database = database else { return false }
        
        let sql = "select systemhashless as 'hashless' from systems where systemoeid = '\(system)'"
        let result = try? database.executeQuery(sql)
        return result?.last?["hashless"] as? Int32 == 1
    }
    
    func headerROMCheck(forSystem system: String) -> Bool {
        guard let database = database else { return false }
        
        let sql = "select systemheader as 'header' from systems where systemoeid = '\(system)'"
        let result = try? database.executeQuery(sql)
        return result?.last?["header"] as? Int32 == 1
    }
    
    func serialROMCheck(forSystem system: String) -> Bool {
        guard let database = database else { return false }
        
        let sql = "select systemserial as 'serial' from systems where systemoeid = '\(system)'"
        let result = try? database.executeQuery(sql)
        // NOTE: As of OpenVGDB 28, the “systemSerial” column is of type “TEXT”.
        return (result?.last?["serial"] as? String) == "1" || (result?.last?["serial"] as? Int32) == 1
    }
    
    func sizeOfROMHeader(forSystem system: String) -> Int32 {
        guard let database = database else { return 0 }
        
        let sql = "select systemheadersizebytes as 'size' from systems where systemoeid = '\(system)'"
        let result = try? database.executeQuery(sql)
        return result?.last?["size"] as? Int32 ?? 0
    }
}
