//
//  OEDBSmartCollectionMigrationPolicy.swift
//  OpenEmu
//
//  Created by Christoph Leimbrock on 1/6/16.
//
//

import CoreData

class OEDBSmartCollectionMigrationPolicy : NSEntityMigrationPolicy
{
    override func createDestinationInstancesForSourceInstance(sInstance: NSManagedObject, entityMapping mapping: NSEntityMapping, manager: NSMigrationManager) throws
    {
        assert(manager.sourceModel.versionIdentifiers.count == 1, "Found a source model with various versionIdentifiers!");
        let version = manager.sourceModel.versionIdentifiers.first!;
        if version == "1.3"
        {
            for attributeMapping in mapping.attributeMappings!
            {
                switch attributeMapping.name!
                {
                case "predicateData":
                    let predicate = NSPredicate(value: true)
                    let data = NSKeyedArchiver.archivedDataWithRootObject(predicate)
                    attributeMapping.valueExpression = NSExpression(forConstantValue: data)
                    break
                case "fetchSortKey":
                    attributeMapping.valueExpression = NSExpression(forConstantValue: "importDate")
                    break
                default: break
                }
            }
        }

        try super.createDestinationInstancesForSourceInstance(sInstance, entityMapping: mapping, manager: manager)
    }
}