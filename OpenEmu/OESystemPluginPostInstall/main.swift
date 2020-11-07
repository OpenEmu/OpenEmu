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

// This tool searches a OpenEmu.app bundle for all system plugins contained
// within, and then updates the Info.plist accordingly.

import Foundation


// IMPORTANT: to be updated when adding a language to OpenEmu
let regionsToLanguages = [
    "eu": ["ca", "de", "en-GB", "es", "fr", "it", "nl", "pt", "ru"],
    "na": ["en", "fr-CA"],
    "jp": ["ja", "zh-Hans", "zh-Hant"]
]

// Extensions we don't want to include in the Info.plist because they are
// already included in it
let extensionBlacklist = [
    "tgz", "tar", "gz", "zip", "7z", "rar"
]

// Extensions we want to include with LSHandlerRank set to None
let extensionGrayListNone = [
    "cue", "m3u"
]

// Extensions we want to include with LSHandlerRank set to Alternate
let extensionGrayListAlternate = [
    "iso"
]

func regionalizedSystemName(plugin: Bundle, languageCode lcode: String) -> String?
{
    for (region, languages) in regionsToLanguages {
        if languages.contains(lcode) {
            if let regionalNames = plugin.object(forInfoDictionaryKey: "OERegionalizedSystemNames") as! [String: String]? {
                return regionalNames[region]
            } else {
                return nil
            }
        }
    }
    fatalError("cannot get region corresponding to language \(lcode); update regionsToLanguages in OESystemPluginPostInstall/main.swift")
}


func toGameFileName(systemName name: String, appBundle app: Bundle, localization loc: String) -> String
{
    let locBundle = Bundle.init(url: app.url(forResource: loc, withExtension: "lproj")!)!
    let format = locBundle.localizedString(forKey: "%@ Game", value: "%@ Game", table: "InfoPlist")
    return String.init(format: format, name)
}


func readLocalizedInfoPlistStrings(appBundle: Bundle) -> [String: [String: String]]
{
    let localizations = appBundle.localizations
    var res: [String: [String: String]] = [:]
    for localization in localizations {
        if localization == "Base" {
            continue
        }
        let plist = appBundle.resourceURL!.appendingPathComponent(localization + ".lproj/InfoPlist.strings")
        do {
            let data = try Data.init(contentsOf: plist)
            var strings = try PropertyListSerialization.propertyList(from: data, options: .mutableContainers, format: nil) as! [String: String]
            strings.removeValue(forKey: "%@ Game")
            res[localization] = strings
        } catch {
            res[localization] = [:]
        }
    }
    return res
}


func escape(string: String) -> String
{
    return String.init(string.flatMap { (c) -> [Character] in
        switch c {
            case "\n":
                return ["\\", "n"]
            case "\r":
                return ["\\", "r"]
            case "\t":
                return ["\\", "t"]
            case "\\":
                return ["\\", "\\"]
            case "\"":
                return ["\\", "\""]
            default:
                return [c]
        }
    })
}


func serializeStrings(_ strings: [String: String]) -> Data
{
    var buffer = ""
    for (key, value) in strings.sorted(by: {$0.0 < $1.0}) {
        buffer += "\n\"\(escape(string: key))\" = \"\(escape(string: value))\";\n"
    }
    return buffer.data(using: .utf8)!
}


func writeLocalizedInfoPlistStrings(appBundle: Bundle, strings dict: [String: [String: String]])
{
    let localizations = appBundle.localizations
    for localization in localizations {
        if localization == "Base" {
            continue
        }
        let plist = appBundle.resourceURL!.appendingPathComponent(localization + ".lproj/InfoPlist.strings")
        let data = serializeStrings(dict[localization]!)
        try! data.write(to: plist)
    }
}


func computeCommonExtensions(appBundle: Bundle, systemPlugins: [Bundle]) -> Set<String>
{
    var allSeenExts = Set<String>.init()
    var multiSystemExts = Set<String>.init()
    
    for plugin in systemPlugins {
        let allExts = plugin.object(forInfoDictionaryKey: "OEFileSuffixes") as! [String]
        
        let sanifiedExts = allExts.filter { (ext: String) -> Bool in !extensionBlacklist.contains(ext) }
        if sanifiedExts.count == 0 {
            continue
        }
        
        let extsAlreadySeen = allSeenExts.intersection(sanifiedExts)
        allSeenExts.formUnion(sanifiedExts)
        multiSystemExts.formUnion(extsAlreadySeen)
    }
    
    return multiSystemExts
}


