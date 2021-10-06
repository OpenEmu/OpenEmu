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
import OpenEmuKit.OECorePlugin
import OpenEmuKit.OESystemPlugin

@objc(OEBlankSlateView)
class BlankSlateView: NSView {
    
    private let ContainerWidth: CGFloat = 427
    private let ContainerHeight: CGFloat = 418
    
    private let BoxHeight: CGFloat = 261 // height of box
    private let BoxImageToTop: CGFloat = 52 // image top to view top distance
    private let BoxImageToBottom: CGFloat = 70 // image bottom to view bottom distance
    private let BoxTextToTop: CGFloat = 202 // distance of box top to text
    
    private let HeadlineHeight: CGFloat = 41 // height of headline
    private let HeadlineToTop: CGFloat = 337 // space between headline and view top
    
    private let BottomTextHeight: CGFloat = 67 // height of instructional text
    private let BottomTextTop: CGFloat = 357
    
    private let CoreToTop: CGFloat = 357 // space between core icon and view top
    private let CoreX: CGFloat = 263 // x coordinate of core icon
    
    private let RightColumnX: CGFloat = 310
    
    private var containerView: NSView?
    @objc weak var delegate: BlankSlateViewDelegate?
    private lazy var dragIndicationLayer = CALayer()
    private lazy var lastDragOperation: NSDragOperation = []
    
    required init?(coder: NSCoder) {
        super.init(coder: coder)
        commonInit()
    }
    
    override init(frame frameRect: NSRect) {
        super.init(frame: frameRect)
        commonInit()
    }
    
    private func commonInit() {
        
        let layer = CALayer()
        layer.delegate = self
        self.layer = layer
        
        if #available(macOS 11.0, *) {
            effectiveAppearance.performAsCurrentDrawingAppearance {
                dragIndicationLayer.borderColor = NSColor.controlAccentColor.cgColor
            }
        } else {
            NSAppearance.current = effectiveAppearance
            dragIndicationLayer.borderColor = NSColor.controlAccentColor.cgColor
        }
        dragIndicationLayer.borderWidth = 2
        dragIndicationLayer.cornerRadius = 8
        dragIndicationLayer.isHidden = true
        
