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

class OEN64SystemController: OESystemController {
    override func serialLookup(for file: OEFile) -> String? {
        // Read the first 4 bytes of the header to get the 'magic word' in hex
        let hexString = file.readData(in: NSRange(location: 0, length: 4)).hexString

        var serial = file.readData(in: NSRange(location: 0x3B, length: 4))

        if serial.count < 4 || serial.contains(0) {
            return nil
        }

        // Detect rom formats using 'magic word' hex and swap byte order to [ABCD] if neccessary
        // .z64 rom is N64 native (big endian) with header 0x80371240 [ABCD], no need to swap
        if hexString == "80371240" {}

        // Byteswapped .v64 rom with header 0x37804012 [BADC]
        else if hexString == "37804012" {
            serial = Data([serial[1], serial[0], serial[3], serial[2]])
        }

        // Little endian .n64 rom with header 0x40123780 [DCBA]
        else if hexString == "40123780" {
            serial = Data([serial[3], serial[2], serial[1], serial[0]])
        }

        // Wordswapped .n64 rom with header 0x12408037 [CDAB]
        else if hexString == "12408037" {
            serial = Data([serial[2], serial[3], serial[0], serial[1]])
        }

        let gameID = String(bytes: serial, encoding: .ascii)
        return gameID
    }

    override var coverAspectRatio: CGFloat {
        return OELocalizationHelper.shared.isRegionJPN ? 1.37 : 0.7
    }
}
