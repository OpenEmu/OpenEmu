//
//  OERegonizer.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 06.10.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OELocalizationHelper.h"

#ifndef UDRegionKey
#define UDRegionKey @"region"		
#endif
@interface OELocalizationHelper (Private)
- (void)_updateRegion;
@end

#define OERegionNALocales [NSArray arrayWithObjects:@"", nil]


@implementation OELocalizationHelper
static OELocalizationHelper *sharedHelper;

+ (OELocalizationHelper*)sharedHelper
{
	return sharedHelper ?: [[self new] autorelease];
}

- (id)init
{
	if(sharedHelper)
	{
		[self release];
	}
	else if((self = sharedHelper = [[super init] retain]))
	{
		[self _updateRegion];
	}
	return sharedHelper;
}

- (void)dealloc {
    [super dealloc];
}

#pragma mark -
- (BOOL)isRegionNA{
	return region == OERegionNA;
}
- (BOOL)isRegionEU{
	return region == OERegionEU;
}
- (BOOL)isRegionJAP{
	return region == OERegionJAP;
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
- (void)_updateRegion{
	NSUserDefaults *standardUserDefaults = [NSUserDefaults standardUserDefaults];
	if([standardUserDefaults valueForKey:UDRegionKey]){
		region = [[standardUserDefaults valueForKey:UDRegionKey] intValue];
	} else {
		NSLocale *locale = [NSLocale currentLocale];
		
		NSString *countryCode = [locale objectForKey:NSLocaleCountryCode];
		if([OERegionCodesEurope containsObject:countryCode]){
			region = OERegionEU;
		} else if([OERegionCodesNorthAmerica containsObject:countryCode]){
			region = OERegionNA;	
		} else if([[NSArray arrayWithObjects:@"JP", @"HK", @"TW", nil] containsObject:countryCode]){
			region = OERegionJAP;
		} else {
			region = OERegionOther;
		}
	}
}
@end
