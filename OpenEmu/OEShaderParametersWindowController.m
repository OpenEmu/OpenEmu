// Copyright (c) 2019, OpenEmu Team
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the OpenEmu Team nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#import "OEShaderParametersWindowController.h"
#import "OEGameViewController.h"
#import "OEGameDocument.h"
#import "OEShaderParameterValue.h"
#import "OpenEmu-Swift.h"

static NSUserInterfaceItemIdentifier const CheckboxType  = @"Checkbox";
static NSUserInterfaceItemIdentifier const SliderType    = @"Slider";
static NSUserInterfaceItemIdentifier const GroupType     = @"Group";

@interface OEShaderParameterValue (View)
- (NSString *)selectCellType;
@end

@interface OEShaderParametersWindowController ()<NSTableViewDelegate, NSTableViewDataSource>
{
    OEGameViewController * __weak _controller;
}
@end

@implementation OEShaderParametersWindowController

- (instancetype)initWithGameViewController:(OEGameViewController *)controller
{
    if ((self = [super initWithWindowNibName:@"ShaderParameters"]))
    {
        _controller = controller;
    }
    return self;
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary<NSKeyValueChangeKey,id> *)change context:(void *)context
{
    OEShaderParameterValue *param = (OEShaderParameterValue *)object;
    [_controller.document gameViewController:_controller
                     setShaderParameterValue:param.value.doubleValue
                                    forIndex:param.index];
    [self.shader writeWithParameters:_params identifier:_controller.document.systemIdentifier];
}

- (void)windowDidLoad {
    [super windowDidLoad];
    
    NSTableView *tableView = self.tableView;
    [tableView setDelegate:self];
    [tableView setDataSource:self];
    
    [tableView setHeaderView:nil];
    [tableView setRowHeight:30.0];
    [tableView setGridStyleMask:0];
    [tableView setAllowsColumnReordering:NO];
    [tableView setAllowsColumnResizing:NO];
    [tableView setAllowsColumnSelection:NO];
    [tableView setAllowsEmptySelection:YES];
    [tableView setAllowsMultipleSelection:NO];
    [tableView setAllowsTypeSelect:NO];
    
    //[tableView registerNib:nil forIdentifier:SliderType];
    
}

- (void)setParams:(NSArray<OEShaderParameterValue *> *)params
{
    if ([_params isEqualTo:params])
    {
        return;
    }
    
    [self willChangeValueForKey:@"params"];
    
    if (_params)
    {
        [_params removeObserver:self
           fromObjectsAtIndexes:[NSIndexSet indexSetWithIndexesInRange:NSMakeRange(0, _params.count)]
                     forKeyPath:@"value"];
    }

    // are there groups?
    BOOL hasGroups = NO;
    {
        NSEnumerator<OEShaderParameterValue *> *pe = params.objectEnumerator;
        OEShaderParameterValue *p = [pe nextObject];
        NSString *group = p.group;
        
        while ((p = [pe nextObject])) {
            if (![p.group isEqualToString:group]) {
                hasGroups = YES;
                break;
            }
        }
    }
    
    if (hasGroups)
    {
        NSMutableArray<OEShaderParameterValue *> *p2 = [NSMutableArray new];
        NSEnumerator<OEShaderParameterValue *> *pe = params.objectEnumerator;
        OEShaderParameterValue *p = [pe nextObject];
        while (p != nil) {
            NSString *group = p.group;
            // add a dummy group parameter
            [p2 addObject:[OEShaderParameterValue groupWithName:group]];
            while (p != nil && [p.group isEqualToString:group])
            {
                [p2 addObject:p];
                p = [pe nextObject];
            }
        }
        params = p2;
    }
    
    _params = params;
    
    [self didChangeValueForKey:@"params"];
    
    if (_params)
    {
        [_params addObserver:self
          toObjectsAtIndexes:[NSIndexSet indexSetWithIndexesInRange:NSMakeRange(0, _params.count)]
                  forKeyPath:@"value"
                     options:NSKeyValueObservingOptionNew
                     context:nil];
    }
    
    [self.tableView reloadData];
}

#pragma mark - Actions

- (IBAction)resetAll:(id)sender
{
    [_params enumerateObjectsUsingBlock:^(OEShaderParameterValue * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
        obj.value = obj.initial;
    }];
    
}

#pragma mark - NSTableViewDelegate

- (BOOL)tableView:(NSTableView *)tableView shouldTrackCell:(NSCell *)cell forTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    return YES;
}

- (BOOL)tableView:(NSTableView *)tableView shouldSelectRow:(NSInteger)row
{
    return NO;
}

- (NSView*)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    OEShaderParameterValue *param = [_params objectAtIndex:row];
    NSString *type = [param selectCellType];
    NSView *cellView = [tableView makeViewWithIdentifier:type owner:self];

    if(type == CheckboxType)
    {
        NSButton *checkbox = cellView.subviews.lastObject;
        [checkbox setTitle:param.desc];

        NSDictionary *options = @{ NSContinuouslyUpdatesValueBindingOption:@YES };
        [checkbox bind:@"value" toObject:param withKeyPath:@"value" options:options];
    }
    else if(type == SliderType)
    {
        NSTextField *lbl = cellView.subviews.firstObject;
        [lbl setStringValue:param.desc];
        
        NSSlider  *slid  = [cellView viewWithTag:100];
        slid.continuous  = YES;
        slid.minValue    = param.minimum.doubleValue;
        slid.maxValue    = param.maximum.doubleValue;
        NSUInteger ticks = (NSUInteger)(slid.maxValue - slid.minValue) + 1;
        if (param.step.doubleValue == 1.0 && ticks <= 11)
        {
            slid.numberOfTickMarks        = ticks;
            slid.allowsTickMarkValuesOnly = YES;
        }
        else
        {
            slid.numberOfTickMarks = 0;
            slid.allowsTickMarkValuesOnly = NO;
        }
        
        NSTextField *num  = [cellView viewWithTag:101];
        
        NSStepper *step = [cellView viewWithTag:102];
        step.minValue   = param.minimum.doubleValue;
        step.maxValue   = param.maximum.doubleValue;
        step.increment  = param.step.doubleValue;
        
        NSDictionary *options = @{ NSContinuouslyUpdatesValueBindingOption:@YES };
        
        [slid bind:@"value" toObject:param withKeyPath:@"value" options:options];
        [num bind:@"value" toObject:param withKeyPath:@"value" options:options];
        [step bind:@"value" toObject:param withKeyPath:@"value" options:options];
    } else if (type == GroupType)
    {
        NSTextField *lbl = cellView.subviews.firstObject;
        [lbl setStringValue:param.group];
    }
    
    return cellView;
}

#pragma mark - NSTableViewDataSource

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
    return [_params count];
}

- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    return [_params objectAtIndex:row];
}

@end

@implementation OEShaderParameterValue (View)

- (NSString *)selectCellType
{
    if (self.index == -1) {
        return GroupType;
    }
    
    if (self.minimum.doubleValue == 0.0 && self.maximum.doubleValue == 1.0 && self.step.doubleValue == 1.0)
    {
        return CheckboxType;
    }
    return SliderType;
}

@end
