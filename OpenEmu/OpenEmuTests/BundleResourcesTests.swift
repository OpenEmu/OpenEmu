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

import XCTest
@testable import OpenEmu

class BundleResourcesTests: XCTestCase {
    
    func testBundleResources() throws {
        let secretSound = try XCTUnwrap(Bundle.main.url(forResource: "secret", withExtension: "mp3"))
        XCTAssertNotNil(NSSound(contentsOf: secretSound, byReference: true))
        
        let cheatsDatabase = try XCTUnwrap(Bundle.main.url(forResource: "cheats-database", withExtension: "xml"))
        XCTAssertNoThrow(try Data(contentsOf: cheatsDatabase))
        
        let controllerDatabase = try XCTUnwrap(Bundle.main.url(forResource: "Controller-Database", withExtension: "plist"))
        XCTAssertNoThrow(try Data(contentsOf: controllerDatabase))
        
        let databaseModel = try XCTUnwrap(Bundle.main.url(forResource: "OEDatabase", withExtension: "momd"))
        XCTAssertNotNil(NSManagedObjectModel(contentsOf: databaseModel))
        
        let composition = try XCTUnwrap(Bundle.main.path(forResource: "OE Startup", ofType: "qtz"))
        XCTAssertNotNil(QCComposition(file: composition))
    }
    
    func testAssetCatalogResources() {
        XCTAssertNotNil(NSColor(named: "blank_slate_box_text"))
        
        XCTAssertNotNil(NSImage(named: "game_scanner_cancel"))
        XCTAssertNotNil(NSImage(named: "game_scanner_continue"))
        XCTAssertNotNil(NSImage(named: "game_scanner_pause"))
        XCTAssertNotNil(NSImage(named: "grid_slider_max"))
        XCTAssertNotNil(NSImage(named: "grid_slider_min"))
        XCTAssertNotNil(NSImage(named: "missing_artwork"))
        XCTAssertNotNil(NSImage(named: "quick_load_touch_bar_template"))
        XCTAssertNotNil(NSImage(named: "quick_save_touch_bar_template"))
        XCTAssertNotNil(NSImage(named: "spinner"))
        XCTAssertNotNil(NSImage(named: "wood_knob_vertical"))
    }
}
