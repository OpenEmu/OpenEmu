#!/usr/bin/ruby

require 'find'

def try_xcodebuild(path)
  if FileTest.directory?(path)
   if File.basename(path)[0] == ?.
     return
   else
     print "Building project at " + File.basename(path) + "\n"
     Dir.chdir(path)
     `xcodebuild -target 'Build & Install' -configuration Release`
   end
 end
end

wd = Dir.getwd
Dir.foreach(wd) {|x| try_xcodebuild(wd+'/'+x)}
