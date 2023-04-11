// Copyright (c) 2023, OpenEmu Team
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

class OEGCSystemController: OESystemController {
    // Read header to detect GameCube ISO, GCM & CISO.
    override func canHandle(_ file: OEFile) -> OEFileSupport {
        // Handle gcm file and return early
        if ["gcm", "gcz", "rvz"].contains(file.fileExtension) {
            return .yes
        }

        var dataRange = NSRange(location: 0x1C, length: 4)

        // Handle ciso file and set the offset for the Magicword in compressed iso.
        if file.fileExtension == "ciso" {
            dataRange.location = 0x801C
        }

        let dataBuffer = file.readData(in: dataRange)
        // GameCube Magicword 0xC2339F3D
        let bytes: [UInt8] = [0xC2, 0x33, 0x9F, 0x3D]
        let comparisonData = Data(bytes: bytes, count: 4)

        if dataBuffer == comparisonData {
            return .yes
        }

        return .no
    }

    override func serialLookup(for file: OEFile) -> String? {
        var dataRange = NSRange(location: 0x0, length: 6)

        // Check if it's a CISO and adjust the Game ID offset location
        let magic = file.readASCIIString(in: NSRange(location: 0x0, length: 4))
        if magic == "CISO" {
            dataRange.location = 0x8000
        }

        // Read the game ID
        var gameID = file.readASCIIString(in: dataRange)

        // Read the disc number and version number bytes from the header.
        let headerDiscData = file.readData(in: NSRange(location: dataRange.location + 0x6, length: 1))
        let headerDiscByte = [UInt8](headerDiscData).first ?? 0
        let headerVersionData = file.readData(in: NSRange(location: dataRange.location + 0x7, length: 1))
        let headerVersionByte = [UInt8](headerVersionData).first ?? 0

        if headerDiscByte > 0 {
            gameID += "-DISC\(headerDiscByte + 1)"
        }

        if headerVersionByte > 0 {
            gameID += "-REV\(headerVersionByte)"
        }

        return gameID
    }
}
