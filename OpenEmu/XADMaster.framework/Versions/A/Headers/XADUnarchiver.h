#import <Foundation/Foundation.h>

#import "XADArchiveParser.h"

#define XADIgnoredForkStyle 0
#define XADMacOSXForkStyle 1
#define XADHiddenAppleDoubleForkStyle 2
#define XADVisibleAppleDoubleForkStyle 3
#define XADHFVExplorerAppleDoubleForkStyle 4

#ifdef __APPLE__
#define XADDefaultForkStyle XADMacOSXForkStyle
#else
#define XADDefaultForkStyle XADVisibleAppleDoubleForkStyle
#endif

@interface XADUnarchiver:NSObject
{
	XADArchiveParser *parser;
	NSString *destination;
	int forkstyle;
	BOOL preservepermissions;
	double updateinterval;

	id delegate;
	BOOL shouldstop;

	NSMutableArray *deferreddirectories,*deferredlinks;
}

+(XADUnarchiver *)unarchiverForArchiveParser:(XADArchiveParser *)archiveparser;
+(XADUnarchiver *)unarchiverForPath:(NSString *)path;
+(XADUnarchiver *)unarchiverForPath:(NSString *)path error:(XADError *)errorptr;

-(id)initWithArchiveParser:(XADArchiveParser *)archiveparser;
-(void)dealloc;

-(XADArchiveParser *)archiveParser;

-(id)delegate;
-(void)setDelegate:(id)newdelegate;

-(NSString *)destination;
-(void)setDestination:(NSString *)destpath;

-(int)macResourceForkStyle;
-(void)setMacResourceForkStyle:(int)style;

-(BOOL)preservesPermissions;
-(void)setPreserevesPermissions:(BOOL)preserveflag;

-(double)updateInterval;
-(void)setUpdateInterval:(double)interval;

-(XADError)parseAndUnarchive;

-(XADError)extractEntryWithDictionary:(NSDictionary *)dict;
-(XADError)extractEntryWithDictionary:(NSDictionary *)dict forceDirectories:(BOOL)force;
-(XADError)extractEntryWithDictionary:(NSDictionary *)dict as:(NSString *)path;
-(XADError)extractEntryWithDictionary:(NSDictionary *)dict as:(NSString *)path forceDirectories:(BOOL)force;

-(XADError)finishExtractions;
-(XADError)_fixDeferredLinks;
-(XADError)_fixDeferredDirectories;

-(XADUnarchiver *)unarchiverForEntryWithDictionary:(NSDictionary *)dict
wantChecksum:(BOOL)checksum error:(XADError *)errorptr;
-(XADUnarchiver *)unarchiverForEntryWithDictionary:(NSDictionary *)dict
resourceForkDictionary:(NSDictionary *)forkdict wantChecksum:(BOOL)checksum error:(XADError *)errorptr;

-(XADError)_extractFileEntryWithDictionary:(NSDictionary *)dict as:(NSString *)destpath;
-(XADError)_extractDirectoryEntryWithDictionary:(NSDictionary *)dict as:(NSString *)destpath;
-(XADError)_extractLinkEntryWithDictionary:(NSDictionary *)dict as:(NSString *)destpath;
-(XADError)_extractArchiveEntryWithDictionary:(NSDictionary *)dict to:(NSString *)destpath name:(NSString *)filename;
-(XADError)_extractResourceForkEntryWithDictionary:(NSDictionary *)dict asAppleDoubleFile:(NSString *)destpath;

-(XADError)_updateFileAttributesAtPath:(NSString *)path forEntryWithDictionary:(NSDictionary *)dict
deferDirectories:(BOOL)defer;
-(XADError)_ensureDirectoryExists:(NSString *)path;

-(XADError)runExtractorWithDictionary:(NSDictionary *)dict outputHandle:(CSHandle *)handle;
-(XADError)runExtractorWithDictionary:(NSDictionary *)dict
outputTarget:(id)target selector:(SEL)sel argument:(id)arg;

-(NSString *)adjustPathString:(NSString *)path forEntryWithDictionary:(NSDictionary *)dict;

-(BOOL)_shouldStop;

@end




@interface NSObject (XADUnarchiverDelegate)

-(void)unarchiverNeedsPassword:(XADUnarchiver *)unarchiver;

-(BOOL)unarchiver:(XADUnarchiver *)unarchiver shouldExtractEntryWithDictionary:(NSDictionary *)dict suggestedPath:(NSString **)pathptr;
-(void)unarchiver:(XADUnarchiver *)unarchiver willExtractEntryWithDictionary:(NSDictionary *)dict to:(NSString *)path;
-(void)unarchiver:(XADUnarchiver *)unarchiver didExtractEntryWithDictionary:(NSDictionary *)dict to:(NSString *)path error:(XADError)error;

-(BOOL)unarchiver:(XADUnarchiver *)unarchiver shouldCreateDirectory:(NSString *)directory;
-(BOOL)unarchiver:(XADUnarchiver *)unarchiver shouldDeleteFileAndCreateDirectory:(NSString *)directory;

-(BOOL)unarchiver:(XADUnarchiver *)unarchiver shouldExtractArchiveEntryWithDictionary:(NSDictionary *)dict to:(NSString *)path;
-(void)unarchiver:(XADUnarchiver *)unarchiver willExtractArchiveEntryWithDictionary:(NSDictionary *)dict withUnarchiver:(XADUnarchiver *)subunarchiver to:(NSString *)path;
-(void)unarchiver:(XADUnarchiver *)unarchiver didExtractArchiveEntryWithDictionary:(NSDictionary *)dict withUnarchiver:(XADUnarchiver *)subunarchiver to:(NSString *)path error:(XADError)error;

-(NSString *)unarchiver:(XADUnarchiver *)unarchiver destinationForLink:(XADString *)link from:(NSString *)path;

-(BOOL)extractionShouldStopForUnarchiver:(XADUnarchiver *)unarchiver;
-(void)unarchiver:(XADUnarchiver *)unarchiver extractionProgressForEntryWithDictionary:(NSDictionary *)dict
fileFraction:(double)fileprogress estimatedTotalFraction:(double)totalprogress;

-(void)unarchiver:(XADUnarchiver *)unarchiver findsFileInterestingForReason:(NSString *)reason;

@end

@interface NSObject (XADUnarchiverDelegateDeprecated)
// Deprecated.
-(NSString *)unarchiver:(XADUnarchiver *)unarchiver pathForExtractingEntryWithDictionary:(NSDictionary *)dict;
-(BOOL)unarchiver:(XADUnarchiver *)unarchiver shouldExtractEntryWithDictionary:(NSDictionary *)dict to:(NSString *)path;
-(NSString *)unarchiver:(XADUnarchiver *)unarchiver linkDestinationForEntryWithDictionary:(NSDictionary *)dict from:(NSString *)path;
@end
