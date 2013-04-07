/*
 Copyright (c) 2011, OpenEmu Team
 
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

#import "OELocalizationHelper.h"

NSString *const OERegionKey = @"region";

@interface OELocalizationHelper ()
- (void)OE_updateRegion;
@end

#define OERegionNALocales [NSArray arrayWithObjects:@"", nil]

@implementation OELocalizationHelper
@synthesize region;

static OELocalizationHelper *sharedHelper;

+ (OELocalizationHelper *)sharedHelper
{    
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedHelper = [self new];
    });
    
	return sharedHelper;
}

- (id)init
{
    if(self = [super init])
	{
		[self OE_updateRegion];

        [[NSUserDefaults standardUserDefaults] addObserver:self forKeyPath:OERegionKey options:0 context:nil];
    }
	return self;
}

- (void)dealloc
{
    [[NSUserDefaults standardUserDefaults] removeObserver:self forKeyPath:OERegionKey];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    [self OE_updateRegion];
}

#pragma mark -
- (BOOL)isRegionNA  { return region == OERegionNA;  }
- (BOOL)isRegionEU  { return region == OERegionEU;  }
- (BOOL)isRegionJAP { return region == OERegionJAP; }

- (NSString*)regionName
{
    switch ([self region])
    {
        case OERegionEU:
            return NSLocalizedString(@"Europe", "");
            break;
        case OERegionNA:
            return NSLocalizedString(@"North America", "");
            break;
        case OERegionJAP:
            return NSLocalizedString(@"Japan", "");
            break;
            
        default:
            break;
    }
    return NSLocalizedString(@"Other Region", "");
}
#pragma mark -
#define OERegionCodesAfrica [NSArray arrayWithObjects:@"AO",@"BF",@"BI",@"BJ",@"BW",@"CD",@"CF",@"CG",@"CI",@"CM",@"CV",@"DJ",@"DZ",@"EG",@"EH",@"ER",@"ET",@"GA",@"GH",@"GM",@"GN",@"GQ",@"GW",@"KE",@"KM",@"LR",@"LS",@"LY",@"MA",@"MG",@"ML",@"MR",@"MU",@"MW",@"MZ",@"NA",@"NE",@"NG",@"RE",@"RW",@"SC",@"SD",@"SH",@"SL",@"SN",@"SO",@"SS",@"ST",@"SZ",@"TD",@"TG",@"TN",@"TZ",@"UG",@"YT",@"ZA",@"ZM",@"ZW",nil]
#define OERegionCodesAntarctica [NSArray arrayWithObjects:@"AQ",@"BV",@"GS",@"HM",@"TF",nil]
#define OERegionCodesAsia [NSArray arrayWithObject:@"AE",@"AF",@"AM",@"AZ",@"BD",@"BH",@"BN",@"BT",@"CC",@"CN",@"CX",@"CY",@"GE",@"HK",@"ID",@"IL",@"IN",@"IO",@"IQ",@"IR",@"JO",@"JP",@"KG",@"KH",@"KP",@"KR",@"KW",@"KZ",@"LA",@"LB",@"LK",@"MM",@"MN",@"MO",@"MV",@"MY",@"NP",@"OM",@"PH",@"PK",@"PS",@"QA",@"SA",@"SG",@"SY",@"TH",@"TJ",@"TL",@"TM",@"TR",@"TW",@"UZ",@"VN",@"YE",nil]
#define OERegionCodesEurope [NSArray arrayWithObjects:@"AD",@"AL",@"AT",@"AX",@"BA",@"BE",@"BG",@"BY",@"CH",@"CZ",@"DE",@"DK",@"EE",@"ES",@"FI",@"FO",@"FR",@"GB",@"GG",@"GI",@"GR",@"HR",@"HU",@"IE",@"IM",@"IS",@"IT",@"JE",@"LI",@"LT",@"LU",@"LV",@"MC",@"MD",@"ME",@"MK",@"MT",@"NL",@"NO",@"PL",@"PT",@"RO",@"RS",@"RU",@"SE",@"SI",@"SJ",@"SK",@"SM",@"UA",@"VA",nil]
#define OERegionCodesNorthAmerica [NSArray arrayWithObjects:@"AG",@"AI",@"AW",@"BB",@"BL",@"BM",@"BQ",@"BS",@"BZ",@"CA",@"CR",@"CU",@"CW",@"DM",@"DO",@"GD",@"GL",@"GP",@"GT",@"HN",@"HT",@"JM",@"KN",@"KY",@"LC",@"MF",@"MQ",@"MS",@"MX",@"NI",@"PA",@"PM",@"PR",@"SV",@"SX",@"TC",@"TT",@"US",@"VC",@"VG",@"VI",nil]
#define OERegionCodesSouthAmerica [NSArray arrayWithObjects:@"AR",@"BO",@"BR",@"CL",@"CO",@"EC",@"FK",@"GF",@"GY",@"PE",@"PY",@"SR",@"UY",@"VE",nil]
#define OERegionCodesOceania [NSArray arrayWithObjects:@"AS",@"AU",@"CK",@"FJ",@"FM",@"GU",@"KI",@"MH",@"MP",@"NC",@"NF",@"NR",@"NU",@"NZ",@"PF",@"PG",@"PN",@"PW",@"SB",@"TK",@"TO",@"TV",@"UM",@"VU",@"WF",@"WS",nil]
#pragma mark -
#pragma mark Private Methods

- (void)OE_updateRegion
{
	NSUserDefaults *standardUserDefaults = [NSUserDefaults standardUserDefaults];
	if([standardUserDefaults valueForKey:OERegionKey])
		region = [[standardUserDefaults valueForKey:OERegionKey] intValue];
	else
    {
		NSLocale *locale = [NSLocale currentLocale];
		
		NSString *countryCode = [locale objectForKey:NSLocaleCountryCode];
        
		if([OERegionCodesEurope containsObject:countryCode])
			region = OERegionEU;
		else if([OERegionCodesNorthAmerica containsObject:countryCode])
			region = OERegionNA;	
		else if([[NSArray arrayWithObjects:@"JP", @"HK", @"TW", nil] containsObject:countryCode])
			region = OERegionJAP;
		else
			region = OERegionOther;
	}
}
@end
