#import "CSHandle.h"
#import "XADException.h"

@interface XADResourceFork:NSObject
{
	NSDictionary *resources;
}

+(XADResourceFork *)resourceForkWithHandle:(CSHandle *)handle;
+(XADResourceFork *)resourceForkWithHandle:(CSHandle *)handle error:(XADError *)errorptr;

-(id)init;
-(void)dealloc;

-(void)parseFromHandle:(CSHandle *)handle;
-(NSData *)resourceDataForType:(uint32_t)type identifier:(int)identifier;

-(NSMutableDictionary *)_parseResourceDataFromHandle:(CSHandle *)handle;
-(NSDictionary *)_parseMapFromHandle:(CSHandle *)handle withDataObjects:(NSMutableDictionary *)dataobjects;
-(NSDictionary *)_parseReferencesFromHandle:(CSHandle *)handle count:(int)count;

@end