        layer.addSublayer(dragIndicationLayer)
    }
    
    override func viewDidChangeEffectiveAppearance() {
        if #available(macOS 11.0, *) {
            effectiveAppearance.performAsCurrentDrawingAppearance {
                dragIndicationLayer.borderColor = NSColor.controlAccentColor.cgColor
            }
        } else {
            NSAppearance.current = effectiveAppearance
            dragIndicationLayer.borderColor = NSColor.controlAccentColor.cgColor
        }
    }
    
    private var representedSystemPlugin: OESystemPlugin? {
        didSet {
            guard representedSystemPlugin != nil else { return }
            representedCollectionName = nil
            setupView(systemPlugin: representedSystemPlugin!)
        }
    }
    
    private var representedCollectionName: String? {
        didSet {
            guard representedCollectionName != nil else { return }
            representedSystemPlugin = nil
            setupView(collectionName: representedCollectionName!)
        }
    }
    
    @objc var representedObject: Any? {
        didSet {
            guard representedObject != nil else { return }
            
            let containerSize = NSSize(width: ContainerWidth, height: ContainerHeight)
            
            let containerFrame = NSRect(x: ceil((bounds.width - containerSize.width) / 2),
                                        y: ceil((bounds.height - containerSize.height) / 2),
                                    width: containerSize.width,
                                   height: containerSize.height)
            
            let container = NSView(frame: containerFrame)
            container.autoresizingMask = [.minXMargin, .maxXMargin, .minYMargin, .maxYMargin]
            
            containerView = container
            
            setupViewForRepresentedObject()
            
            // Remove current blank slate subview
            subviews.last?.removeFromSuperview()
            
            addSubview(container)
        }
    }
    
    func setupViewForRepresentedObject() {
        
        if let system = representedObject as? OEDBSystem {
            
            representedSystemPlugin = system.plugin
        }
        else if let collection = representedObject as? GameCollectionViewItemProtocol {
            
            representedCollectionName = collection.collectionViewName
        }
        else if representedObject is OEDBSaveStatesMedia ||
                representedObject is OEDBScreenshotsMedia {
            
            setupRepresentedMediaType()
        }
        else {
            DLog("Unknown represented object: \(String(describing: representedObject))")
        }
    }
    
    private func setupRepresentedMediaType() {
        representedSystemPlugin = nil
        representedCollectionName = nil
        
        if representedObject is OEDBScreenshotsMedia {
            
            let imageView = makeImageView(withImageNamed: "blank_slate_camera")
            setupBox(text: NSLocalizedString("No Screenshots Found", comment: "No screen shots found"), imageView: imageView)
            
            addLeftHeadline(NSLocalizedString("Screenshots", comment: ""))
            
            let text = NSLocalizedString("Create your personal collection of screenshots. To take a screenshot, you can use the keyboard shortcut ⌘ + T while playing a game.", comment: "")
            addInformationalText(text)
        }
        else if representedObject is OEDBSaveStatesMedia {
            
            let imageView = makeImageView(withImageNamed: "blank_slate_arrow")
            setupBox(text: NSLocalizedString("Drag & Drop Save States Here", comment: "Blank Slate DnD Save States Here"), imageView: imageView)
            
            addLeftHeadline(NSLocalizedString("Save States", comment: ""))
            
            let text = NSLocalizedString("With OpenEmu you can save your progress at any time using save states. We will even make auto saves for you, when you leave a game. Come back here to get an overview of all your saves.", comment: "")
            addInformationalText(text)
        }
    }
    
    private func setupView(systemPlugin plugin: OESystemPlugin) {
        guard let containerFrame = containerView?.frame else { return }
        
        setupDragAndDropBox()
        addLeftHeadline(plugin.systemName)
        
        let rect = NSRect(x: 0,
                          y: containerFrame.height - BottomTextHeight - BottomTextTop,
                      width: containerFrame.width / 12 * 7,
                     height: BottomTextHeight)
        
        let textView = ArrowCursorTextView(frame: rect.insetBy(dx: -4, dy: 0))
        
        let text: String
        if plugin.supportsDiscsWithDescriptorFile {
            text = String(format: NSLocalizedString("%@ games will appear here. Check out %@ on how to add disc-based games.", comment: ""), plugin.systemName, NSLocalizedString("this guide", comment: ""))
        } else {
            text = String(format: NSLocalizedString("%@ games you add to OpenEmu will appear in this Console Library", comment: ""), plugin.systemName)
        }
        
        textView.drawsBackground = false
        textView.isEditable = false
        textView.selectionGranularity = .selectByCharacter
        textView.delegate = self
        textView.font = .systemFont(ofSize: 12)
        textView.textColor = .labelColor
        textView.textContainerInset = .zero
        
        let paraStyle = NSMutableParagraphStyle()
        paraStyle.lineSpacing = 2
        
        textView.defaultParagraphStyle = paraStyle
        textView.string = text
        
        var attributes = textView.typingAttributes
        attributes[.cursor] = NSCursor.arrow
        
        textView.typingAttributes = attributes
        textView.markedTextAttributes = attributes
        textView.selectedTextAttributes = attributes
        
        if plugin.supportsDiscsWithDescriptorFile {
            let linkRange = (textView.string as NSString).range(of: NSLocalizedString("this guide", comment: "this guide"))
            textView.textStorage?.addAttribute(.link, value: URL.userGuideCDBasedGames, range: linkRange)
        }
        
        var linkAttributes = attributes
        linkAttributes[.underlineStyle] = NSUnderlineStyle.single.rawValue
        linkAttributes[.cursor] = NSCursor.pointingHand
        textView.linkTextAttributes = linkAttributes
        
        containerView?.addSubview(textView)
        
        // Get core plugins that can handle the system
        let pluginFilter = NSPredicate(block: { evaluatedPlugin, bindings in
            return (evaluatedPlugin as! OECorePlugin).systemIdentifiers.contains(plugin.systemIdentifier)
        })
        
        let pluginsForSystem = (OECorePlugin.allPlugins as NSArray).filtered(using: pluginFilter)
        let extraspace = CGFloat(max(0, pluginsForSystem.count - 2))
        
        let coreIconRect = NSRect(x: CoreX,
                                  y: containerFrame.height - 40 - CoreToTop + 16 * extraspace,
                              width: 40,
                             height: 40)
        
        let coreIconView = NSImageView(frame: coreIconRect)
        coreIconView.image = NSImage(named: "blank_slate_core_icon")
        
        containerView?.addSubview(coreIconView)
        
        coreIconView.unregisterDraggedTypes()
        
        let labelRect = NSRect(x: RightColumnX,
                               y: containerFrame.height - 16 - BottomTextTop + 16 * extraspace,
                           width: containerFrame.width,
                          height: 16)
        
        let coreSuppliedByLabel = NSTextField(labelWithString: NSLocalizedString("Core Provided By…", comment: ""))
        coreSuppliedByLabel.frame = labelRect
        coreSuppliedByLabel.font = .systemFont(ofSize: 12)
        coreSuppliedByLabel.textColor = .labelColor
        
        containerView?.addSubview(coreSuppliedByLabel)
        
        for (idx, core) in pluginsForSystem.enumerated() {
            
            guard let core = core as? OECorePlugin else { continue }
            
            let projectURL = core.infoDictionary["OEProjectURL"] as? String
            let name = core.displayName ?? ""
            
            // Create weblink button for current core
            var frame = NSRect(x: RightColumnX,
                               y: containerFrame.height - 2 * 16 - BottomTextTop - 16 * CGFloat(idx) - 2 + 16 * extraspace + 4,
                           width: containerFrame.width - RightColumnX,
                          height: 20)
            
            if #available(macOS 11.0, *) {
                frame.origin.x += 2;
            }
            
            let gotoButton = TextButton(frame: frame)
            gotoButton.autoresizingMask = .width
            gotoButton.alignment = .left
            gotoButton.target = self
            gotoButton.action = #selector(gotoProjectURL(_:))
            gotoButton.title = name
            gotoButton.toolTip = .localizedStringWithFormat(NSLocalizedString("Takes you to the %@ project website", comment: "Weblink tooltip"), name)
            gotoButton.cell?.representedObject = projectURL
            gotoButton.font = .boldSystemFont(ofSize: 12)
            gotoButton.textColor = .labelColor
            gotoButton.textColorHover = .labelColor.withSystemEffect(.rollover)
            gotoButton.showArrow = true
            
            gotoButton.sizeToFit()
            
            containerView?.addSubview(gotoButton)
        }
    }
    
    // TODO: Use a different wording for smart collections, as games cannot be added manually.
    private func setupView(collectionName: String) {
        setupDragAndDropBox()
        addLeftHeadline(NSLocalizedString("Collections", comment: ""))
        
        let text = String.localizedStringWithFormat(NSLocalizedString("Create a personal game selection. To add to a collection, select a game from your console library and drag it to ’%@’ in the sidebar.", comment: ""), collectionName)
        addInformationalText(text)
    }
    
    func setupBox(text: String, imageView: NSView) {
        guard let containerFrame = containerView?.frame else { return }
        
        let boxImageRect = NSRect(x: 0,
                                  y: containerFrame.height - BoxHeight,
                              width: containerFrame.width,
                             height: BoxHeight)
        
        let boxImageView = NSImageView(frame: boxImageRect)
        boxImageView.image = NSImage(named: "blank_slate_box")
        
        containerView?.addSubview(boxImageView)
        
        boxImageView.unregisterDraggedTypes()
        
        let height = BoxHeight - BoxImageToBottom - BoxImageToTop
        let width: CGFloat = 300
        imageView.frame = NSRect(x: round(containerFrame.width - width) / 2,
                                 y: containerFrame.height - height - BoxImageToTop,
                             width: width,
                            height: height)
        
        containerView?.addSubview(imageView)
        
        let color = NSColor(named: "blank_slate_box_text")
        
        let dragAndDropHereRect = NSRect(x: 0,
                                         y: containerFrame.height - 30 - BoxTextToTop,
                                     width: containerFrame.width,
                                    height: 31)
        
        let dragAndDropHereField = NSTextField(labelWithString: text)
        dragAndDropHereField.frame = dragAndDropHereRect
        dragAndDropHereField.font = .systemFont(ofSize: 24)
        dragAndDropHereField.textColor = color
        dragAndDropHereField.alignment = .center
        
        containerView?.addSubview(dragAndDropHereField)
    }
    
    private func setupDragAndDropBox() {
        let imageView = makeImageView(withImageNamed: "blank_slate_arrow")
        setupBox(text: NSLocalizedString("Drag & Drop Games Here", comment: "Blank Slate DnD Here"), imageView: imageView)
    }
    
    func addLeftHeadline(_ text: String) {
        guard let containerFrame = containerView?.frame else { return }
        
        let headlineFrame = NSRect(x: -1,
                                   y: containerFrame.height - 30 - HeadlineToTop,
                               width: containerFrame.width,
                              height: HeadlineHeight)
        
        let headlineField = NSTextField(labelWithString: text)
        headlineField.frame = headlineFrame
        headlineField.font = .systemFont(ofSize: 20, weight: .medium)
        headlineField.textColor = .secondaryLabelColor
        
        containerView?.addSubview(headlineField)
    }
    
    func addInformationalText(_ text: String) {
        guard let containerFrame = containerView?.frame else { return }
        
        var rect = NSRect(x: 0,
                          y: 0,
                      width: containerFrame.width,
                     height: BottomTextHeight)
        rect.origin.y = containerFrame.height - rect.height - BottomTextTop
        
        let textView = NSTextView(frame: rect.insetBy(dx: -4, dy: 0))
        textView.string = text
        textView.drawsBackground = false
        textView.isEditable = false
        textView.isSelectable = false
        textView.font = .systemFont(ofSize: 12)
        textView.textColor = .labelColor
        textView.textContainerInset = NSSize(width: 0, height: 0)
        
        containerView?.addSubview(textView)
    }
    
    private func makeImageView(withImageNamed imageName: String) -> NSImageView {
        
        let imageView = NSImageView(frame: .zero)
        imageView.image = NSImage(named: imageName)
        imageView.imageScaling = .scaleNone
        imageView.imageAlignment = .alignTop
        imageView.unregisterDraggedTypes()
        
        return imageView
    }
    
    // MARK: - Actions
    
    @objc func gotoProjectURL(_ sender: NSButton) {
        let urlString = sender.cell?.representedObject as? String ?? ""
        if let url = URL(string: urlString) {
            NSWorkspace.shared.open(url)
        }
    }
}

