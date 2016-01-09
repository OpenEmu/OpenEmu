//
//  OEDBGameMigrationPolicy.swift
//  OpenEmu
//
//  Created by Christoph Leimbrock on 1/9/16.
//
//

import Cocoa

class OEDBGameMigrationPolicy: NSEntityMigrationPolicy
{
    override func createDestinationInstancesForSourceInstance(sInstance: NSManagedObject, entityMapping mapping: NSEntityMapping, manager: NSMigrationManager) throws
    {
        assert(manager.sourceModel.versionIdentifiers.count == 1, "Found a source model with various versionIdentifiers!");
        let version = manager.sourceModel.versionIdentifiers.first!;
        if version == "1.3" {
            for attributeMapping in mapping.attributeMappings! {
                let roms = sInstance.valueForKey("roms") as! Set<NSManagedObject>

                switch attributeMapping.name! {
                case "playCount":
                    let totalPlayCount: Int = roms.map({
                        (e: NSManagedObject) -> Int in
                        return e.valueForKey("playCount")?.integerValue ?? 0
                    }).reduce(0, combine: +)

                    attributeMapping.valueExpression = NSExpression(forConstantValue: totalPlayCount)

                case "playTime":
                    let totalPlayTime: Double = roms.map({
                        (e: NSManagedObject) -> Double in
                        return e.valueForKey("playTime")?.doubleValue ?? 0.0
                    }).reduce(0, combine: +)
                    attributeMapping.valueExpression = NSExpression(forConstantValue: totalPlayTime)

                default: break
                }
            }
        }

        try super.createDestinationInstancesForSourceInstance(sInstance, entityMapping: mapping, manager: manager)
    }
}
