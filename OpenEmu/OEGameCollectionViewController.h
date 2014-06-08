//
//  OEGameCollectionViewController.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 02/06/14.
//
//

#import "OECollectionViewController.h"

@interface OEGameCollectionViewController : OECollectionViewController
- (IBAction)showSelectedGamesInFinder:(id)sender;

- (void)deleteSaveState:(id)stateItem;
- (void)makeNewCollectionWithSelectedGames:(id)sender;
- (void)addSelectedGamesToCollection:(id)sender;
- (void)downloadCoverArt:(id)sender;
- (void)cancelCoverArtDownload:(id)sender;
- (void)addCoverArtFromFile:(id)sender;
- (void)addSaveStateFromFile:(id)sender;
- (void)consolidateFiles:(id)sender;
@end
