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
import CommonCrypto

enum Crypto {
    struct MD5 {
        static func digest<T: StringProtocol>(string: T) -> String {
            var digest = [UInt8](repeating: 0, count: Int(CC_MD5_DIGEST_LENGTH))
            string.withFastUTF8IfAvailable {
                _ = CC_MD5($0.baseAddress, CC_LONG($0.count), &digest)
            }
            var res = ""
            for byte in digest {
                res.append(String(format:"%02x", UInt8(byte)))
            }
            return res
        }
    }
    
    struct SHA256 {
        static func digest<T: StringProtocol>(string: T) -> String {
            var digest = [UInt8](repeating: 0, count: Int(CC_SHA256_DIGEST_LENGTH))
            string.withFastUTF8IfAvailable {
                _ = CC_SHA256($0.baseAddress, CC_LONG($0.count), &digest)
            }
            var res = ""
            for byte in digest {
                res.append(String(format:"%02x", UInt8(byte)))
            }
            return res
        }
    }
}

fileprivate extension StringProtocol {
    var isUTF8ContiguousStorageAvailable: Bool {
        utf8.withContiguousStorageIfAvailable { _ in 0 } != .none
    }
    
    func withFastUTF8IfAvailable<R>(
        _ f: (UnsafeBufferPointer<UInt8>) throws -> R
    ) rethrows -> R? {
        if isUTF8ContiguousStorageAvailable {
            return try utf8.withContiguousStorageIfAvailable(f)
        }
        var s = String(self)
        return try s.withUTF8(f)
    }
}
