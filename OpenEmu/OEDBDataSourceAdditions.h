/*
 Copyright (c) 2011, OpenEmu Team
 
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

#import <Cocoa/Cocoa.h>

#import "OEDBGame.h"
#import "OEDBSystem.h"
#import "OEDBCollection.h"
#import "OEDBSmartCollection.h"
#import "OEDBCollectionFolder.h"

#import <Quartz/Quartz.h> // for ImageKit

#import "OECoverGridDataSourceItem.h"
#import "OECoverFlowDataSourceItem.h"
#import "OESidebarItem.h"
#import "OEListViewDataSourceItem.h"
#import "OEDBAllGamesCollection.h"

#import "OECollectionViewItemProtocol.h"

extern NSString * const OECoverGridViewAutoDownloadEnabledKey;
@interface OEDBGame (DataSourceAdditions) <OECoverGridDataSourceItem, OECoverFlowDataSourceItem, OEListViewDataSourceItem>
@end

@interface OEDBSystem (DataSourceAdditions) <OESidebarItem> 
@end


@interface OEDBCollection (DataSourceAdditions) <OESidebarItem> 
@end

@interface OEDBSmartCollection (DataSourceAdditions) <OESidebarItem> 
@end

@interface OEDBCollectionFolder (DataSourceAdditions) <OESidebarItem> 
@end

#pragma mark -
#pragma mark Implementation of items that can be presented by CollectionView

@interface OEDBSystem (OECollectionViewItemAdditions) <OEGameCollectionViewItemProtocol>
@end

@interface OEDBCollection (OECollectionViewItemAdditions) <OEGameCollectionViewItemProtocol>
@end

@interface OEDBCollectionFolder (OECollectionViewItemAdditions) <OEGameCollectionViewItemProtocol>
@end

@interface OEDBSmartCollection (OECollectionViewItemAdditions) <OEGameCollectionViewItemProtocol>
@end

@interface OEDBAllGamesCollection (OECollectionViewItemAdditions) <OEGameCollectionViewItemProtocol>
@end
