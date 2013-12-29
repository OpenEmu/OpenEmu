#import "XADUnarchiver.h"
#import "CSHandle.h"

@interface XADPlatform:NSObject {}

// Archive entry extraction.
+(XADError)extractResourceForkEntryWithDictionary:(NSDictionary *)dict
unarchiver:(XADUnarchiver *)unarchiver toPath:(NSString *)destpath;
+(XADError)updateFileAttributesAtPath:(NSString *)path
forEntryWithDictionary:(NSDictionary *)dict parser:(XADArchiveParser *)parser
preservePermissions:(BOOL)preservepermissions;
+(XADError)createLinkAtPath:(NSString *)path withDestinationPath:(NSString *)link;

// Archive post-processing.
+(id)readCloneableMetadataFromPath:(NSString *)path;
+(void)writeCloneableMetadata:(id)metadata toPath:(NSString *)path;
+(BOOL)copyDateFromPath:(NSString *)src toPath:(NSString *)dest;
+(BOOL)resetDateAtPath:(NSString *)path;

// Path functions.
+(BOOL)fileExistsAtPath:(NSString *)path;
+(BOOL)fileExistsAtPath:(NSString *)path isDirectory:(BOOL *)isdirptr;
+(NSString *)uniqueDirectoryPathWithParentDirectory:(NSString *)parent;
+(NSString *)sanitizedPathComponent:(NSString *)component;
+(NSArray *)contentsOfDirectoryAtPath:(NSString *)path;
+(BOOL)moveItemAtPath:(NSString *)src toPath:(NSString *)dest;
+(BOOL)removeItemAtPath:(NSString *)path;

// Resource forks
+(CSHandle *)handleForReadingResourceForkAtPath:(NSString *)path;

// Time functions.
+(double)currentTimeInSeconds;

@end
