// Copyright (c) 2022, OpenEmu Team
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
import CryptoKit
import CommonCrypto

// https://stackoverflow.com/questions/42935148/

extension FileManager {
    
    enum HashFunction {
        case md5, sha256
    }
    
    func hashFile(at url: URL, fileOffset: Int = 0, hashFunction: HashFunction = .md5) throws -> String {
        guard #available(macOS 10.15.4, *) else {
            return try legacyHashFile(at: url, fileOffset: fileOffset, hashFunction: hashFunction)
        }
        
        let bufferSize = 1024 * 32
        
        // Open file for reading:
        let file = try FileHandle(forReadingFrom: url)
        defer {
            try? file.close()
        }
        
        try? file.seek(toOffset: UInt64(fileOffset))
        
        let hexDigest: String
        
        switch hashFunction {
        case .md5:
            // Create and initialize MD5 context:
            var md5 = Insecure.MD5()
            
            // Read up to `bufferSize` bytes, until EOF is reached, and update MD5 context:
            while autoreleasepool(invoking: {
                if let data = try? file.read(upToCount: bufferSize),
                   data.count > 0 {
                    md5.update(data: data)
                    return true // Continue
                } else {
                    return false // End of file
                }
            }) {}
            
            // Compute the MD5 digest:
            let digest = md5.finalize()
            
            hexDigest = digest.map { String(format: "%02x", $0) }.joined()
        case .sha256:
            // Create and initialize SHA256 context:
            var sha256 = SHA256()
            
            // Read up to `bufferSize` bytes, until EOF is reached, and update SHA256 context:
            while autoreleasepool(invoking: {
                if let data = try? file.read(upToCount: bufferSize),
                   data.count > 0 {
                    sha256.update(data: data)
                    return true // Continue
                } else {
                    return false // End of file
                }
            }) {}
            
            // Compute the SHA256 digest:
            let digest = sha256.finalize()
            
            hexDigest = digest.map { String(format: "%02x", $0) }.joined()
        }
        
        return hexDigest
    }
    
    @available(macOS, deprecated: 10.15)
    private func legacyHashFile(at url: URL, fileOffset: Int = 0, hashFunction: HashFunction = .md5) throws -> String {
        
        let bufferSize = 1024 * 32
        
        // Open file for reading:
        let file = try FileHandle(forReadingFrom: url)
        defer {
            file.closeFile()
        }
        
        file.seek(toFileOffset: UInt64(fileOffset))
        
        let hexDigest: String
        
        switch hashFunction {
        case .md5:
            // Create and initialize MD5 context:
            var context = CC_MD5_CTX()
            CC_MD5_Init(&context)
            
            // Read up to `bufferSize` bytes, until EOF is reached, and update MD5 context:
            while autoreleasepool(invoking: {
                let data = file.readData(ofLength: bufferSize)
                if data.count > 0 {
                    data.withUnsafeBytes {
                        _ = CC_MD5_Update(&context, $0.baseAddress, numericCast(data.count))
                    }
                    return true // Continue
                } else {
                    return false // End of file
                }
            }) {}
            
            // Compute the MD5 digest:
            var digest: [UInt8] = Array(repeating: 0, count: Int(CC_MD5_DIGEST_LENGTH))
            _ = CC_MD5_Final(&digest, &context)
            
            hexDigest = digest.map { String(format: "%02x", $0) }.joined()
        case .sha256:
            // Create and initialize SHA256 context:
            var context = CC_SHA256_CTX()
            CC_SHA256_Init(&context)
            
            // Read up to `bufferSize` bytes, until EOF is reached, and update SHA256 context:
            while autoreleasepool(invoking: {
                let data = file.readData(ofLength: bufferSize)
                if data.count > 0 {
                    data.withUnsafeBytes {
                        _ = CC_SHA256_Update(&context, $0.baseAddress, numericCast(data.count))
                    }
                    return true // Continue
                } else {
                    return false // End of file
                }
            }) {}
            
            // Compute the SHA256 digest:
            var digest: [UInt8] = Array(repeating: 0, count: Int(CC_SHA256_DIGEST_LENGTH))
            _ = CC_SHA256_Final(&digest, &context)
            
            hexDigest = digest.map { String(format: "%02x", $0) }.joined()
        }
        
        return hexDigest
    }
}
