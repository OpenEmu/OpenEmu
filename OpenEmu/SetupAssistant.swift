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
import Finite

final class SetupAssistant: NSViewController {
    
    private enum State: Int {
        case videoIntro, welcome, coreSelection, lastScreen, end
    }
    
    static let hasFinishedKey = "setupAssistantFinished"
    private static let videoIntroductionDuration: TimeInterval = 10
    
    @IBOutlet private var replaceView: NSView!
    @IBOutlet private var coreListDownloadView: NSView!
    @IBOutlet private var coreListDownloadProgress: NSProgressIndicator!
    @IBOutlet private var welcomeView: NSView!
    @IBOutlet private var coreSelectionView: NSView!
    @IBOutlet private var lastStepView: NSView!
    @IBOutlet private var installCoreTableView: NSTableView!
    
    var completionBlock: (() -> Void)?
    
    /// installCoreTableView data source
    private var coresToDownload = [SetupCoreInfo]()
    private let viewTransition: CATransition = {
        let viewTransition = CATransition()
        viewTransition.type = .fade
        viewTransition.subtype = .fromRight
        viewTransition.timingFunction = CAMediaTimingFunction(name: .default)
        viewTransition.duration = 1
        return viewTransition
    }()
    private var fsm = StateMachine<State>(initial: .videoIntro) { c in
        c.allow(from: .videoIntro, to: .welcome)
        c.allow(from: .welcome , to: [.videoIntro, .coreSelection])
        c.allow(from: .coreSelection, to: [.welcome, .lastScreen])
        c.allow(from: .lastScreen, to: [.coreSelection, .end])
        c.allow(from: .end, to: .lastScreen)
    }
    
    private let startTimeOfSetupAssistant = Date()
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        attemptInitialCoreListUpdate()
        
        replaceView.wantsLayer = true
        replaceView.animations = ["subviews" : viewTransition]
        
        setUpFiniteStateMachine()
    }
    
    @IBAction private func backEvent(_ sender: Any?) {
        if let state = State(rawValue: fsm.state.rawValue-1) {
            try? fsm.transition(to: state)
        }
    }
    
    @IBAction private func nextEvent(_ sender: Any?) {
        if let state = State(rawValue: fsm.state.rawValue+1) {
            try? fsm.transition(to: state)
        }
    }
    
    private func setUpFiniteStateMachine() {
        
        // Video introduction
        
        fsm.onTransitions(from: .videoIntro, to: .welcome) { [unowned self] in
            dissolveToView(welcomeView)
        }
        
        // Welcome screen
        
        fsm.onTransitions(from: .videoIntro, to: .welcome) { [unowned self] in
            // Note: we are not worrying about a core being removed from the core list
            let knownCores = coresToDownload.compactMap { $0.core }
            for core in CoreUpdater.shared.coreList {
                if !knownCores.contains(core) {
                    coresToDownload.append(SetupCoreInfo(core: core))
                }
            }
            // Check if a core is set as default in AppDelegate
            for coreInfo in coresToDownload {
                let coreID = coreInfo.core.bundleIdentifier
                let systemIDs = coreInfo.core.systemIdentifiers
                
                for systemID in systemIDs {
                    let key = "defaultCore.\(systemID)"
                    if let defaultCoreID = UserDefaults.standard.string(forKey: key),
                       defaultCoreID.caseInsensitiveCompare(coreID) == .orderedSame {
                        coreInfo.isDefaultCore = true
                    }
                }
            }
        }
        fsm.onTransitions(from: .welcome, to: .coreSelection) { [unowned self] in
            goForwardToView(coreSelectionView)
        }
        
        // Core selection screen
        
        fsm.onTransitions(from: .coreSelection, to: .lastScreen) { [unowned self] in
            // Note: if the user selected a few cores and clicked next, we start downloading them.
            //       if the user goes back to the core selection screen, they shouldn't really deselect because
            //       once the download started, we don't cancel or remove it
            for coreInfo in coresToDownload {
                if coreInfo.isSelected && !coreInfo.isDownloadRequested {
                    coreInfo.core.start()
                    coreInfo.isDownloadRequested = true
                }
            }
        }
        fsm.onTransitions(from: .coreSelection, to: .welcome) { [unowned self] in
            goBackToView(welcomeView)
        }
        fsm.onTransitions(from: .coreSelection, to: .lastScreen) { [unowned self] in
            goForwardToView(lastStepView)
        }
        
        // Last screen
        
        fsm.onTransitions(from: .lastScreen, to: .coreSelection) { [unowned self] in
            goBackToView(coreSelectionView)
        }
        
        // This is the end, beautiful friend
        
        fsm.onTransitions(from: .lastScreen, to: .end) { [unowned self] in
            // Mark setup done
            UserDefaults.standard.set(true, forKey: Self.hasFinishedKey)
            
            if let completionBlock = completionBlock {
                completionBlock()
            }
        }
    }
    
    private func attemptInitialCoreListUpdate() {
        coreListDownloadProgress.startAnimation(nil)
        CoreUpdater.shared.checkForNewCores { error in
            self.initialCoreListUpdateDidComplete(error: error)
        }
    }
    
    private func initialCoreListUpdateDidComplete(error: Error?) {
        coreListDownloadProgress.stopAnimation(nil)
        
        if error == nil {
            coreListDownloadView.isHidden = true
            
            let deltaT = Date().timeIntervalSince(startTimeOfSetupAssistant)
            let timeLeft = TimeInterval(max(0, Self.videoIntroductionDuration - deltaT))
            DispatchQueue.main.asyncAfter(deadline: DispatchTime.now() + timeLeft * TimeInterval(NSEC_PER_SEC) / TimeInterval(NSEC_PER_SEC), execute: {
                self.nextEvent(nil)
            })
            return
        }
        
        let alert = OEAlert()
        alert.messageText = NSLocalizedString("OpenEmu could not download required data from the internet", comment: "Setup Assistant")
        alert.informativeText = String(format: NSLocalizedString("An error occurred while preparing the setup assistant. (%@) Make sure you are connected to the internet and try again.", comment: "Setup Assistant"), error!.localizedDescription)
        alert.defaultButtonTitle = NSLocalizedString("Retry", comment: "")
        alert.alternateButtonTitle = NSLocalizedString("Quit OpenEmu", comment: "")
        alert.beginSheetModal(for: view.window!) { result in
            if result == .alertSecondButtonReturn {
                NSApp.terminate(nil)
            }
            self.attemptInitialCoreListUpdate()
        }
    }
    
    // MARK: - View switching
    
    private func goBackToView(_ view: NSView) {
        switchToView(view, transitionType: .push, transitionSubtype: .fromLeft)
    }
    
    private func goForwardToView(_ view: NSView) {
        switchToView(view, transitionType: .push, transitionSubtype: .fromRight)
    }
    
    private func dissolveToView(_ view: NSView) {
        switchToView(view, transitionType: .fade)
    }
    
    private func switchToView(_ view: NSView, transitionType: CATransitionType, transitionSubtype: CATransitionSubtype? = nil) {
        viewTransition.type = transitionType
        viewTransition.subtype = transitionSubtype
        
        view.frame = replaceView.frame
        
        if replaceView.subviews.count == 0 {
            replaceView.animator().addSubview(view)
        } else if let oldView = replaceView.subviews.first {
            replaceView.animator().replaceSubview(oldView, with: view)
        }
    }
}

