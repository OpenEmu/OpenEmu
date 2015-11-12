/*
 Copyright (c) 2013, OpenEmu Team

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

@import Cocoa;

@interface NSDocument (OEAdditions)

- (void)canCloseDocumentWithCompletionHandler:(void(^)(NSDocument *document, BOOL shouldClose))handler;
- (void)shouldCloseWindowController:(NSWindowController *)windowController withCompletionHandler:(void(^)(NSDocument *document, BOOL shouldClose))handler;

- (void)runModalSavePanelForSaveOperation:(NSSaveOperationType)saveOperation withCompletionHandler:(void(^)(NSDocument *document, BOOL didSave))handler;
- (void)saveToURL:(NSURL *)absoluteURL ofType:(NSString *)typeName forSaveOperation:(NSSaveOperationType)saveOperation withCompletionHandler:(void(^)(NSDocument *document, BOOL didSaveSuccessfully))handler;
- (void)autosaveDocumentWithCompletionHandler:(void(^)(NSDocument *document, BOOL didAutosaveSuccessfully))handler;
- (void)saveDocumentWithCompletionHandler:(void(^)(NSDocument *document, BOOL didSaveSuccessfully))handler;
- (void)duplicateWithCompletionHandler:(void(^)(NSDocument *document, BOOL didDuplicate))handler;

- (void)runModalPageLayoutWithPrintInfo:(NSPrintInfo *)printInfo completionHandler:(void(^)(NSDocument *document, BOOL userAccepted))handler;
- (void)runModalPrintOperation:(NSPrintOperation *)printOperation completionHandler:(void(^)(NSDocument *document, BOOL didPrintSuccessfully))handler;
- (void)printDocumentWithSettings:(NSDictionary *)printSettings showPrintPanel:(BOOL)showPrintPanel completionHandler:(void(^)(NSDocument *document, BOOL didPrintSuccessfully))handler;
- (void)presentError:(NSError *)error modalForWindow:(NSWindow *)window completionHandler:(void(^)(BOOL didRecover))handler;

@end