func systemDocuments(typeName: String, extensions: [String]) -> [[String : Any]]
{
    let template: [String : Any] = [
        "NSDocumentClass": "OEGameDocument",
        "CFBundleTypeRole": "Viewer",
        "CFBundleTypeName": typeName
    ]
    
    let extensionsDefault = extensions.filter({ !(extensionGrayListAlternate+extensionGrayListNone).contains($0) }).sorted()
    let extensionsAlternate = extensions.filter({ extensionGrayListAlternate.contains($0) }).sorted()
    let extensionsNone = extensions.filter({ extensionGrayListNone.contains($0) }).sorted()
    
    var res: [[String : Any]] = []
    
    if extensionsDefault.count > 0 {
        res += [template.merging([
            "LSHandlerRank": "Default",
            "CFBundleTypeExtensions": extensionsDefault
        ]) { (a, _) in a }]
    }
    if extensionsAlternate.count > 0 {
        res += [template.merging([
            "LSHandlerRank": "Alternate",
            "CFBundleTypeExtensions": extensionsAlternate
        ]) { (a, _) in a }]
    }
    if extensionsNone.count > 0 {
        res += [template.merging([
            "LSHandlerRank": "None",
            "CFBundleTypeExtensions": extensionsNone
        ]) { (a, _) in a }]
    }
    
    return res
}


func updateInfoPlist(appBundle: Bundle, systemPlugins: [Bundle])
{
    var newTypes: [[String : Any]] = []
    var localizations = readLocalizedInfoPlistStrings(appBundle: appBundle)
    
    let multiSystemExts = computeCommonExtensions(appBundle: appBundle, systemPlugins: systemPlugins)
    
    for plugin in systemPlugins {
        let allExts = plugin.object(forInfoDictionaryKey: "OEFileSuffixes") as! [String]
        let sanifiedExts = allExts.filter { (ext: String) -> Bool in
            !extensionBlacklist.contains(ext) && !multiSystemExts.contains(ext)
        }
        if sanifiedExts.count == 0 {
            continue
        }
        
        let baseSystemName = plugin.object(forInfoDictionaryKey: "OESystemName") as! String
        let typeName = toGameFileName(systemName: baseSystemName, appBundle: appBundle, localization: "en")
        
        newTypes += systemDocuments(typeName: typeName, extensions: sanifiedExts)
        for (localization, var strings) in localizations {
            let localizedName = regionalizedSystemName(plugin: plugin, languageCode: localization) ?? baseSystemName
            strings[typeName] = toGameFileName(systemName: localizedName, appBundle: appBundle, localization: localization)
            localizations[localization] = strings
        }
    }
    
    // Generate one type for all extensions that are used by multiple systems
    let multiSysTypeName = toGameFileName(systemName: "OpenEmu", appBundle: appBundle, localization: "en")
    newTypes += systemDocuments(typeName: multiSysTypeName, extensions: Array<String>.init(multiSystemExts))
    for (localization, var strings) in localizations {
        strings[multiSysTypeName] = toGameFileName(systemName: "OpenEmu", appBundle: appBundle, localization: localization)
        localizations[localization] = strings
    }
    
    let infoPlistPath = (appBundle.bundlePath as NSString).appendingPathComponent("Contents/Info.plist")
    let infoPlistXml = FileManager.default.contents(atPath: infoPlistPath)!
    
    do {
        var infoPlist = try PropertyListSerialization.propertyList(from: infoPlistXml, options: .mutableContainers, format: nil) as! [String : Any]
        
        let existingTypes = infoPlist["CFBundleDocumentTypes"] as! [[String : Any]]
        let generatedTypeNames = Set<String>(newTypes.map({ $0["CFBundleTypeName"] as! String }))
        let existingTypesToKeep = existingTypes.filter({ !generatedTypeNames.contains($0["CFBundleTypeName"] as! String) })
        infoPlist["CFBundleDocumentTypes"] = existingTypesToKeep + newTypes
        
        let updatedInfoPlist = try PropertyListSerialization.data(fromPropertyList: infoPlist, format: .xml, options: 0)
        
        try updatedInfoPlist.write(to: URL(fileURLWithPath: infoPlistPath), options: .atomic)
        
        writeLocalizedInfoPlistStrings(appBundle: appBundle, strings: localizations)
        
    } catch {
        fatalError("Error updating Info.plist: \(error)")
    }
}


if CommandLine.arguments.count != 2 {
    print("usage: \(CommandLine.arguments[0]) OpenEmu.app")
    exit(1)
}
let appPath = CommandLine.arguments[1]
let appBundle = Bundle.init(path: appPath)!
let pluginsDir = appBundle.builtInPlugInsURL!.appendingPathComponent("Systems", isDirectory: true)
let pluginURLs = try! FileManager.default.contentsOfDirectory(at: pluginsDir, includingPropertiesForKeys: nil, options: [.skipsPackageDescendants, .skipsHiddenFiles, .skipsSubdirectoryDescendants])
let plugins = pluginURLs.map { (url: URL) -> Bundle in
    Bundle.init(url: url)!
}
updateInfoPlist(appBundle: appBundle, systemPlugins: plugins)

