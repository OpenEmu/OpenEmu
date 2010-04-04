#import "CSHandle.h"
#import "CSSubHandle.h"

@interface CSHandle (Checksums)

-(BOOL)hasChecksum;
-(BOOL)isChecksumCorrect;

@end

@interface CSSubHandle (Checksums)

-(BOOL)hasChecksum;
-(BOOL)isChecksumCorrect;

@end
