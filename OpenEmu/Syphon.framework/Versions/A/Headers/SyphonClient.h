/*
    SyphonClient.h
    Syphon

    Copyright 2010-2011 bangnoise (Tom Butterworth) & vade (Anton Marini).
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
 
    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#import <Cocoa/Cocoa.h>
#import <Quartz/Quartz.h>
#import <OpenGL/OpenGL.h>

@class SyphonImage;

/*! 
 SyphonClient makes available frames from a remote SyphonServer. A client is created from a NSDictionary which describes the server. Typically this is obtained from the shared SyphonServerDirectory, or one of Syphon's notifications.
 
 SyphonClient allows for lazy drawing by the use of a new-frame-handler. Using a handler you can perform drawing without using a timer or polling, achieving frame-accuracy with the minimum of overhead. Alternatively, if your application uses a traditional display link or timer, you can use the hasNewFrame property to make decisions about work you may need to do. Irrespective of the presence of new frames, you can draw with a SyphonClient at any time.
 
 It is safe to access instances of this class across threads.
 */

@interface SyphonClient : NSObject
{
@private
 id _connectionManager;
 uint32_t _lastFrameID;
 void (^_handler)(id);
 int32_t _status;
 int32_t _lock;
}
/*! 
 Returns a new client instance for the described server. You should check the isValid property after initialization to ensure a connection was made to the server.
 @param description Typically acquired from the shared SyphonServerDirectory, or one of Syphon's notifications.
 @param options Currently ignored. May be nil.
 @param handler A block which is invoked when a new frame becomes available. handler may be nil. This block may be invoked on a thread other than that on which the client was created.
 @returns A newly initialized SyphonClient object, or nil if a client could not be created.
*/

- (id)initWithServerDescription:(NSDictionary *)description options:(NSDictionary *)options newFrameHandler:(void (^)(SyphonClient *client))handler;

/*!
 A client is valid if it has a working connection to a server. Once this returns NO, the SyphonClient will not yield any further frames.
 */

@property (readonly) BOOL isValid;

/*!
 Returns a dictionary with a description of the server the client is attached to. See SyphonServerDirectory for the keys this dictionary contains
*/

@property (readonly) NSDictionary *serverDescription;

/*!
 Returns YES if the server has output a new frame since the last time newFrameImageForContext: was called for this client, NO otherwise.
*/

@property (readonly) BOOL hasNewFrame;

/*!
 Returns a SyphonImage representing the current output from the server. The texture associated with the image may continue to update when you draw with it, but you should not depend on that behaviour: call this method every time you wish to access the current server frame. This object may have GPU resources associated with it and you should release it as soon as you are finished drawing with it.
 
 This method does not lock the CGL context. If there is a chance other threads may use the context during calls to this method, bracket it with calls to CGLLockContext() and CGLUnlockContext().
 @param cgl_ctx The CGL context in which the SyphonImage will be valid.
 @returns A SyphonImage representing the live output from the server. YOU ARE RESPONSIBLE FOR RELEASING THIS OBJECT when you are finished with it.
 */
- (SyphonImage *)newFrameImageForContext:(CGLContextObj)cgl_ctx;

/*!
 Stops the client from receiving any further frames from the server. In garbage-collected applications you must call this method prior to removing strong references to the client. In non-garbage-collected applications, use of this method is optional and releasing all references to the client has the same effect.
 */

- (void)stop;

@end
