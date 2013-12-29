#import <Foundation/Foundation.h>

#import "XADArchiveParser.h"
#import "XADUnarchiver.h"
#import "XADRegex.h"

#define XADNeverCreateEnclosingDirectory 0
#define XADAlwaysCreateEnclosingDirectory 1
#define XADCreateEnclosingDirectoryWhenNeeded 2

@interface XADSimpleUnarchiver:NSObject
{
	XADArchiveParser *parser;
	XADUnarchiver *unarchiver,*subunarchiver;

	id delegate;
	BOOL shouldstop;

	NSString *destination,*enclosingdir;
	BOOL extractsubarchives,removesolo;
	BOOL overwrite,rename,skip;
	BOOL copydatetoenclosing,copydatetosolo,resetsolodate;
	BOOL propagatemetadata;

	NSMutableArray *regexes;
	NSMutableIndexSet *indices;

	NSMutableArray *entries,*reasonsforinterest;
	NSMutableDictionary *renames;
	NSMutableSet *resourceforks;
	id metadata;
	NSString *unpackdestination,*finaldestination,*overridesoloitem;
	int numextracted;

	NSString *toplevelname;
	BOOL lookslikesolo;

	off_t totalsize,currsize,totalprogress;
}

+(XADSimpleUnarchiver *)simpleUnarchiverForPath:(NSString *)path;
+(XADSimpleUnarchiver *)simpleUnarchiverForPath:(NSString *)path error:(XADError *)errorptr;

-(id)initWithArchiveParser:(XADArchiveParser *)archiveparser;
-(id)initWithArchiveParser:(XADArchiveParser *)archiveparser entries:(NSArray *)entryarray;
-(void)dealloc;

-(XADArchiveParser *)archiveParser;
-(XADArchiveParser *)outerArchiveParser;
-(XADArchiveParser *)innerArchiveParser;
-(NSArray *)reasonsForInterest;

-(id)delegate;
-(void)setDelegate:(id)newdelegate;

// TODO: Encoding wrappers?

-(NSString *)password;
-(void)setPassword:(NSString *)password;

-(NSString *)destination;
-(void)setDestination:(NSString *)destpath;

-(NSString *)enclosingDirectoryName;
-(void)setEnclosingDirectoryName:(NSString *)dirname;

-(BOOL)removesEnclosingDirectoryForSoloItems;
-(void)setRemovesEnclosingDirectoryForSoloItems:(BOOL)removeflag;

-(BOOL)alwaysOverwritesFiles;
-(void)setAlwaysOverwritesFiles:(BOOL)overwriteflag;

-(BOOL)alwaysRenamesFiles;
-(void)setAlwaysRenamesFiles:(BOOL)renameflag;

-(BOOL)alwaysSkipsFiles;
-(void)setAlwaysSkipsFiles:(BOOL)skipflag;

-(BOOL)extractsSubArchives;
-(void)setExtractsSubArchives:(BOOL)extractflag;

-(BOOL)copiesArchiveModificationTimeToEnclosingDirectory;
-(void)setCopiesArchiveModificationTimeToEnclosingDirectory:(BOOL)copyflag;

-(BOOL)copiesArchiveModificationTimeToSoloItems;
-(void)setCopiesArchiveModificationTimeToSoloItems:(BOOL)copyflag;

-(BOOL)resetsDateForSoloItems;
-(void)setResetsDateForSoloItems:(BOOL)resetflag;

-(BOOL)propagatesRelevantMetadata;
-(void)setPropagatesRelevantMetadata:(BOOL)propagateflag;

-(int)macResourceForkStyle;
-(void)setMacResourceForkStyle:(int)style;

-(BOOL)preservesPermissions;
-(void)setPreserevesPermissions:(BOOL)preserveflag;

-(double)updateInterval;
-(void)setUpdateInterval:(double)interval;

-(void)addGlobFilter:(NSString *)wildcard;
-(void)addRegexFilter:(XADRegex *)regex;
-(void)addIndexFilter:(int)index;
-(void)setIndices:(NSIndexSet *)indices;

-(off_t)predictedTotalSize;
-(off_t)predictedTotalSizeIgnoringUnknownFiles:(BOOL)ignoreunknown;

-(int)numberOfItemsExtracted;
-(BOOL)wasSoloItem;
-(NSString *)actualDestination;
-(NSString *)soloItem;
-(NSString *)createdItem;
-(NSString *)createdItemOrActualDestination;



-(XADError)parse;
-(XADError)_setupSubArchiveForEntryWithDataFork:(NSDictionary *)datadict resourceFork:(NSDictionary *)resourcedict;

-(XADError)unarchive;
-(XADError)_unarchiveRegularArchive;
-(XADError)_unarchiveSubArchive;

-(XADError)_finalizeExtraction;

-(void)_testForSoloItems:(NSDictionary *)entry;

-(BOOL)_shouldStop;

-(NSString *)_checkPath:(NSString *)path forEntryWithDictionary:(NSDictionary *)dict deferred:(BOOL)deferred;
-(BOOL)_recursivelyMoveItemAtPath:(NSString *)src toPath:(NSString *)dest overwrite:(BOOL)overwritethislevel;

+(NSString *)_findUniquePathForOriginalPath:(NSString *)path;
+(NSString *)_findUniquePathForOriginalPath:(NSString *)path reservedPaths:(NSSet *)reserved;

@end



@interface NSObject (XADSimpleUnarchiverDelegate)

-(void)simpleUnarchiverNeedsPassword:(XADSimpleUnarchiver *)unarchiver;

-(NSString *)simpleUnarchiver:(XADSimpleUnarchiver *)unarchiver encodingNameForXADString:(id <XADString>)string;

-(BOOL)simpleUnarchiver:(XADSimpleUnarchiver *)unarchiver shouldExtractEntryWithDictionary:(NSDictionary *)dict to:(NSString *)path;
-(void)simpleUnarchiver:(XADSimpleUnarchiver *)unarchiver willExtractEntryWithDictionary:(NSDictionary *)dict to:(NSString *)path;
-(void)simpleUnarchiver:(XADSimpleUnarchiver *)unarchiver didExtractEntryWithDictionary:(NSDictionary *)dict to:(NSString *)path error:(XADError)error;

-(NSString *)simpleUnarchiver:(XADSimpleUnarchiver *)unarchiver replacementPathForEntryWithDictionary:(NSDictionary *)dict
originalPath:(NSString *)path suggestedPath:(NSString *)unique;
-(NSString *)simpleUnarchiver:(XADSimpleUnarchiver *)unarchiver deferredReplacementPathForOriginalPath:(NSString *)path
suggestedPath:(NSString *)unique;

-(BOOL)extractionShouldStopForSimpleUnarchiver:(XADSimpleUnarchiver *)unarchiver;

-(void)simpleUnarchiver:(XADSimpleUnarchiver *)unarchiver
extractionProgressForEntryWithDictionary:(NSDictionary *)dict
fileProgress:(off_t)fileprogress of:(off_t)filesize
totalProgress:(off_t)totalprogress of:(off_t)totalsize;
-(void)simpleUnarchiver:(XADSimpleUnarchiver *)unarchiver
estimatedExtractionProgressForEntryWithDictionary:(NSDictionary *)dict
fileProgress:(double)fileprogress totalProgress:(double)totalprogress;

@end
