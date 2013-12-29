#import "XADArchiveParser.h"

@interface XADArchiveParser (Descriptions)

-(NSString *)descriptionOfValueInDictionary:(NSDictionary *)dict key:(NSString *)key;
-(NSString *)descriptionOfKey:(NSString *)key;
-(NSArray *)descriptiveOrderingOfKeysInDictionary:(NSDictionary *)dict;

@end

NSString *XADHumanReadableFileSize(uint64_t size);
NSString *XADShortHumanReadableFileSize(uint64_t size);
NSString *XADHumanReadableBoolean(uint64_t boolean);
NSString *XADHumanReadablePOSIXPermissions(uint64_t permissions);
NSString *XADHumanReadableAmigaProtectionBits(uint64_t protection);
NSString *XADHumanReadableDOSFileAttributes(uint64_t attributes);
NSString *XADHumanReadableWindowsFileAttributes(uint64_t attributes);
NSString *XADHumanReadableOSType(uint64_t ostype);
NSString *XADHumanReadableEntryWithDictionary(NSDictionary *dict,XADArchiveParser *parser);

NSString *XADHumanReadableObject(id object);
NSString *XADHumanReadableDate(NSDate *date);
NSString *XADHumanReadableData(NSData *data);
NSString *XADHumanReadableArray(NSArray *array);
NSString *XADHumanReadableDictionary(NSDictionary *dict);
NSString *XADHumanReadableList(NSArray *labels,NSArray *values);
NSString *XADIndentTextWithSpaces(NSString *text,int spaces);
