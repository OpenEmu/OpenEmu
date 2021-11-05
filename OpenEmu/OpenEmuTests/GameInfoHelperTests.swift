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

import XCTest
@testable import OpenEmu

class GameInfoHelperTests: XCTestCase {
    
    func testROMChecks() {
        let helper = GameInfoHelper.shared
        
        let arcade = "openemu.system.arcade"
        let saturn = "openemu.system.saturn"
        let psx = "openemu.system.psx"
        let nes = "openemu.system.nes"
        let c64 = "openemu.system.c64"
        let iOS = "openemu.system.iOS"
        
        XCTAssertTrue(helper.hashlessROMCheck(forSystem: arcade))
        XCTAssertTrue(helper.headerROMCheck(forSystem: saturn))
        XCTAssertTrue(helper.serialROMCheck(forSystem: psx))
        XCTAssert(helper.sizeOfROMHeader(forSystem: nes) == 16)
        
        XCTAssertFalse(helper.hashlessROMCheck(forSystem: c64))
        XCTAssertFalse(helper.headerROMCheck(forSystem: c64))
        XCTAssertFalse(helper.serialROMCheck(forSystem: c64))
        XCTAssert(helper.sizeOfROMHeader(forSystem: c64) == 0)
        
        XCTAssertFalse(helper.hashlessROMCheck(forSystem: iOS))
        XCTAssertFalse(helper.headerROMCheck(forSystem: iOS))
        XCTAssertFalse(helper.serialROMCheck(forSystem: iOS))
        XCTAssert(helper.sizeOfROMHeader(forSystem: iOS) == 0)
    }
}
