// Copyright (c) 2022, OpenEmu Team
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

@objc
@objcMembers
class OEDBItem: NSManagedObject {
    
    var libraryDatabase: OELibraryDatabase! {
        return managedObjectContext?.libraryDatabase
    }
    
    @objc(createObjectInContext:)
    class func createObject(in context: NSManagedObjectContext) -> Self {
        return NSEntityDescription.insertNewObject(forEntityName: entityName, into: context) as! Self
    }
    
    @objc(objectWithURI:inContext:)
    class func object(withURI objectURI: URL, in context: NSManagedObjectContext) -> Self? {
        var objectID: NSManagedObjectID?
        
        context.performAndWait {
            objectID = context.persistentStoreCoordinator?.managedObjectID(forURIRepresentation: objectURI)
        }
        
        guard let objectID else { return nil }
        return object(with: objectID, in: context)
    }
    
    @objc(objectWithID:inContext:)
    class func object(with objectID: NSManagedObjectID, in context: NSManagedObjectContext) -> Self? {
        var result: Self?
        
        context.performAndWait {
            let object = context.object(with: objectID)
            
            let fetchRequest = NSFetchRequest<NSFetchRequestResult>(entityName: entityName)
            
            let predicate = NSPredicate(format: "(self == %@)", object)
            fetchRequest.predicate = predicate
            
            result = (try? context.fetch(fetchRequest) as? [Self])?.first
        }
        
        return result
    }
    
    var permanentID: NSManagedObjectID {
        var result = objectID
        
        if result.isTemporaryID {
            try? managedObjectContext?.obtainPermanentIDs(for: [self])
            result = objectID
        }
        
        return result
    }
    
    var permanentIDURI: URL {
        return permanentID.uriRepresentation()
    }
    
    class var entityName: String {
        preconditionFailure("entityName must be overriden")
    }
    
    var entityName: String {
        return Self.entityName
    }
    
    class func entityDescription(in context: NSManagedObjectContext) -> NSEntityDescription {
        return NSEntityDescription.entity(forEntityName: entityName, in: context)!
    }
    
    @discardableResult
    func save() -> Bool {
        var result = false
        
        let context = managedObjectContext
        context?.performAndWait {
            do {
                try context?.save()
                result = true
            } catch {
                DLog("\(error)")
            }
        }
        
        return result
    }
    
    func delete() {
        let context = managedObjectContext
        context?.performAndWait {
            context?.delete(self)
        }
    }
}
