/*
 Copyright (c) 2009 Remy Demarest
 
 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without
 restriction, including without limitation the rights to use,
 copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following
 conditions:
 
 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
 */

#import "PSYBlockTimer.h"

typedef void (^PSYTimerBlock)(NSTimer *);

@interface NSTimer (PSYBlockTimer_private)
+ (void)PSYBlockTimer_executeBlockWithTimer:(NSTimer *)timer;
@end


@implementation NSTimer (PSYBlockTimer)
+ (NSTimer *)PSY_scheduledTimerWithTimeInterval:(NSTimeInterval)seconds repeats:(BOOL)repeats usingBlock:(void (^)(NSTimer *timer))fireBlock
{
    return [self scheduledTimerWithTimeInterval:seconds target:self selector:@selector(PSYBlockTimer_executeBlockWithTimer:) userInfo:[fireBlock copy] repeats:repeats];
}

+ (NSTimer *)PSY_timerWithTimeInterval:(NSTimeInterval)seconds repeats:(BOOL)repeats usingBlock:(void (^)(NSTimer *timer))fireBlock
{
    return [self timerWithTimeInterval:seconds target:self selector:@selector(PSYBlockTimer_executeBlockWithTimer:) userInfo:[fireBlock copy] repeats:repeats];
}
@end


@implementation NSTimer (PSYBlockTimer_private)
+ (void)PSYBlockTimer_executeBlockWithTimer:(NSTimer *)timer
{
    if([timer isValid])
    {
        PSYTimerBlock block = [timer userInfo];
        block(timer);
    }
}
@end
