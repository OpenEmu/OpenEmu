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

#import <XCTest/XCTest.h>
#import "OEROMImporter+Private.h"

@interface OEROMImporterTests : XCTestCase

@end

@implementation OEROMImporterTests

- (void)testArchiveUnarchiveOperationQueue {
    OEROMImporter *ri = [[OEROMImporter alloc] init];
    
    NSData *data = nil;
    {
        NSMutableArray<OEImportOperation *> *ops = [NSMutableArray new];
        
        OEImportOperation *op = nil;
#define ADD_OP(ARG1, ARG2) \
        op = [OEImportOperation new]; \
        op.URL = [NSURL URLWithString:@ ARG1]; \
        op.sourceURL = [NSURL URLWithString:@ ARG2]; \
        [ops addObject:op];
        
        ADD_OP("file://url/op1", "file://source/op1");
        ADD_OP("file://url/op2", "file://source/op2");
        
        data = [ri dataForOperationQueue:ops];
        XCTAssertNotNil(data);
    }
    
    {
        NSArray<OEImportOperation *> *ops = [ri operationQueueFromData:data];
        XCTAssertEqual(2, ops.count);
        OEImportOperation *op = nil;
        
        op = ops[0];
        XCTAssertEqualObjects(op.URL.absoluteString, @"file://url/op1");
        XCTAssertEqualObjects(op.sourceURL.absoluteString, @"file://source/op1");
        op = ops[1];
        XCTAssertEqualObjects(op.URL.absoluteString, @"file://url/op2");
        XCTAssertEqualObjects(op.sourceURL.absoluteString, @"file://source/op2");
    }
}

@end
