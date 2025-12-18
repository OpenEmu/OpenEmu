import Cocoa

@objcMembers final class VisualEffectManager: NSObject {
    static let shared = VisualEffectManager()

    @objc class func sharedManager() -> VisualEffectManager { return VisualEffectManager.shared }

    private var effects = NSHashTable<NSVisualEffectView>.weakObjects()
    private var accessibilityObserver: Any?
    private var appearanceObserver: Any?

    private override init() {
        // Observe accessibility reduce transparency
        accessibilityObserver = NotificationCenter.default.addObserver(forName: NSWorkspace.accessibilityDisplayOptionsDidChangeNotification, object: nil, queue: .main) { [weak self] _ in
            self?.applyGlobalSettings()
        }

        // Observe preference changes
        appearanceObserver = NotificationCenter.default.addObserver(forName: Notification.Name("OEAppearanceDidChange"), object: nil, queue: .main) { [weak self] _ in
            self?.applyGlobalSettings()
        }

        // Apply initial settings
        applyGlobalSettings()
    }

    deinit {
        if let obs = accessibilityObserver { NotificationCenter.default.removeObserver(obs) }
        if let obs = appearanceObserver { NotificationCenter.default.removeObserver(obs) }
    }

    /// Install an effect view pinned to the given container. The returned view is owned by the caller.
    @discardableResult
    func installEffect(in container: NSView, positioned: NSWindow.OrderingMode = .below, relativeTo sibling: NSView? = nil) -> NSVisualEffectView {
        let effect = NSVisualEffectView(frame: .zero)
        effect.translatesAutoresizingMaskIntoConstraints = false
        effect.wantsLayer = true
        effect.layer?.masksToBounds = false
        container.addSubview(effect, positioned: positioned, relativeTo: sibling)

        NSLayoutConstraint.activate([
            effect.leadingAnchor.constraint(equalTo: container.leadingAnchor),
            effect.trailingAnchor.constraint(equalTo: container.trailingAnchor),
            effect.topAnchor.constraint(equalTo: container.topAnchor),
            effect.bottomAnchor.constraint(equalTo: container.bottomAnchor),
        ])

        effects.add(effect)

        // Apply the current global appearance settings
        applySettings(to: effect)

        return effect
    }

    // Objective-C friendly helpers
    @objc func installEffectInContainer(_ container: NSView, positioned: NSInteger, relativeTo sibling: NSView?) -> NSVisualEffectView {
        let ordering: NSWindow.OrderingMode = (positioned == NSWindow.Above) ? .above : .below
        return installEffect(in: container, positioned: ordering, relativeTo: sibling)
    }

    @objc func removeEffectObjc(_ effect: NSVisualEffectView) {
        removeEffect(effect)
    }

    func removeEffect(_ effect: NSVisualEffectView) {
        effects.remove(effect)
        effect.removeFromSuperview()
    }

    /// Install an effect in the window titlebar using NSTitlebarAccessoryViewController.
    @discardableResult
    func installTitlebarEffect(on window: NSWindow, height: CGFloat = 36) -> NSTitlebarAccessoryViewController {
        let accessory = NSTitlebarAccessoryViewController()
        let container = NSView(frame: NSRect(x: 0, y: 0, width: 100, height: height))
        container.translatesAutoresizingMaskIntoConstraints = false
        accessory.view = container
        accessory.layoutAttribute = .top

        let effect = installEffect(in: container, positioned: .below, relativeTo: nil)
        // Pin effect to accessory view
        NSLayoutConstraint.activate([
            effect.heightAnchor.constraint(equalToConstant: height)
        ])

        window.addTitlebarAccessoryViewController(accessory)
        return accessory
    }

    @objc func installTitlebarEffectOnWindow(_ window: NSWindow, height: CGFloat) -> NSTitlebarAccessoryViewController {
        return installTitlebarEffect(on: window, height: height)
    }

    func applySettings(to effect: NSVisualEffectView) {
        let defaults = UserDefaults.standard
        let useLiquidGlass = defaults.object(forKey: "OEUseLiquidGlass") as? Bool ?? true
        guard useLiquidGlass, !NSWorkspace.shared.accessibilityDisplayShouldReduceTransparency else {
            effect.isHidden = true
            return
        }

        let requestedMaterial = defaults.string(forKey: "OEVisualEffectMaterial") ?? "sidebar"
        if #available(macOS 10.14, *) {
            switch requestedMaterial {
            case "contentBackground": effect.material = .contentBackground
            case "titlebar": effect.material = .titlebar
            default: effect.material = .sidebar
            }
        }

        let alpha = defaults.double(forKey: "OEVisualEffectOpacity")
        var effectiveAlpha = alpha > 0 ? CGFloat(alpha) : 1.0
        // If the user still has the original app default, tweak opacity per material for better contrast
        if alpha == 0.92 {
            if requestedMaterial == "contentBackground" {
                effectiveAlpha = 0.86
            } else if requestedMaterial == "titlebar" {
                effectiveAlpha = 0.9
            } else {
                effectiveAlpha = 0.92
            }
        }
        effect.alphaValue = effectiveAlpha
        effect.isHidden = false
    }

    private func applyGlobalSettings() {
        for effect in effects.allObjects {
            applySettings(to: effect)
        }
    }
}
