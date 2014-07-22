//
//  OEGridGameCellInteractoin.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 22/07/14.
//
//

#import "OEGridView.h"
@protocol OEGameGridViewDelegate <OEGridViewDelegate>
- (void)gridView:(OEGridView*)gridView requestsDownloadRomForItemAtIndex:(NSUInteger)index;
@end
