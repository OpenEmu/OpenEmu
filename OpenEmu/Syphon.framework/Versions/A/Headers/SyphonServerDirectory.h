/*
    SyphonServerDirectory.h
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

/*! @name Server Description Dictionary Key Constants */
/*! @{ */

/*!
 @relates SyphonServerDirectory
 The object for this key is a NSString which uniquely identifies a SyphonServer instance. If two dictionaries contain the same string for this key, they represent the same server. This is provided solely to allow you to programmatically determine the identity of a server, and should never be displayed to users in interface elements.
*/

extern NSString * const SyphonServerDescriptionUUIDKey;

/*!
 @relates SyphonServerDirectory
 The object for this key is a NSString which is the human-readable non-unique name for the SyphonServer. If this string exists and is non-empty, you should use it in interface elements to identify the server, usually in combination with the name of the server's application (see SyphonServerDescriptionAppNameKey).
*/

extern NSString * const SyphonServerDescriptionNameKey;

/*!
 @relates SyphonServerDirectory
 The object for this key is a NSString with the localized name of the application in which the SyphonServer is running. Use this in combination with the server's name (if present) to identify the server in interface elements.
*/

extern NSString * const SyphonServerDescriptionAppNameKey;

/*!
 @relates SyphonServerDirectory
 The object for this key is a NSImage representation of the icon of the application in which the SyphonServer is running.
*/

extern NSString * const SyphonServerDescriptionIconKey;

/*! @} */

/*! @name Notifications */
/*! @{ */

/*!
 @relates SyphonServerDirectory
 A new SyphonServer is available on the system. The notification object is a NSDictionary describing the server.
*/

extern NSString * const SyphonServerAnnounceNotification;

/*!
 @relates SyphonServerDirectory
 An existing SyphonServer instance has changed its description. The notification object is a NSDictionary describing the server.
*/

extern NSString * const SyphonServerUpdateNotification;

/*!
 @relates SyphonServerDirectory
 A SyphonServer instance will no longer be available. The notification object is a NSDictionary describing the retiring server.
*/

extern NSString * const SyphonServerRetireNotification;

/*! @} */

/*! 
 @nosubgrouping
 SyphonServerDirectory provides information on available Syphon Servers. Servers are represented by dictionaries. Generally you can expect to find some or all of the keys listed in Constants.
*/

@interface SyphonServerDirectory : NSObject {
@private
 NSMutableArray *_servers;
 pthread_mutex_t _generalLock;
 pthread_mutex_t _mutateLock;
 NSMutableSet *_pings;
}

/*!
 Returns the shared server directory instance. This object is KVO complaint, and can be used to observe changes in server availability, server names and statuses. 
 @returns the shared server instance 
*/

+ (SyphonServerDirectory *)sharedDirectory;

/*!
 NSArray of NSDictionaries that describe (using the keys above) currently available SyphonServer instances on the system.
*/

@property (readonly) NSArray *servers;

/*! 
 Use this method to discover servers based soley on their name, or application host name. Both parameters are optional. If you do not specify either, all available SyphonServers will be returned.
 @param name Optional (pass nil to not specify) Name of the published SyphonServer, matches the key value for SyphonServerDescriptionNameKey
 @param appname Optional (pass nil to not specify) Application Name of the published SyphonServer, matches the key value for SyphonServerDescriptionAppNameKey
 @returns An array of NSDictionaries matching the query you specified. 
*/

- (NSArray *)serversMatchingName:(NSString *)name appName:(NSString *)appname;

@end
