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

enum ShaderValue: Equatable, Hashable {
    case boolean(Bool)
    case double(Double)
}

extension ShaderValue: Encodable {
    
    enum CodingKeys: String, CodingKey {
        case valueType
        case associatedValue
    }
    
    enum CodingError: Error {
        case unknownValue
    }
    
    func encode(to encoder: Encoder) throws {
        var container = encoder.container(keyedBy: CodingKeys.self)
        switch self {
        case .boolean(let v):
            try container.encode(0, forKey: .valueType)
            try container.encode(v, forKey: .associatedValue)
        case .double(let v):
            try container.encode(1, forKey: .valueType)
            try container.encode(v, forKey: .associatedValue)
        }
    }
}

extension ShaderValue: Decodable {
    init(from decoder: Decoder) throws {
        let container = try decoder.container(keyedBy: CodingKeys.self)
        let valueType = try container.decode(Int.self, forKey: .valueType)
        switch valueType {
        case 0:
            let v = try container.decode(Bool.self, forKey: .associatedValue)
            self = .boolean(v)
        case 1:
            let v = try container.decode(Double.self, forKey: .associatedValue)
            self = .double(v)
        default:
            throw CodingError.unknownValue
        }
    }
}

struct ShaderPreset: Codable, Hashable, Identifiable {
    public let id: UUID
    public let shader: String
    public var name: String
    public var parameters: [String: ShaderValue]
}
