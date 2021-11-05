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
import SQLite3

final class SQLiteDatabase {
    
    private static let SQLiteErrorDomain = "OESQLiteErrorDomain"
    
    private var connection: OpaquePointer?
    
    init(url: URL) throws {
        let file = (url as NSURL).fileSystemRepresentation
        let sqlErr = sqlite3_open(file, &connection)
        if sqlErr != SQLITE_OK {
            sqlite3_close(connection)
            let userInfo = [NSLocalizedDescriptionKey : String(cString: sqlite3_errmsg(connection))]
            let error = NSError(domain: Self.SQLiteErrorDomain, code: Int(sqlErr), userInfo: userInfo)
            throw error
        }
    }
    
    deinit {
        let sqlErr = sqlite3_close(connection)
        if sqlErr != SQLITE_OK {
            DLog("Could not close SQL database correctly. (Error Code \(sqlErr): \(String(cString: sqlite3_errmsg(connection))))")
        }
    }
    
    @objc(executeQuery:error:)
    func executeQuery(_ sql: String) throws -> [[String : Any]] {
        #if swift(>=5.5)
        lazy var result: [[String : Any]] = []
        #else
        var result: [[String : Any]] = []
        #endif
        
        try DispatchQueue(label: "org.openemu.OpenEmu.SQLiteDatabase").sync {
            var stmt: OpaquePointer?
            let cSQL = sql.cString(using: .utf8)
            
            let sqlErr = sqlite3_prepare_v2(connection, cSQL, -1, &stmt, nil)
            if sqlErr != SQLITE_OK {
                sqlite3_finalize(stmt)
                let userInfo = [NSLocalizedDescriptionKey : String(cString: sqlite3_errmsg(connection))]
                let error = NSError(domain: Self.SQLiteErrorDomain, code: Int(sqlErr), userInfo: userInfo)
                throw error
            }
            
            while sqlite3_step(stmt) == SQLITE_ROW {
                let columnCount = sqlite3_column_count(stmt)
                var dict = [String : Any](minimumCapacity: Int(columnCount))
                
                for column in 0..<columnCount {
                    if let cName = sqlite3_column_name(stmt, column),
                       let name = String(cString: cName, encoding: .utf8),
                       let value = valueOfSQLStatement(stmt, atColumn: column) {
                        dict[name] = value
                    }
                }
                
                result.append(dict)
            }
            
            sqlite3_finalize(stmt)
        }
        
        return result
    }
    
    private func valueOfSQLStatement(_ stmt: OpaquePointer!, atColumn column: Int32) -> Any? {
        
        var value: Any?
        let type = sqlite3_column_type(stmt, column)
        switch type {
        case SQLITE_INTEGER:
            value = sqlite3_column_int(stmt, column)
        case SQLITE_FLOAT:
            value = sqlite3_column_double(stmt, column)
        case SQLITE_TEXT:
            value = String(cString: sqlite3_column_text(stmt, column))
        case SQLITE_NULL:
            value = nil
        case SQLITE_BLOB:
            DLog("SQLite type 'BLOB' not supported right now")
            value = nil
        default:
            DLog("Unknown data type: \(type)")
            value = nil
        }
        
        return value
    }
}
