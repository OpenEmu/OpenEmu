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

import Cocoa

final class GridSizeSlider: NSView {
    
    private(set) var slider: NSSlider!
    private(set) var minButton: NSButton!
    private(set) var maxButton: NSButton!
    
    @objc dynamic weak var target: AnyObject?
    
    required init?(coder: NSCoder) {
        super.init(coder: coder)
        commonInit()
    }
    
    override init(frame frameRect: NSRect) {
        super.init(frame: frameRect)
        commonInit()
    }
    
    private func commonInit() {
        slider = NSSlider(value: 1.0, minValue: 0.5, maxValue: 2.5, target: nil, action: nil)
        slider.controlSize = .small
        
        minButton = NSButton(image: NSImage(named: "grid_slider_min")!, target: nil, action: nil)
        minButton.isBordered = false
        
        maxButton = NSButton(image: NSImage(named: "grid_slider_max")!, target: nil, action: nil)
        maxButton.isBordered = false
        
        bind(NSBindingName(rawValue: #keyPath(target)), to: slider!, withKeyPath: #keyPath(NSSlider.target), options: nil)
        bind(NSBindingName(rawValue: #keyPath(target)), to: minButton!, withKeyPath: #keyPath(NSButton.target), options: nil)
        bind(NSBindingName(rawValue: #keyPath(target)), to: maxButton!, withKeyPath: #keyPath(NSButton.target), options: nil)
        
        addSubview(slider)
        addSubview(minButton)
        addSubview(maxButton)
        
        translatesAutoresizingMaskIntoConstraints = false
        slider.translatesAutoresizingMaskIntoConstraints = false
        minButton.translatesAutoresizingMaskIntoConstraints = false
        maxButton.translatesAutoresizingMaskIntoConstraints = false
        
        NSLayoutConstraint.activate([
            slider.widthAnchor.constraint(equalToConstant: 64),
            
            minButton.leadingAnchor.constraint(equalTo: leadingAnchor),
            minButton.trailingAnchor.constraint(equalTo: slider.leadingAnchor, constant: -5),
            maxButton.leadingAnchor.constraint(equalTo: slider.trailingAnchor, constant: 5),
            maxButton.trailingAnchor.constraint(equalTo: trailingAnchor),
            
            slider.centerYAnchor.constraint(equalTo: centerYAnchor),
            minButton.centerYAnchor.constraint(equalTo: slider.centerYAnchor),
            maxButton.centerYAnchor.constraint(equalTo: slider.centerYAnchor),
        ])
    }
    
    override var intrinsicContentSize: NSSize {
        NSSize(width: 104, height: 16)
    }
}
