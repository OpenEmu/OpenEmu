MagicKit.framework
========

`MagicKit.framework` is an easy-to-use wrapper around the [`libmagic`](http://www.darwinsys.com/file/) file identification library. It provides a high-level Objective-C interface and deals in Foundation types (NSString, NSData, et al).

`MagicKit.framework` is available for both OS X (as a framework) or iOS as a static library with header files. `MagicKit.framework` consists of a master class, `GEMagicKit`, and an intermediary class, `GEMagicResult`. The `GEMagicKit` master class has six class methods, listed below in descending order of convenience.

    + (GEMagicResult *)magicForFileAtPath:(NSString *)path;
    + (GEMagicResult *)magicForFileAtURL:(NSURL *)aURL;
    + (GEMagicResult *)magicForData:(NSData *)data;
    
    + (GEMagicResult *)magicForFileAtPath:(NSString *)path decompress:(BOOL)decompress;
    + (GEMagicResult *)magicForFileAtURL:(NSURL *)aURL decompress:(BOOL)decompress;
    + (GEMagicResult *)magicForData:(NSData *)data decompress:(BOOL)decompress;

These methods all return the `GEMagicResult` intermediary class. The `GEMagicResult` class has four properties. These are `mimeType`, `description`, `uniformType` and `uniformTypeHierarchy`. the first three properties are `NSString` objects, the final returns an `NSArray` object.

Example MIME types include:

* `application/xml; charset=us-ascii` 
* `video/x-msvideo; charset=binary` 

Or a textual description, respectively:

* `XML  document text` 
* `RIFF (little-endian) data, AVI, 624 x 352, 23.98 fps, video: XviD, audio: MPEG-1 Layer 3 (stereo, 48000 Hz)`)

Finally, `uniformType` and `uniformTypeHierarchy` return [Uniform Type Identifiers](http://en.wikipedia.org/wiki/Uniform_Type_Identifier) for the relevant file/data. The type hierarchy is an array of types that the given file conforms to.

`GEMagicKit` can also decompress compressed files and hence attempt to determine the contents within.

`MagicKit.framework` is [MIT](http://www.opensource.org/licenses/mit-license.html)-licensed. `libmagic` is [BSD](http://www.opensource.org/licenses/bsd-license.php)-licensed. 