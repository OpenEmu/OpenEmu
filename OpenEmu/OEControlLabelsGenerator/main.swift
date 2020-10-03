// OpenEmu system plugins controls label strings extraction
//
// Copyright (c) 2016-2020, OpenEmu Team
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

var outputComments = false

var groupLabels: NSMutableSet
var buttonLabels: NSMutableSet
var labelToSystems: NSMutableDictionary

groupLabels = NSMutableSet()
buttonLabels = NSMutableSet()
labelToSystems = NSMutableDictionary()


func addLabels(fromPlist plist: NSDictionary)
{
    let systemName = plist["OESystemName"] as! NSString
    let controlsArray = plist["OEControlListKey"] as! NSArray
    addLabels(fromArray: controlsArray, systemName:systemName)
}


func addLabels(fromArray a: NSArray, systemName: NSString)
{
    for item in a {
        if (item as AnyObject).isKind(of: NSArray.self) {
            addLabels(fromArray: item as! NSArray, systemName: systemName)
        } else if (item as AnyObject).isKind(of: NSString.self) {
            addSystem(name: systemName, label:item as! NSString)
            groupLabels.add(item)
        } else if (item as AnyObject).isKind(of: NSDictionary.self) {
            let label = (item as! NSDictionary)["OEControlListKeyLabelKey"] as! NSString
            addSystem(name: systemName, label:label)
            buttonLabels.add(label)
        }
    }
}


func addSystem(name s: NSString, label: NSString)
{
    if let systems = labelToSystems[label] as? NSArray {
        labelToSystems.setObject(systems.adding(s), forKey:label)
    } else {
        labelToSystems.setObject([s], forKey:label)
    }
}


func systems(forLabel l: NSString) -> String {
    let systems = labelToSystems[l] as! NSArray
    
    if systems.count == 0 {
        return ""
    }
    
    var output = systems[0] as! String
    for system in systems.dropFirst() {
        output = output + ", " + (system as! String)
    }
    return output
}


var args = CommandLine.arguments.dropFirst()
if let outputCommentsIdx = args.firstIndex(of: "--output-comments") {
    outputComments = true
    args.remove(at: outputCommentsIdx)
}

let currentDirectory = URL.init(fileURLWithPath: FileManager.default.currentDirectoryPath)
let allPlugins: [URL]
do {
    allPlugins = try FileManager.default.contentsOfDirectory(
            at: currentDirectory,
            includingPropertiesForKeys: nil,
            options: [.skipsSubdirectoryDescendants])
        .filter{(url: URL) -> Bool in url.pathExtension == "oesystemplugin"}
} catch {
    allPlugins = []
}
for plugin in allPlugins {
    let pluginBundle = Bundle.init(url: plugin)
    addLabels(fromPlist: pluginBundle!.infoDictionary! as NSDictionary)
}

guard allPlugins.count > 0 || args.count > 1 else {
    print("Usage: \(CommandLine.arguments[0]) [--output-comments] system1-info.plist ")
    print("       system2-info.plist... > labels.strings")
    print("Manually specifying the info plists is not required if there is any")
    print("system plugin bundle in the current directory")
    exit(0)
}

for file in args {
    let tmp = NSDictionary(contentsOfFile: file)!
    addLabels(fromPlist: tmp)
}

print("/* Group Labels */\n")
for label in groupLabels.sortedArray(using: [NSSortDescriptor(key: nil, ascending: true)]) {
    let line = "\"\(label)\" = \"\(label)\";"
    if !outputComments {
        print(line)
    } else {
        print("\(line)  // \(systems(forLabel: label as! NSString))")
    }
    print()
}

print("\n/* Button Labels */\n")
for label in buttonLabels.sortedArray(using: [NSSortDescriptor(key: nil, ascending: true)]) {
    let line = "\"\(label)\" = \"\(label)\";"
    if !outputComments {
        print(line)
    } else {
        print("\(line)  // \(systems(forLabel: label as! NSString))")
    }
    print()
}


