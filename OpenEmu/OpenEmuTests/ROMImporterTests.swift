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

class ROMImporterTests: XCTestCase {
    
    func testArchiveUnarchiveOperationQueue() {
        let ri = ROMImporter(database: OELibraryDatabase.default!)
        
        let data: Data!
        let url1 = URL(string: "file://url/op1")!
        let url2 = URL(string: "file://url/op2")!
        
        do {
            var ops: [ImportOperation] = []
            var op: ImportOperation
            
            op = ImportOperation(url: url1, sourceURL: url1)
            ops.append(op)
            
            op = ImportOperation(url: url2, sourceURL: url2)
            ops.append(op)
            
            data = ri._data(forOperationQueue: ops)
            XCTAssertNotNil(data)
        }
        
        do {
            let ops: [ImportOperation]! = ri._operationQueue(from: data)
            XCTAssertNotNil(ops)
            XCTAssertEqual(ops.count, 2)
            var op: ImportOperation
            
            op = ops[0]
            XCTAssertEqual(op.url.absoluteString, url1.absoluteString)
            XCTAssertEqual(op.sourceURL.absoluteString, url1.absoluteString)
            
            op = ops[1]
            XCTAssertEqual(op.url.absoluteString, url2.absoluteString)
            XCTAssertEqual(op.sourceURL.absoluteString, url2.absoluteString)
        }
    }
}
