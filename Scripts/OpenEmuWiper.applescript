-- 
--  Copyright (c) 2012, OpenEmu Team
--  
--  
--  Redistribution and use in source and binary forms, with or without
--  modification, are permitted provided that the following conditions are met:
--      * Redistributions of source code must retain the above copyright
--        notice, this list of conditions and the following disclaimer.
--      * Redistributions in binary form must reproduce the above copyright
--        notice, this list of conditions and the following disclaimer in the
--        documentation and/or other materials provided with the distribution.
--      * Neither the name of the OpenEmu Team nor the
--        names of its contributors may be used to endorse or promote products
--        derived from this software without specific prior written permission.
--  
--  THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
--  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
--  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
--  DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
--  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
--  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
--   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
--  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
--  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
--   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
--

tell application "Finder"
	set title to "Wipe out OpenEmu data?"
	
	set msg to "Are you sure you want to wipe out OpenEmu data? OpenEmu’s ROM library will be removed, too.
	
	The following files and folders will be removed:
	
	• ~/Library/Application Support/OpenEmu/
	• ~/Library/Preferences/org.openemu.*
	• ~/Library/Saved Application State/
	    org.openemu.OpenEmu.savedState/
	• ~/Library/Caches/org.openemu.OpenEmu/"
	
	display alert title message msg as warning buttons {"Cancel", "Delete"} default button "Cancel" cancel button "Cancel"
	
	set openEmuRunning to true
	repeat while openEmuRunning
		tell application "System Events" to set openEmuRunning to exists (processes where name is "OpenEmu")
		if openEmuRunning then
			display alert "OpenEmu is currently running" message "OpenEmu must quit before wiping its data." as warning buttons {"Cancel", "Quit OpenEmu For Me"} default button "Quit OpenEmu For Me" cancel button "Cancel"
		end if
		tell application "OpenEmu"
			quit
		end tell
		delay 1
	end repeat
	
	try
		set appSupportPath to path to application support from user domain
		set openEmuAppSupport to folder "OpenEmu" of appSupportPath
		delete openEmuAppSupport
	end try
	
	set libraryPath to path to library folder from user domain
	
	try
		set shellScript to "ls " & POSIX path of libraryPath & "Preferences/org.openemu.*"
		tell current application to set plists to do shell script shellScript
		set plistArray to paragraphs of plists
		
		repeat with plist in plistArray
			set plistFile to my POSIX file plist
			delete plistFile
		end repeat
		
	end try
	
	try
		set appStatePath to folder "Saved Application State" of libraryPath
		set openEmuStatePath to folder "org.openemu.OpenEmu.savedState" of appStatePath
		delete openEmuStatePath
	end try
	
	try
		set cachePath to folder "Caches" of libraryPath
		set openEmuCachePath to folder "org.openemu.OpenEmu" of cachePath
		delete OpenEmuCachePath
	end try
	
	display alert "OpenEmu data have been wiped out" buttons {"OK"}
end tell
