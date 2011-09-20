/*
    SyphonServer.h
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

/*! @name Server Options Dictionary Key Constants */
/*! @{ */

/*!
 @relates SyphonServer
 If this key is matched with a NSNumber with a BOOL value YES, then the server will be invisible to other Syphon users. You are then responsible for passing the NSDictionary returned by serverDescription to processes which require it to create a SyphonClient. Default is NO.
 */
extern NSString * const SyphonServerOptionIsPrivate;

/*! @} */

/*!
 @nosubgrouping
 SyphonServer handles the publishing of frames from one video source to any number of clients. Frames can be published either by passing in an existing OpenGL texture, or by binding the server's FBO, drawing using OpenGL calls, then calling the unbindAndPublish method.
 
 A SyphonServer object represents one video output for your application. If your application produces several video outputs, then they should each have their own SyphonServer. If your application might have multiple servers running, you should name each server to aid identification by users.
 
 It is safe to access instances of this class across threads, except for those calls related to OpenGL: a call to bindToDrawFrameOfSize: must have returned before a call is made to unbindAndPublish, and these methods must be paired and called in order. You should not call the stop method while the FBO is bound.
 */

@class SyphonImage;

@interface SyphonServer : NSObject
{
@private
 NSString *_name;
 NSString *_uuid;
 BOOL _broadcasts;

 id _connectionManager;

 CGLContextObj cgl_ctx;

 void *_surfaceRef;
 BOOL _pushPending;
 SyphonImage *_surfaceTexture;
 GLuint _surfaceFBO;

 GLint _previousReadFBO;
 GLint _previousDrawFBO;
 GLint _previousFBO;

 int32_t _mdLock;
}
/** @name Instantiation */
/** @{ */
/*!
 Creates a new server with the specified human-readable name (which need not be unique), CGLContext and options. The server will be started immediately. Init may fail and return nil if the server could not be started.
 @param serverName Non-unique human readable server name. This is not required and may be nil, but is usually used by clients in their UI to aid identification.
 @param context The CGLContextObj context that textures will be valid and available on for publishing.
 @param options A dictionary containing key-value pairs to specify options for the server. Currently the only option is SyphonServerOptionIsPrivate. See its description for details.
 @returns A newly intialized SyphonServer. Nil on failure.
*/

- (id)initWithName:(NSString*)serverName context:(CGLContextObj)context options:(NSDictionary *)options;

/** @} */

/** @name Properties */
/** @{ */
/*!
 The CGLContext the server uses for drawing. This may or may not be the context passed in at init.
*/

@property (readonly) CGLContextObj context;

/*! 
 A string representing the name of the SyphonServer.
*/

@property (retain) NSString* name;

/*! 
 A dictionary describing the server. Normally you won't need to access this, however if you created the server as private (using SyphonServerOptionIsPrivate) then you must pass this dictionary to any process in which you wish to create a SyphonClient. You should not rely on the presence of any particular keys in this dictionary. The content will always conform to the \<NSCoding\> protocol.
*/

@property (readonly) NSDictionary* serverDescription;

/*! 
YES if clients are currently attached, NO otherwise. If you generate frames frequently (for instance on a display-link timer), you may choose to test this and only call publishFrameTexture:textureTarget:imageRegion:textureDimensions:flipped: when clients are attached.
*/

@property (readonly) BOOL hasClients;

/** @} */
/** @name Publishing Frames */
/** @{ */

/*! 
 Publishes the part of the texture described in region of the named texture to clients. The texture is copied and can be safely disposed of or modified once this method has returned. You should not bracket calls to this method with calls to -bindToDrawFrameOfSize: and -unbindAndPublish - they are provided as an alternative to using this method.
 
 This method does not lock the server's CGL context. If there is a chance of other threads using the context during calls to this method, bracket it with calls to CGLLockContext() and CGLUnlockContext(), passing in the value of the server's context property as the argument.
 @param texID The name of the texture to publish, which must be a texture valid in the CGL context provided when the server was created.
 @param target GL_TEXTURE_RECTANGLE_EXT or GL_TEXTURE_2D.
 @param region The sub-region of the texture to publish.
 @param size The full size of the texture
 @param isFlipped Is the texture flipped?
*/

- (void)publishFrameTexture:(GLuint)texID textureTarget:(GLenum)target imageRegion:(NSRect)region textureDimensions:(NSSize)size flipped:(BOOL)isFlipped;

/*! 
 Binds an FBO for you to publish a frame of the given dimensions by drawing into the server's context (check it using the context property). If YES is returned, you must pair this with a call to -unbindAndPublish once you have finished drawing. If NO is returned you should abandon drawing and not call -unbindAndPublish.
 This method does not lock the server's CGL context. If there is a chance other threads may use the context during calls to this method, bracket it with calls to CGLLockContext() and CGLUnlockContext(), passing in the value of the server's context property as the argument.
 @param size The size the frame you wish to publish.
 @returns YES if binding succeeded, NO otherwise.
*/

- (BOOL)bindToDrawFrameOfSize:(NSSize)size;

/*! 
 Restores any previously bound FBO and publishes the just-drawn frame. This method will flush the GL context (so you don't have to).
 This method does not lock the server's CGL context. If there is a chance of other threads using the context during calls to this method, bracket it with calls to CGLLockContext() and CGLUnlockContext(), passing in the value of the server's context property as the argument.
*/

- (void)unbindAndPublish;

/*! 
 Returns a SyphonImage representing the current output from the server, valid in the server's CGL context. Call this method every time you wish to access the current server frame. This object has a limited useful lifetime, and may have GPU resources associated with it: you should release it as soon as you are finished drawing with it.
 
 This method does not lock the CGL context. If there is a chance other threads may use the context during calls to this method, or while you are drawing with the returned SyphonImage, bracket access with calls to CGLLockContext() and CGLUnlockContext().
  
 @returns A SyphonImage representing the current output from the server. YOU ARE RESPONSIBLE FOR RELEASING THIS OBJECT when you are finished with it. 
 */
- (SyphonImage *)newFrameImage;

/*! 
 Stops the server instance. In garbage-collected applications you must call this method prior to removing strong references to the server. In non-garbage-collected applications, use of this method is optional.
*/

- (void)stop;
/** @} */
@end