// MARK: - CALayerDelegate

extension BlankSlateView: CALayerDelegate {
    
    func layoutSublayers(of layer: CALayer) {
        CATransaction.begin()
        CATransaction.setDisableActions(true)
        
        let bounds = self.bounds
        let contentLayoutRect = window!.contentLayoutRect
        
        for layer in layer.sublayers ?? [] {
            
            if layer == dragIndicationLayer {
                
                var dragIndicationFrame = bounds
                dragIndicationFrame.size.height = contentLayoutRect.height
                
                layer.frame = dragIndicationFrame.insetBy(dx: 1, dy: 1)
            } else if layer != subviews.last?.layer {
                layer.frame = bounds
            }
        }
        
        CATransaction.commit()
    }
}

// MARK: - NSTextViewDelegate

extension BlankSlateView: NSTextViewDelegate {
    
    func textView(_ textView: NSTextView, clickedOnLink link: Any, at charIndex: Int) -> Bool {
        if let link = link as? URL {
            NSWorkspace.shared.open(link)
        }
        return true
    }
    
    func textView(_ textView: NSTextView, willChangeSelectionFromCharacterRange oldSelectedCharRange: NSRange, toCharacterRange newSelectedCharRange: NSRange) -> NSRange {
        return NSRange(location: 0, length: 0)
    }
}