extension SetupAssistant: NSTableViewDataSource {
    
    func numberOfRows(in tableView: NSTableView) -> Int {
        return coresToDownload.count
    }
    
    func tableView(_ tableView: NSTableView, objectValueFor tableColumn: NSTableColumn?, row: Int) -> Any? {
        guard let identifier = tableColumn?.identifier else { return nil }
        let coreInfo = coresToDownload[row]
        
        switch identifier {
        case .checkbox:
            return coreInfo.isSelected
        case .coreName:
            return coreInfo.core.name
        case .coreSystem:
            return coreInfo.core.systemNames.joined(separator: ", ")
        default:
            return nil
        }
    }
    
    func tableView(_ tableView: NSTableView, setObjectValue object: Any?, for tableColumn: NSTableColumn?, row: Int) {
        if tableColumn?.identifier == .checkbox {
            let coreInfo = coresToDownload[row]
            coreInfo.isSelected = object as? Bool ?? false
        }
    }
}

extension SetupAssistant: NSTableViewDelegate {
    
    func tableView(_ tableView: NSTableView, shouldEdit tableColumn: NSTableColumn?, row: Int) -> Bool {
        if tableColumn?.identifier == .checkbox {
            let coreInfo = coresToDownload[row]
            return !coreInfo.isDownloadRequested
        }
        return false
    }
    
    func tableView(_ tableView: NSTableView, dataCellFor tableColumn: NSTableColumn?, row: Int) -> NSCell? {
        let cell = tableColumn?.dataCell(forRow: row) as? NSCell
        
        if tableColumn?.identifier == .checkbox {
            let coreInfo = coresToDownload[row]
            let buttonCell = cell as? NSButtonCell
            buttonCell?.isEnabled = !coreInfo.isDownloadRequested
            
            if coreInfo.isDefaultCore {
                buttonCell?.isEnabled = false
            }
        }
        return cell
    }
}

private extension NSUserInterfaceItemIdentifier {
    static let checkbox   = NSUserInterfaceItemIdentifier("enabled")
    static let coreName   = NSUserInterfaceItemIdentifier("emulatorName")
    static let coreSystem = NSUserInterfaceItemIdentifier("emulatorSystem")
}

private class SetupCoreInfo: NSObject {
    weak var core: CoreDownload!
    var isSelected = true
    var isDownloadRequested = false
    var isDefaultCore = false
    
    init(core: CoreDownload) {
        self.core = core
    }
}
