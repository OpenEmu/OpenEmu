/*
 Copyright (c) 2012, OpenEmu Team
 
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

#import "WiimoteBrowser.h"

@implementation WiimoteBrowser
- (id)init
{
	self = [super init];
	if (self != nil)
    {
		_maxWiimotes = -1;
	}
	return self;
}

- (int)maxWiimoteCount
{
	return _maxWiimotes;
}

- (void)setMaxWiimoteCount:(int)newMax
{
	_maxWiimotes = newMax;
}

@synthesize delegate;

- (void)startSearch
{
	if(![self delegate]) // check if delegate is set
    {
		DLog(@"Error: Can't run WiimoteBrowser without delegate!");
		return;
	}

	if(_isSearching) // already searching, just igone startSearch ;)
		return;
	
	_isSearching = YES;
	if([[self delegate] respondsToSelector:@selector(wiimoteBrowserWillSearch)])
		[[self delegate] performSelector:@selector(wiimoteBrowserWillSearch)];
	
	_inquiry = [IOBluetoothDeviceInquiry inquiryWithDelegate:self];
	[_inquiry setSearchCriteria:kBluetoothServiceClassMajorAny majorDeviceClass:0x05 minorDeviceClass:0x01];
	[_inquiry setInquiryLength:20];
	[_inquiry setUpdateNewDeviceNames:NO];

	IOReturn status = [_inquiry start];
	if (status != kIOReturnSuccess)
    {
        DLog(@"Error: Inquiry did not start, error %d", status);
		[_inquiry setDelegate:nil];
		_inquiry = nil;
		
		_isSearching = FALSE;
		
		if([[self delegate] respondsToSelector:@selector(wiimoteBrowserSearchFailedWithError:)])
			[[self delegate] performSelector:@selector(wiimoteBrowserSearchFailedWithError:)];
	}		
}

- (void)stopSearch
{}

- (NSArray*)discoveredDevices
{
	return _discoveredDevices;
}
#pragma mark -
#pragma mark privat methods
- (NSArray*)_convertFoundDevicesToWiimotes:(NSArray*)foundDevices
{
	NSMutableArray* wiimotes = [[NSMutableArray alloc] init];
	
	Wiimote* wiimo = nil;
	for(IOBluetoothDevice* btDev in foundDevices)
    {
		wiimo = [[Wiimote alloc] initWithDevice:btDev];		
		[wiimotes addObject:wiimo];
	}
	
	return wiimotes;
}

#pragma mark -
#pragma mark BT Inquiry	Delegates
- (void)deviceInquiryDeviceFound:(IOBluetoothDeviceInquiry *)sender device:(IOBluetoothDevice *)device
{
	// note: never try to connect to the wiimote while the inquiry is still running! (cf apple docs)
	if([[sender foundDevices] count]==_maxWiimotes)
		[_inquiry stop];
}

- (void) deviceInquiryComplete:(IOBluetoothDeviceInquiry*)sender error:(IOReturn)error aborted:(BOOL)aborted
{
	DLog(@"inquiry stopped, was aborted: %d", aborted);
	DLog(@"We've found: %lu devices, the maxiumum was set to %d", [[sender foundDevices] count], _maxWiimotes);
	_isSearching = FALSE;

	NSArray* results = [self _convertFoundDevicesToWiimotes:[sender foundDevices]];

	_discoveredDevices = results;
	
	if([[self delegate] respondsToSelector:@selector(wiimoteBrowserDidStopSearchWithResults:)])
		[[self delegate] performSelector:@selector(wiimoteBrowserDidStopSearchWithResults:) withObject:results];
	
}
@end