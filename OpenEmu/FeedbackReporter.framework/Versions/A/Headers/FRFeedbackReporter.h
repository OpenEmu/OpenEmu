/*
 * Copyright 2008-2010, Torsten Curdt
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#import <Cocoa/Cocoa.h>

@protocol FRFeedbackReporterDelegate
@optional
- (NSDictionary*) customParametersForFeedbackReport;
- (NSMutableDictionary*) anonymizePreferencesForFeedbackReport:(NSMutableDictionary *)preferences;
@end

@interface FRFeedbackReporter : NSObject {

    @private
        id feedbackController;
        id<FRFeedbackReporterDelegate> delegate;
}

// Creates and returns the singleton FRFeedbackReporter. Does not perform any checks or other real work.
+ (FRFeedbackReporter *)sharedReporter;

// Gets/sets the delegate.
- (id<FRFeedbackReporterDelegate>) delegate;
- (void) setDelegate:(id<FRFeedbackReporterDelegate>) delegate;

// Displays the feedback user interface allowing the user to provide general feedback. Returns YES if it was able to display the UI, NO otherwise.
- (BOOL) reportFeedback;

// Searches the disk for crash logs, and displays the feedback user interface if there are crash logs newer than since the last check. Updates the 'last crash check date' in user defaults. Returns YES if it was able to display the UI, NO otherwise.
- (BOOL) reportIfCrash;

// Displays the feedback user interface for the given exception. Do not pass nil. Returns YES if it was able to display the UI, NO otherwise.
- (BOOL) reportException:(NSException *)exception;

@end
