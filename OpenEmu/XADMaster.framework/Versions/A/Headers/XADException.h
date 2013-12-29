#import <Foundation/Foundation.h>

typedef int XADError;

#define XADNoError               0x0000 /* no error */
#define XADUnknownError          0x0001 /* unknown error */
#define XADInputError            0x0002 /* input data buffers border exceeded */
#define XADOutputError           0x0003 /* failed to write to file */
#define XADBadParametersError    0x0004 /* function called with illegal parameters */
#define XADOutOfMemoryError      0x0005 /* not enough memory available */
#define XADIllegalDataError      0x0006 /* data is corrupted */
#define XADNotSupportedError     0x0007 /* file not fully supported */
#define XADResourceError         0x0008 /* required resource missing */
#define XADDecrunchError         0x0009 /* error on decrunching */
#define XADFiletypeError         0x000A /* unknown file type */
#define XADOpenFileError         0x000B /* opening file failed */
#define XADSkipError             0x000C /* file, disk has been skipped */
#define XADBreakError            0x000D /* user break in progress hook */
#define XADFileExistsError       0x000E /* file already exists */
#define XADPasswordError         0x000F /* missing or wrong password */
#define XADMakeDirectoryError    0x0010 /* could not create directory */
#define XADChecksumError         0x0011 /* wrong checksum */
#define XADVerifyError           0x0012 /* verify failed (disk hook) */
#define XADGeometryError         0x0013 /* wrong drive geometry */
#define XADDataFormatError       0x0014 /* unknown data format */
#define XADEmptyError            0x0015 /* source contains no files */
#define XADFileSystemError       0x0016 /* unknown filesystem */
#define XADFileDirectoryError    0x0017 /* name of file exists as directory */
#define XADShortBufferError      0x0018 /* buffer was too short */
#define XADEncodingError         0x0019 /* text encoding was defective */
#define XADLinkError             0x001a /* could not create link */

#define XADSubArchiveError 0x10000

#ifndef CLANG_ANALYZER_NORETURN
	#ifdef __clang__
		#if __has_feature(attribute_analyzer_noreturn)
			#define CLANG_ANALYZER_NORETURN __attribute__((analyzer_noreturn))
		#else
			#define CLANG_ANALYZER_NORETURN
		#endif
	#else
		#define CLANG_ANALYZER_NORETURN
	#endif
#endif

extern NSString *XADExceptionName;

@interface XADException:NSObject
{
}

+(void)raiseUnknownException CLANG_ANALYZER_NORETURN;
+(void)raiseInputException CLANG_ANALYZER_NORETURN;
+(void)raiseOutputException CLANG_ANALYZER_NORETURN;
+(void)raiseIllegalDataException CLANG_ANALYZER_NORETURN;
+(void)raiseNotSupportedException CLANG_ANALYZER_NORETURN;
+(void)raiseDecrunchException CLANG_ANALYZER_NORETURN;
+(void)raisePasswordException CLANG_ANALYZER_NORETURN;
+(void)raiseChecksumException CLANG_ANALYZER_NORETURN;
+(void)raiseDataFormatException CLANG_ANALYZER_NORETURN;
+(void)raiseOutOfMemoryException CLANG_ANALYZER_NORETURN;
+(void)raiseExceptionWithXADError:(XADError)errnum CLANG_ANALYZER_NORETURN;

+(XADError)parseException:(id)exception;
+(NSString *)describeXADError:(XADError)errnum;

@end
