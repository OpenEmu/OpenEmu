//
//  ArchiveVG_OutputFormats.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 10.05.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef OpenEmu_ArchiveVG_OutputFormats_h
#define OpenEmu_ArchiveVG_OutputFormats_h
typedef enum {
	AVGOutputFormatXML,
	AVGOutputFormatJSON,
	AVGOutputFormatYAML,
} AVGOutputFormat;
const AVGOutputFormat AVGDefaultOutputFormat = AVGOutputFormatXML;
#endif