// MARK: - NSDraggingDestination

extension BlankSlateView {
    
    override func draggingEntered(_ sender: NSDraggingInfo) -> NSDragOperation {
        // The delegate has to be able to validate and accept drops, if it can't do then there is no need to drag anything around
        if let delegate = delegate,
           delegate.responds(to: #selector(BlankSlateViewDelegate.blankSlateView(_:validateDrop:))),
           delegate.responds(to: #selector(BlankSlateViewDelegate.blankSlateView(_:acceptDrop:))) {
            lastDragOperation = delegate.blankSlateView!(self, validateDrop: sender)
        }
        
        dragIndicationLayer.isHidden = lastDragOperation == []
        
        return lastDragOperation
    }
    
    override func draggingUpdated(_ sender: NSDraggingInfo) -> NSDragOperation {
        if let delegate = delegate,
           delegate.responds(to: #selector(BlankSlateViewDelegate.blankSlateView(_:draggingUpdated:))) {
            lastDragOperation = delegate.blankSlateView!(self, draggingUpdated: sender)
        }
        
        dragIndicationLayer.isHidden = lastDragOperation == []
        
        return lastDragOperation
    }
    
    override func draggingExited(_ sender: NSDraggingInfo?) {
        dragIndicationLayer.isHidden = true
    }
    
    override func performDragOperation(_ sender: NSDraggingInfo) -> Bool {
        dragIndicationLayer.isHidden = true
        return delegate?.responds(to: #selector(BlankSlateViewDelegate.blankSlateView(_:acceptDrop:))) ?? false && delegate?.blankSlateView?(self, acceptDrop: sender) ?? false
    }
}

// MARK: - BlankSlateViewDelegate

@objc(OEBlankSlateViewDelegate)
protocol BlankSlateViewDelegate: NSObjectProtocol {
    @objc optional func blankSlateView(_ gridView: BlankSlateView, validateDrop sender: NSDraggingInfo) -> NSDragOperation
    @objc optional func blankSlateView(_ gridView: BlankSlateView, draggingUpdated sender: NSDraggingInfo) -> NSDragOperation
    @objc optional func blankSlateView(_ gridView: BlankSlateView, acceptDrop sender: NSDraggingInfo) -> Bool
}
