#import <Foundation/Foundation.h>

// Options to handle 'broken' cue files:
#define OECUESheetImproveReadingByUsingBinExtension 1 // If a referneced file does not have an extension we automatically append .bin and see if that works better
#define OECUESheetImproveReadingByUsingSheetBin 1 // if there's only one referenced file which is not available we check if there's a bin file with the same name as the cue and use that instead

@interface OECUESheet : NSObject
- (id)initWithPath:(NSString*)path;
- (id)initWithPath:(NSString*)path andReferencedFilesDirectory:(NSString*)referencedFiles;

#pragma mark - File Handling
- (BOOL)moveReferencedFilesToPath:(NSString*)newDirectory withError:(NSError**)outError;
- (BOOL)copyReferencedFilesToPath:(NSString*)newDirectory withError:(NSError**)outError;

- (BOOL)allFilesAvailable;
- (NSArray*)referencedFiles;
@end
