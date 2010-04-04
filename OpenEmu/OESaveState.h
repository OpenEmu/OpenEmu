/*
 Copyright (c) 2009, OpenEmu Team
 All rights reserved.
 
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
#import <Quartz/Quartz.h>

@class OEROMFile;

@interface OESaveState : NSManagedObject
{
    NSString *emulatorID;
    NSString *userDescription;
}

- (id)initInsertedIntoManagedObjectContext:(NSManagedObjectContext *)context;

@property(nonatomic, retain) OEROMFile *romFile;
@property(readonly)          NSBundle  *bundle;

@property(nonatomic, retain) NSData    *pathAlias;
@property(nonatomic, retain) NSString  *bundlePath;
@property(readonly)          NSString  *screenshotPath;
@property(readonly)          NSString  *saveDataPath;

@property(nonatomic, retain) NSImage   *screenshot;
@property(nonatomic, retain) NSData    *saveData;
@property(nonatomic, retain) NSString  *emulatorID;

@property(nonatomic, retain) NSDate    *timeStamp;

@property(readonly)          NSString  *resourcePath;
@property(readonly)          NSString  *infoPlistPath;

@end
