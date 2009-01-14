#import <Cocoa/Cocoa.h>
#import "xadmaster.h"

#define XADResourceForkData @"XADResourceForkData"
#define XADFinderFlags @"XADFinderFlags"

typedef enum {
	XADAbort,XADRetry,XADSkip,XADOverwrite,XADRename,
} XADAction;

typedef xadERROR XADError;

@class XADArchivePipe,UniversalDetector;

@interface XADArchive:NSObject
{
	NSString *filename;
	NSArray *volumes;
	NSData *memdata;
	XADArchive *parentarchive;
	XADArchivePipe *pipe;

	id delegate;
	NSStringEncoding name_encoding;
	NSString *password;
	NSTimeInterval update_interval;
	double update_time;

	struct xadMasterBase *xmb;
	struct xadArchiveInfo *archive;
	struct Hook progresshook;

	NSMutableArray *fileinfos;
	NSMutableDictionary *dittoforks;
	NSMutableArray *writeperms;

	int currentry;
	xadSize extractsize,totalsize;
	NSString *immediatedestination;
	BOOL immediatefailed;

	UniversalDetector *detector;
	NSStringEncoding detected_encoding;
	float detector_confidence;

	XADError lasterror;
}

-(id)init;
-(id)initWithFile:(NSString *)file;
-(id)initWithFile:(NSString *)file error:(XADError *)error;
-(id)initWithFile:(NSString *)file delegate:(id)del error:(XADError *)error;
-(id)initWithData:(NSData *)data;
-(id)initWithData:(NSData *)data error:(XADError *)error;
-(id)initWithArchive:(XADArchive *)archive entry:(int)n;
-(id)initWithArchive:(XADArchive *)archive entry:(int)n error:(XADError *)error;
-(id)initWithArchive:(XADArchive *)otherarchive entry:(int)n
     immediateExtractionTo:(NSString *)destination error:(XADError *)error;
-(void)dealloc;

-(BOOL)_finishInit:(xadTAGPTR)tags error:(XADError *)error;
-(xadUINT32)_newEntryCallback:(struct xadProgressInfo *)info;

-(NSString *)filename;
-(NSArray *)allFilenames;
-(NSString *)formatName;
-(BOOL)isEncrypted;
-(BOOL)isCorrupted;
-(BOOL)immediateExtractionFailed;

-(int)numberOfEntries;
-(NSString *)nameOfEntry:(int)n;
-(BOOL)entryHasSize:(int)n;
-(int)sizeOfEntry:(int)n;
-(BOOL)entryIsDirectory:(int)n;
-(BOOL)entryIsLink:(int)n;
-(BOOL)entryIsEncrypted:(int)n;
-(BOOL)entryIsArchive:(int)n;
-(NSDictionary *)attributesOfEntry:(int)n;
-(NSDictionary *)attributesOfEntry:(int)n withResourceFork:(BOOL)resfork;
-(NSData *)contentsOfEntry:(int)n;
-(NSData *)_contentsOfFileInfo:(struct xadFileInfo *)info;
-(BOOL)_entryIsLonelyResourceFork:(int)n;
-(int)_entryIndexOfName:(NSString *)name;
-(int)_entryIndexOfFileInfo:(struct xadFileInfo *)info;
-(const char *)_undecodedNameOfEntry:(int)n;

-(BOOL)extractTo:(NSString *)destination;
-(BOOL)extractTo:(NSString *)destination subArchives:(BOOL)sub;
-(BOOL)extractEntries:(NSIndexSet *)entries to:(NSString *)destination;
-(BOOL)extractEntries:(NSIndexSet *)entries to:(NSString *)destination subArchives:(BOOL)sub;
-(BOOL)extractEntry:(int)n to:(NSString *)destination;
-(BOOL)extractEntry:(int)n to:(NSString *)destination overrideWritePermissions:(BOOL)override;
-(BOOL)extractArchiveEntry:(int)n to:(NSString *)destination;
-(void)fixWritePermissions;

