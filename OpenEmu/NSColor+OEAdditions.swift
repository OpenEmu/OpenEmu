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

import Cocoa

extension NSColor {
    
    @objc(colorFromHexString:)
    convenience init?(from colorString: String) {
        
        guard colorString.count == 7 && colorString.prefix(1) == "#" else {
            return nil
        }
        
        let colorString = colorString.dropFirst(1).uppercased()
        
        var colorRGB: UInt64 = 0
        let hexScanner = Scanner(string: colorString)
        hexScanner.scanHexInt64(&colorRGB)
        
        let components = [CGFloat((colorRGB & 0xFF0000) >> 16) / 255, // r
                          CGFloat((colorRGB & 0x00FF00) >>  8) / 255, // g
                          CGFloat((colorRGB & 0x0000FF) >>  0) / 255, // b
                          CGFloat(1)]                                 // a
        
        self.init(colorSpace: NSColorSpace.deviceRGB, components: components, count: 4)
    }
    
    @objc func toString() -> String? {
        
        let rgbColor = usingColorSpace(NSColorSpace.deviceRGB)
        let result = String(format: "#%02x%02x%02x", Int(rgbColor!.redComponent * 255), Int(rgbColor!.greenComponent * 255), Int(rgbColor!.blueComponent * 255))
        
        return result
    }
}
