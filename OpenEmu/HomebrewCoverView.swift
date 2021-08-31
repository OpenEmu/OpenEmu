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

final class HomebrewCoverView: NSView {
    
    private static let imageDidLoadNotification = Notification.Name("OEURLImagesViewImageDidLoad")
    
    weak var target: AnyObject?
    var doubleAction: Selector?
    
    var urls: [URL]? {
        didSet {
            if urls != nil {
                currentImageIndex = 0
            }
        }
    }
    
    private let itemWidth: CGFloat = 10
    private let itemSpace: CGFloat = 4
    
    private let loadingIndicator = NSProgressIndicator()
    
    private var currentImage: NSImage?
    private var currentImageIndex = 0 {
        didSet {
            fetchCurrentImage()
        }
    }
    
    required init?(coder: NSCoder) {
        super.init(coder: coder)
        commonInit()
    }
    
    override init(frame frameRect: NSRect) {
        super.init(frame: frameRect)
        commonInit()
    }
    
    private func commonInit() {
        loadingIndicator.isIndeterminate = true
        loadingIndicator.style = .spinning
        loadingIndicator.controlSize = .regular
        loadingIndicator.isHidden = true
        loadingIndicator.usesThreadedAnimation = true
        
        addSubview(loadingIndicator)
        
        NotificationCenter.default.addObserver(self, selector: #selector(imageDidLoad(_:)), name: Self.imageDidLoadNotification, object: nil)
    }
    
    @objc private func imageDidLoad(_ notification: Notification) {
        let loadedImage = notification.userInfo?["URL"] as? URL
        let url = urls?[currentImageIndex]
        
        if loadedImage == url {
            DispatchQueue.main.async {
                self.currentImageIndex = self.currentImageIndex
            }
        }
    }
    
    override func viewDidMoveToSuperview() {
        loadingIndicator.sizeToFit()
        
        let x = bounds.midX - loadingIndicator.frame.width/2
        let y = bounds.midY - loadingIndicator.frame.height/2
        loadingIndicator.setFrameOrigin(NSPoint(x: x, y: y))
    }
    
    // MARK: - Frames
    
    private var pageSelectorRect: NSRect {
        let midX = bounds.midX
        let numberOfImages = CGFloat(urls?.count ?? 0)
        
        let width = numberOfImages*itemWidth + (numberOfImages-1)*itemSpace
        let minX = midX - width/2
        return NSRect(x: minX, y: 1, width: width, height: itemWidth)
    }
    
    private func rectForPageSelector(page: Int) -> NSRect {
        let midX = bounds.midX
        let numberOfImages = CGFloat(urls?.count ?? 0)
        
        let minX = midX - (numberOfImages*itemWidth + (numberOfImages-1)*itemSpace)/2 + (CGFloat(page)*itemWidth + (CGFloat(page)-1)*itemSpace)
        
        return NSRect(x: minX, y: 1, width: itemWidth, height: itemWidth)
    }
    
    private func rect(for image: NSImage) -> NSRect {
        let bounds = self.bounds
        let imageSize = image.size
        let aspectRatio = imageSize.width/imageSize.height
        
        let border: CGFloat = 10 // border for drawing image shadows
        let pageSelectorHeight: CGFloat = 10
        
        var rect = bounds.insetBy(dx: border, dy: border)
        
        // leave some room for page selector
        rect.size.height -= pageSelectorHeight
        
        // keep original aspect ratio
        if aspectRatio < bounds.width/bounds.height {
            rect.size.width = rect.height * aspectRatio
        } else {
            rect.size.height = rect.width / aspectRatio
        }
        
        // center horizontally
        rect.origin.x = (bounds.width - rect.width) / 2
        
        // adjust for page selector, again
        rect.origin.y += pageSelectorHeight
        
        return rect
    }
    
    // MARK: - Drawing
    
    override func draw(_ dirtyRect: NSRect) {
        super.draw(dirtyRect)
        
        NSColor.clear.setFill()
        bounds.fill()
        
        // Draw image
        if let image = currentImage {
            let imageRect = rect(for: image)
            
            NSGraphicsContext.current?.saveGraphicsState()
            let shadow = NSShadow()
            shadow.shadowBlurRadius = 3
            shadow.shadowOffset = NSMakeSize(0, -3)
            shadow.shadowColor = .black
            shadow.set()
            
            image.draw(in: imageRect, from: .zero, operation: .copy, fraction: 1)
            NSGraphicsContext.current?.restoreGraphicsState()
        }
        
        // Draw page selector
        let numberOfImages = urls?.count ?? 0
        for i in 0..<numberOfImages {
            let rect = rectForPageSelector(page: i)
            let path = NSBezierPath(ovalIn: rect)
            if i == currentImageIndex {
                NSColor.clear.setFill()
                NSColor.controlAccentColor.setStroke()
            } else {
                NSColor.disabledControlTextColor.setFill()
                NSColor.clear.setStroke()
            }
            
            path.fill()
            path.stroke()
        }
    }
    
    // MARK: - Interaction
    
    override func mouseDown(with event: NSEvent) {
        if event.clickCount == 2, let doubleAction = doubleAction {
            NSApp.sendAction(doubleAction, to: target, from: self)
        }
    }
    
    override func updateTrackingAreas() {
        trackingAreas.forEach { removeTrackingArea($0) }
        
        let trackingRect = pageSelectorRect
        let area = NSTrackingArea(rect: trackingRect, options: [.activeInKeyWindow, .mouseMoved], owner: self, userInfo: nil)
        addTrackingArea(area)
        
        let viewArea = NSTrackingArea(rect: bounds, options: [.activeInKeyWindow, .mouseEnteredAndExited], owner: self, userInfo: nil)
        addTrackingArea(viewArea)
    }
    
    override func mouseMoved(with event: NSEvent) {
        let trackingRect = pageSelectorRect
        let locationInWindow = event.locationInWindow
        let location = convert(locationInWindow, from: nil)
        
        if trackingRect.contains(location) {
            let x = location.x - trackingRect.minX
            let index = x / (itemWidth+itemSpace)
            
            if x > index.rounded(.down) * (itemWidth+itemSpace)-itemSpace + itemWidth {
                return
            }
            
            currentImageIndex = Int(index)
        }
    }
    
    override func mouseDragged(with event: NSEvent) {
        mouseMoved(with: event)
    }
    
    override func mouseEntered(with event: NSEvent) {
    }
    
    override func mouseExited(with event: NSEvent) {
        currentImageIndex = 0
    }
    
    // MARK: -
    
    private static let cache: NSCache<NSURL, NSImage> = {
        let cache = NSCache<NSURL, NSImage>()
        cache.countLimit = 50
        return cache
    }()
    private static var loading = [NSURL : Bool]()
    private static let lock = NSLock()
    
    private func fetchCurrentImage() {
        let url = urls?[currentImageIndex]
        
        Self.lock.lock()
        var cachedImage: NSImage?
        if let url = url {
            cachedImage = Self.cache.object(forKey: url as NSURL)
        }
        Self.lock.unlock()
        
        if let cachedImage = cachedImage {
            currentImage = cachedImage
            if !loadingIndicator.isHidden {
                loadingIndicator.stopAnimation(nil)
                loadingIndicator.isHidden = true
            }
        } else {
            currentImage = nil
            fetchImage(currentImageIndex)
            
            if loadingIndicator.isHidden {
                loadingIndicator.isHidden = false
                loadingIndicator.startAnimation(self)
            }
        }
        
        needsDisplay = true
    }
    
    private func fetchImage(_ index: Int) {
        guard let url = urls?[index] as NSURL? else { return }
        
        Self.lock.lock()
        if Self.cache.object(forKey: url) == nil && Self.loading[url] == nil {
            Self.loading[url] = true
            
            DispatchQueue.global(qos: .default).async {
                let image = NSImage(contentsOf: url as URL) ?? NSImage(named: NSImage.cautionName)!
                
                Self.lock.lock()
                Self.cache.setObject(image, forKey: url)
                Self.loading.removeValue(forKey: url)
                Self.lock.unlock()
                
                let userInfo = ["URL" : url]
                NotificationCenter.default.post(Notification(name: Self.imageDidLoadNotification, userInfo: userInfo))
            }
        }
        Self.lock.unlock()
    }
}
