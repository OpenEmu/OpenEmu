/*
 Copyright (c) 2013, OpenEmu Team

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the OpenEmu Team nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#import "OERetrode.h"
#import "OERetrode_IOLevel.h"
#import "OERetrode_Configuration.h"

#import "NSString+OERangeAdditions.h"
#import "NSFileManager+OEHashingAdditions.h"

#import "ArchiveVGThrottling.h"

const int64_t kOEDisconnectDelay   = 1.0;
const int32_t kOERetrodeVendorID  = 0x0403;
const int32_t kOERetrodeProductIDRevision1 = 0x97c0;
const int32_t kOERetrodeProductIDRevision2 = 0x97c1;

const int32_t kOEVendorIDVersion2DFU  = 0x03eb;
const int32_t kOEProductIDVersion2DFU = 0x2ff9;

#define IfNotConnectedReturn(__RETURN_VALUE__) if([self deviceData] == NULL) return __RETURN_VALUE__;

#pragma mark Retrode Specific Constants
#define kOEConfigurationEncoding NSASCIIStringEncoding
#define kOEConfigurationLineSeparator @"\r\n"
#define kOEConfigurationFileName @"RETRODE.CFG"

#pragma mark Error Constants
NSString * const kOEDiskNotMounted = @"Not Mounted";
NSString * const kOENoBSDDevice    = @"No BSD device";

@interface OERetrode ()
{
    DASessionRef daSession;
    DAApprovalSessionRef apSession;
}

@property (readwrite, strong) NSString *identifier;
@property id delegate;

void OEDADiskUnmountCallback(DADiskRef disk, DADissenterRef dissenter, void *self);
void OEDADiskMountCallback(DADiskRef disk, DADissenterRef dissenter, void *self);
void OEDADiskAppearedCallback(DADiskRef disk, void *self);
void OEDADiskDisappearedCallback(DADiskRef disk, void *self);
DADissenterRef OEDADiskEjectApprovalCallback(DADiskRef disk, void *self);
DADissenterRef OEDADiskUnmountApprovalCallback(DADiskRef disk,void *context);
@end

@implementation OERetrode

- (id)init
{
    self = [super init];
    if (self) {
        [self OE_setupDASessions];
    }
    return self;
}

- (NSString*)description
{
    unsigned long address = (unsigned long)self;
    UInt32 locationID = [self locationID];
    NSString *bsdName = [self bsdDeviceName];
    NSString *firmwareVersion = @"unknown"; // [self firmwareVersion];
    NSString *mountLocation = (NSString*)[self mountPath];
    
    return [NSString stringWithFormat:@"<Retrode: 0x%lx> Firmware: <%@> Location ID: <0x%0x> DADisk available: <%s>, BSD name: <%@>, mounted: <%@>", address, firmwareVersion, locationID, BOOL_STR([self diskDescription]!=nil), bsdName, mountLocation];
}

#pragma mark - OEStorageDevice Overrides -
- (NSString*)name
{
    return @"Retrode";
}

- (NSImage*)icon
{
    return [NSImage imageNamed:@"retrode"];
}

- (BOOL)isReady
{
    return NO;
}
#pragma mark - OESidebarItem -
- (NSString*)sidebarID
{
    return @"";
}

- (NSString*)viewControllerClassName
{
    return @"OERetrodeViewController";
}

#pragma mark -
- (void)rescanGames
{
    if(![self isMounted]) return;

    NSString     *srmExtension = [[self configuration] objectForKey:@"sramExt"];
    NSArray      *allFiles   = [[NSFileManager defaultManager] contentsOfDirectoryAtPath:[self mountPath] error:nil];
    NSPredicate  *fileFilter = [NSPredicate predicateWithBlock:^BOOL(id fileName, NSDictionary *bindings) {
        NSString *extension  = [[fileName pathExtension] lowercaseString];
        return [extension isNotEqualTo:@"cfg"] && [extension isNotEqualTo:srmExtension];
    }];
    
    NSArray    *gameNames = [allFiles filteredArrayUsingPredicate:fileFilter];
    NSMutableArray *games = [NSMutableArray arrayWithCapacity:[gameNames count]];
    [gameNames enumerateObjectsUsingBlock:^(NSString *fileName, NSUInteger idx, BOOL *stop) {
        [games addObject:[self OE_gameForFile:fileName]];
    }];
    
    [self setGames:games];
    [self OE_callDelegateMethod:@selector(retrodeDidRescanGames:)];
}

- (NSMutableDictionary*)OE_gameForFile:(NSString*)fileName
{
    NSMutableDictionary *dictionary = [NSMutableDictionary dictionaryWithCapacity:2];
    NSString            *filePath   = [[self mountPath] stringByAppendingPathComponent:fileName];

    
    // Make file name a little nicer for display (eg. TheLegendOfZelda.AF49.sfc -> The Legend Of Zelda)
    NSMutableString *displayName      = [[[fileName componentsSeparatedByString:@"."] objectAtIndex:0] mutableCopy];
    NSCharacterSet  *uppercaseLetters = [NSCharacterSet uppercaseLetterCharacterSet];
    NSArray         *words            = [displayName componentsSeparatedByCharactersInSet:uppercaseLetters];
    NSIndexSet      *indexes          = [NSIndexSet indexSetWithIndexesInRange:NSMakeRange(1, [words count]-1)];
    
    __block NSUInteger offset = [[words objectAtIndex:0] length]+1;

    [words enumerateObjectsAtIndexes:indexes options:0 usingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        offset += [obj length];
        [displayName insertString:@" " atIndex:offset];
        offset += 2;
    }];
    
    [dictionary setObject:displayName forKey:@"gameTitle"];
    [dictionary setObject:filePath forKey:@"path"];
    
    return dictionary;
}
#pragma mark - Configuration -
- (NSString*)configurationFilePath
{
    NSString *mountPath = [self mountPath];
    if(mountPath == kOEDiskNotMounted || mountPath == kOENoBSDDevice)
        return mountPath;
    return [mountPath stringByAppendingPathComponent:kOEConfigurationFileName];
}

- (void)readConfigurationFromDisk
{
    NSError   *error                 = nil;
    NSString  *configurationFilePath = [self configurationFilePath];
    NSData    *configFileData        = [NSData dataWithContentsOfFile:configurationFilePath options:NSDataReadingUncached error:&error];
    if(configFileData == nil)
    {
        DLog(@"%@", error);
        return;
    }
    
    NSString  *configFile = [[NSString alloc] initWithData:configFileData encoding:kOEConfigurationEncoding];
    NSArray   *lines      = [configFile componentsSeparatedByString:kOEConfigurationLineSeparator];
    
    // Read firmware version from first line
    NSString * firmwareLine         = [lines objectAtIndex:0];
    NSString * const versionPattern = @"(?<=Retrode\\s)\\d*\\.\\d+\\w*(\\s\\w+)?";
    NSRegularExpression *expression = [NSRegularExpression regularExpressionWithPattern:versionPattern options:0 error:&error];
    NSArray *matches = [expression matchesInString:firmwareLine options:0 range:[firmwareLine fullRange]];
    NSString *firmwareVersion;
    if([matches count] != 1)
    {
        firmwareVersion = @"0.17b";
    }
    else
    {
        NSTextCheckingResult *versionMatch = [matches lastObject];
        firmwareVersion = [firmwareLine substringWithRange:[versionMatch range]];
    }
    // Convert early firmware version that start with .xx to 0.xx
    if([firmwareVersion characterAtIndex:0] == '.')
        firmwareVersion = [NSString stringWithFormat:@"0%@", firmwareVersion];
    [self setFirmwareVersion:firmwareVersion];
    
    NSMutableArray      *configEnries = [NSMutableArray arrayWithCapacity:[lines count]];
    NSMutableDictionary *configValues = [NSMutableDictionary dictionaryWithCapacity:[lines count]];
    
    // Read configuration line by line
    NSString * const keyPattern   = @"(?<=\\[)\\w+(?=]\\s+)";
    NSString * const valuePattern = @"[^\\s;]+";
    
    for(NSString *line in lines)
    {
        NSRegularExpression  *keyExpression = [NSRegularExpression regularExpressionWithPattern:keyPattern options:0 error:nil];
        NSTextCheckingResult *keyMatch      = [keyExpression firstMatchInString:line options:0 range:[line fullRange]];
        if(keyMatch)
        {
            // Calculate range of line without key (and without ']' that comes after the key)
            NSRange keyRange   = [keyMatch range];
            NSRange valueRange = [line fullRange];
            valueRange.location += NSMaxRange(keyRange) + 1;
            valueRange.length   -= NSMaxRange(keyRange) + 1;
            
            NSRegularExpression  *valueExpression = [NSRegularExpression regularExpressionWithPattern:valuePattern options:0 error:nil];
            NSTextCheckingResult *valueMatch      = [valueExpression firstMatchInString:line options:0 range:valueRange];
            if(valueMatch)
            {
                NSString *key              = [line substringWithRange:[keyMatch range]];
                id       value             = [line substringWithRange:[valueMatch range]];
                NSString *linePlaceholder  = [line stringByReplacingCharactersInRange:[valueMatch range] withString:@"%@"];
                
                // Try to get an Integer value if that makes sense
                int intValue;
                NSScanner *scanner = [NSScanner scannerWithString:value];
                if([scanner scanInt:&intValue])
                    value = @(intValue);
                
                [configEnries addObject:@{ @"key":key, @"line":linePlaceholder }];
                [configValues setObject:value forKey:key];
            }
        }
        else
            [configEnries addObject:line];
    };
    
    [self setConfiguration:configValues];
    [self setConfigurationLineMapping:configEnries];
}

- (BOOL)writeConfigurationToDisk:(NSError**)outError;
{
    NSError         *error              = nil;
    NSDictionary    *configuration      = [self configuration];
    NSArray         *lineMapping        = [self configurationLineMapping];
    NSMutableArray  *configurationLines = [NSMutableArray arrayWithCapacity:[lineMapping count]];
    
    for(id mapping in lineMapping)
    {
        if([mapping isKindOfClass:[NSString class]])
        {
            [configurationLines addObject:mapping];
        }
        else if([mapping isKindOfClass:[NSDictionary class]])
        {
            NSString *mappingKey  = [mapping objectForKey:@"key"];
            NSString *mappingLine = [mapping objectForKey:@"line"];
            
            id value = [configuration objectForKey:mappingKey];
            NSString *line = [NSString stringWithFormat:mappingLine, value];
            [configurationLines addObject:line];
        }
    }
    
    NSString *filePath     = [self configurationFilePath];
    NSString *fileContents = [configurationLines componentsJoinedByString:kOEConfigurationLineSeparator];
    NSData   *fileData     = [fileContents dataUsingEncoding:kOEConfigurationEncoding];
    BOOL writeSuccess      = [fileData writeToFile:filePath options:0 error:&error];
    if(!writeSuccess)
    {
        DLog(@"Error writing config file");
        DLog(@"%@", error);
        return NO;
    }
    return YES;
}
#pragma mark - I/O Stuff -
+ (NSString*)generateIdentifierFromDeviceData:(OERetrodeDeviceData*)deviceData
{
    NSString *identifier = [NSString stringWithFormat:@"0x%x", deviceData->locationID];
    return identifier;
}

- (void)setupWithDeviceData:(OERetrodeDeviceData*)deviceData
{
    [self setDeviceData:deviceData];
    if(deviceData != NULL)
    {
        [self setLocationID:(deviceData->locationID)];
        [self setIdentifier:[[self class] generateIdentifierFromDeviceData:deviceData]];
        
        io_string_t path;
        IORegistryEntryGetPath(deviceData->ioService, kIOServicePlane, path);
        NSString *devicePath = [@(path) stringByAppendingString:@"/IOUSBInterface@0/IOUSBMassStorageClass/IOSCSIPeripheralDeviceNub/IOSCSIPeripheralDeviceType00/IOBlockStorageServices"];
        NSDictionary *dictionary = @{ @"DADevicePath" : devicePath };
        DARegisterDiskEjectApprovalCallback(apSession, (__bridge CFDictionaryRef)(dictionary), OEDADiskEjectApprovalCallback, (__bridge void *)(self));
        DARegisterDiskAppearedCallback(apSession, (__bridge CFDictionaryRef)(dictionary), OEDADiskAppearedCallback, (__bridge void *)(self));
        DARegisterDiskDisappearedCallback(apSession, (__bridge CFDictionaryRef)(dictionary), OEDADiskDisappearedCallback, (__bridge void *)(self));
    }
    
    if(deviceData != NULL)
    {
        [self OE_callDelegateMethod:@selector(retrodeHardwareDidBecomeAvailable:)];
        [self rescanGames];
    }
    else
        [self OE_callDelegateMethod:@selector(retrodeHardwareDidBecomeUnavailable:)];
}

- (NSString*)bsdDeviceName
{
    IfNotConnectedReturn(kOENoBSDDevice);
    
    NSString *bsdName = nil;
    CFTypeRef bsdNameData = IORegistryEntrySearchCFProperty([self deviceData]->ioService, kIOServicePlane, CFSTR("BSD Name"), kCFAllocatorDefault, kIORegistryIterateRecursively);
    if (bsdNameData == NULL)
        bsdName = kOENoBSDDevice;
    else
        bsdName = (__bridge_transfer NSString*)bsdNameData;
    return bsdName;
}

- (NSString*)mountPath
{
    NSDictionary *diskDescription = [self diskDescription];
    if(!diskDescription) return kOENoBSDDevice;
    return [[diskDescription objectForKey:(__bridge NSString*)kDADiskDescriptionVolumePathKey] path]?:kOEDiskNotMounted;
}

- (void)unmountFilesystem
{
    IfNotConnectedReturn();
    DLog();
    
    DADiskRef    disk_ref = DADiskCreateFromBSDName(kCFAllocatorDefault, daSession, [[self bsdDeviceName] UTF8String]);
    if(disk_ref != NULL)
    {
        DADiskUnmount(disk_ref, 0, OEDADiskUnmountCallback, (__bridge void *)(self));
        CFRelease(disk_ref);
    }
}

- (void)mountFilesystem
{
    IfNotConnectedReturn();
    DLog();
    DADiskRef    disk_ref = DADiskCreateFromBSDName(kCFAllocatorDefault, daSession, [[self bsdDeviceName] UTF8String]);
    if(disk_ref != NULL)
    {
        DADiskMount(disk_ref, NULL, 0, OEDADiskMountCallback, (__bridge void *)(self));
        CFRelease(disk_ref);
    }
}

- (void)setDFUMode:(BOOL)dfuMode
{
    if(_DFUMode == dfuMode) return;
    
    [self willChangeValueForKey:@"DFUMode"];
    _DFUMode = dfuMode;
    [self didChangeValueForKey:@"DFUMode"];
    
    if(_DFUMode)
        [self OE_callDelegateMethod:@selector(retrodeDidEnterDFUMode:)];
    else
        [self OE_callDelegateMethod:@selector(retrodeDidLeaveDFUMode:)];
}

- (void)disconnect
{
    [self setupWithDeviceData:NULL];
    [self OE_tearDownDASessions];
    
    [self OE_callDelegateMethod:@selector(retrodeDidDisconnect:)];
    
    [self setDelegate:nil];
}

#pragma mark I/O Level Helpers
- (NSDictionary*)diskDescription
{
    IfNotConnectedReturn(nil);
    
    NSDictionary *result  = nil;
    NSString *bsdDeviceName = [self bsdDeviceName];
    if(bsdDeviceName == kOENoBSDDevice || bsdDeviceName == kOEDiskNotMounted)
        return nil;
    
    DADiskRef    disk_ref = DADiskCreateFromBSDName(kCFAllocatorDefault, daSession, [bsdDeviceName UTF8String]);
    if(disk_ref != NULL)
    {
        result = (__bridge_transfer NSDictionary*)DADiskCopyDescription(disk_ref);
        CFRelease(disk_ref);
    }
    
    return result;
}

- (void)OE_setupDASessions
{
    if(daSession == NULL)
    {
        DLog("Set sessions up");
        daSession = DASessionCreate(kCFAllocatorDefault);
        DASessionScheduleWithRunLoop(daSession, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
        
        apSession = DAApprovalSessionCreate(kCFAllocatorDefault);
        DAApprovalSessionScheduleWithRunLoop(apSession, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    }
}

- (void)OE_tearDownDASessions
{
    if(daSession != NULL)
    {
        DLog("Tear sessions down");
        DASessionUnscheduleFromRunLoop(daSession, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
        CFRelease(daSession);
        daSession = NULL;
        
        DAApprovalSessionUnscheduleFromRunLoop(apSession, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
        CFRelease(apSession);
        apSession = NULL;
    }
}
#pragma mark - C-Callbacks
void OEDADiskUnmountCallback(DADiskRef disk, DADissenterRef dissenter, void *self)
{
    DLog();
}

void OEDADiskMountCallback(DADiskRef disk, DADissenterRef dissenter, void *self)
{
    DLog();
}

void OEDADiskAppearedCallback(DADiskRef disk, void *self)
{
    DLog();
    OERetrode *retrode = (__bridge OERetrode*)self;
    [retrode setIsMounted:YES];
    [retrode OE_callDelegateMethod:@selector(retrodeDidMount:)];
    [retrode readConfigurationFromDisk];
    [retrode rescanGames];
}

void OEDADiskDisappearedCallback(DADiskRef disk, void *self)
{
    DLog();
    OERetrode *retrode = (__bridge OERetrode*)self;
    [retrode setIsMounted:NO];
    [retrode OE_callDelegateMethod:@selector(retrodeDidUnmount:)];
}

DADissenterRef OEDADiskEjectApprovalCallback(DADiskRef disk, void *self)
{
    DLog();
    return NULL;
}

DADissenterRef OEDADiskUnmountApprovalCallback(DADiskRef disk,void *context)
{
    DLog();
    return NULL;
}
#pragma mark -
- (void)OE_callDelegateMethod:(SEL)sel
{
/*
    if(![[self delegate] respondsToSelector:sel])
        return;
*/    
    if(sel == @selector(retrodeDidRescanGames:))
        [[self delegate] retrodeDidRescanGames:self];
    
    else if(sel == @selector(retrodeHardwareDidBecomeAvailable:))
        [[self delegate] retrodeHardwareDidBecomeAvailable:self];
    else if(sel == @selector(retrodeHardwareDidBecomeUnavailable:))
        [[self delegate] retrodeHardwareDidBecomeUnavailable:self];
    
    else if(sel == @selector(retrodeDidMount:))
        [[self delegate] retrodeDidMount:self];
    else if(sel == @selector(retrodeDidUnmount:))
        [[self delegate] retrodeDidUnmount:self];
    
    else if(sel == @selector(retrodeDidEnterDFUMode:))
        [[self delegate] retrodeDidEnterDFUMode:self];
    else if(sel == @selector(retrodeDidLeaveDFUMode:))
        [[self delegate] retrodeDidLeaveDFUMode:self];
    
    else if(sel == @selector(retrodeDidDisconnect:))
        [[self delegate] retrodeDidDisconnect:self];
    else if(sel == @selector(retrodeDidRescanGames:))
        [[self delegate] retrodeDidRescanGames:self];
    else
        DLog(@"%@", NSStringFromSelector(sel));
}
@end
