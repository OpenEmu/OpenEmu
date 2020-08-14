/*
 Copyright (c) 2015, OpenEmu Team
 
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

#import "OEDBAllGamesCollection.h"

NS_ASSUME_NONNULL_BEGIN

@implementation OEDBAllGamesCollection

+ (instancetype)sharedDBAllGamesCollection
{
    static OEDBAllGamesCollection *sharedInstance;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedInstance = [OEDBAllGamesCollection new];
    });
    return sharedInstance;
}

- (nullable NSImage *)sidebarIcon
{
    return [NSImage imageNamed:@"collection_smart"];
}

- (NSString *)sidebarName
{
    return NSLocalizedString(@"All Games", @"");
}

- (void)setSidebarName:(NSString *)newName
{
    NSLog(@"OEDBAllGamesCollection: can not change name!");
}

- (nullable NSString*)viewControllerClassName
{
    return @"OEGameCollectionViewController";
}

- (nullable NSString*)sidebarID
{
    return @"OEDBAllGamesCollection";
}

- (BOOL)isSelectableInSidebar
{
    return YES;
}

- (BOOL)isEditableInSidebar
{
    return NO;
}

- (BOOL)isGroupHeaderInSidebar
{
    return NO;
}

- (BOOL)hasSubCollections
{
    return NO;
}

@end

NS_ASSUME_NONNULL_END
