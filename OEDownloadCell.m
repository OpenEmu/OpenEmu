// ProgressCell.m

#import "OEDownloadCell.h"
#import "OEDownload.h"

@implementation OEDownloadCell

- (void)drawWithFrame:(NSRect)cellFrame inView:(NSView *)controlView {
	OEDownload* data = [self objectValue];
	
	NSProgressIndicator * progressIndicator = data.progressBar;
	
	[controlView addSubview:progressIndicator];
	
	[progressIndicator setDoubleValue:data.progress];
	[progressIndicator setFrame:cellFrame];
}
@end