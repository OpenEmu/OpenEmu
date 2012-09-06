//
//  OEImportItem.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 14.08.12.
//
//

#import <Foundation/Foundation.h>
typedef enum  {
    OEImportItemStatusIdle,
    OEImportItemStatusActive,
    OEImportItemStatusResolvableError,
    OEImportItemStatusFatalError,
    OEImportItemStatusFinished,
} OEImportItemState;

typedef enum  {
    OEImportStepCheckDirectory,
    OEImportStepHash,
    OEImportStepCheckHash,
    OEImportStepDetermineSystem,
    OEImportStepSyncArchive,
    OEImportStepOrganize,
    OEImportStepCreateRom,
    OEImportStepCreateGame,
} OEImportStep;

typedef void (^OEImportItemCompletionBlock)(void);

@interface OEImportItem : NSObject <NSObject>
@property (strong) NSURL    *URL;
@property        OEImportItemState importState;
@property        OEImportStep  importStep;
@property        NSMutableDictionary *importInfo;

@property          NSError  *error;
@property (strong) OEImportItemCompletionBlock completionHandler;

+ (id)itemWithURL:(NSURL*)url andCompletionHandler:(OEImportItemCompletionBlock)handler;
- (NSString*)localizedStatusMessage;
@end