-(BOOL)_extractEntry:(int)n as:(NSString *)destfile;
-(BOOL)_extractFileEntry:(int)n as:(NSString *)destfile;
-(BOOL)_extractDirectoryEntry:(int)n as:(NSString *)destfile;
-(BOOL)_extractLinkEntry:(int)n as:(NSString *)destfile;
-(xadERROR)_extractFileInfo:(struct xadFileInfo *)info tags:(xadTAGPTR)tags reportProgress:(BOOL)report;
-(BOOL)_ensureDirectoryExists:(NSString *)directory;
-(BOOL)_changeAllAttributes:(NSDictionary *)attrs atPath:(NSString *)path overrideWritePermissions:(BOOL)override;

-(NSString *)commonTopDirectory;

-(void)setDelegate:(id)delegate;
-(id)delegate;

-(NSStringEncoding)nameEncoding;
-(void)setNameEncoding:(NSStringEncoding)encoding;
-(NSStringEncoding)encodingForString:(const char *)cstr;
-(BOOL)_stringIsASCII:(const char *)cstr;
-(void)_runDetectorOn:(const char *)cstr;

-(NSString *)password;
-(void)setPassword:(NSString *)newpassword;
-(const char *)_encodedPassword;

-(void)setProgressInterval:(NSTimeInterval)interval;
-(xadUINT32)_progressCallback:(struct xadProgressInfo *)info;

-(BOOL)_canHaveDittoResourceForks;
-(BOOL)_fileInfoIsDittoResourceFork:(struct xadFileInfo *)info;
-(NSString *)_nameOfDataForkForDittoResourceFork:(struct xadFileInfo *)info;
-(void)_parseDittoResourceFork:(struct xadFileInfo *)info intoAttributes:(NSMutableDictionary *)attrs;

-(XADError)lastError;
-(void)clearLastError;
-(NSString *)describeLastError;
-(NSString *)describeError:(XADError)error;

-(struct xadMasterBase *)xadMasterBase;
-(struct xadArchiveInfo *)xadArchiveInfo;
-(struct xadFileInfo *)xadFileInfoForEntry:(int)n;

-(NSString *)description;

+(XADArchive *)archiveForFile:(NSString *)filename;
+(XADArchive *)recursiveArchiveForFile:(NSString *)filename;

+(NSArray *)volumesForFile:(NSString *)filename;

@end



@interface NSObject (XADArchiveDelegate)

-(NSStringEncoding)archive:(XADArchive *)archive encodingForName:(const char *)bytes guess:(NSStringEncoding)guess confidence:(float)confidence;
-(XADAction)archive:(XADArchive *)archive nameDecodingDidFailForEntry:(int)n bytes:(const char *)bytes;

-(BOOL)archiveExtractionShouldStop:(XADArchive *)archive;
-(BOOL)archive:(XADArchive *)archive shouldCreateDirectory:(NSString *)directory;
-(XADAction)archive:(XADArchive *)archive entry:(int)n collidesWithFile:(NSString *)file newFilename:(NSString **)newname;
-(XADAction)archive:(XADArchive *)archive entry:(int)n collidesWithDirectory:(NSString *)file newFilename:(NSString **)newname;
-(XADAction)archive:(XADArchive *)archive creatingDirectoryDidFailForEntry:(int)n;

-(void)archive:(XADArchive *)archive extractionOfEntryWillStart:(int)n;
-(void)archive:(XADArchive *)archive extractionProgressForEntry:(int)n bytes:(xadSize)bytes of:(xadSize)total;
-(void)archive:(XADArchive *)archive extractionOfEntryDidSucceed:(int)n;
-(XADAction)archive:(XADArchive *)archive extractionOfEntryDidFail:(int)n error:(XADError)error;
-(XADAction)archive:(XADArchive *)archive extractionOfResourceForkForEntryDidFail:(int)n error:(XADError)error;

-(void)archive:(XADArchive *)archive extractionProgressBytes:(xadSize)bytes of:(xadSize)total;
-(void)archive:(XADArchive *)archive extractionProgressFiles:(int)files of:(int)total;

@end
