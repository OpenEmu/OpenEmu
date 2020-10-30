// Copyright (c) 2019, OpenEmu Team
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

// Extensions for OpenEmu.app
extension OEShadersModel {
    func setDefaultShader(_ name: String) {
        UserDefaults.standard.set(name, forKey: Preferences.global.key)
    }
    
    func setShader(name: String, forSystem id: String) {
        UserDefaults.standard.set(name, forKey: Preferences.system(id).key)
    }
}

extension OEShadersModel.OEShaderModel {
    @objc
    public func write(parameters params: [OEShaderParamValue], identifier: String) {
        var state = [String]()
        
        for p in params.filter({ !$0.isInitial }) {
            state.append("\(p.name)=\(p.value)")
        }
        if state.isEmpty {
            UserDefaults.standard.removeObject(forKey: Params.system(self.name, identifier).key)
        } else {
            UserDefaults.standard.set(state.joined(separator: ";"), forKey: Params.system(self.name, identifier).key)
        }
    }
}

