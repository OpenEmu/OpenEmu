/*	------------------------------------------------------------------------------------------------
	Description:	This module will add commas to a number and return it as TCHAR string
	Author:			CaptainCPS-X
	Last update:	Sep 18, 2008
	Originally for:	FB alpha emulator UI features
	Status:			Positive and negative numbers are formatted with commas, so it's very complete
	TODO:			Optimize string management in the function.
	------------------------------------------------------------------------------------------------ */
#include "burner.h"

#define MAX_DIGITS	32

TCHAR* FormatCommasNumber(__int64 nNum) 
{
	// Mixed Math & C++ Formula:

	// nNum		= the big ass number
	// S		= the unformatted string containing the big ass number without commas
	// L		= string length / number of digits
	// 3.0f		= constant position of the commas
	// R		= main result, if this number is a decimal, this is the number of commas
	// RS		= string that will receive the result number as text for later comparison
	// SC1		= comparison string 1 (see the formula / function for details)
	// SC2		= comparison string 2 (see the formula / function for details)
	// SC3		= comparison string 3 (see the formula / function for details)
	// ISDEC1	= int acting as boolean
	// ISDEC2	= int acting as boolean
	// ISINT	= int acting as boolean
	
	// ... other variable definitions should be easy to understand

	TCHAR				szFinalText[256]	= _T("");
	TCHAR				S[MAX_PATH]			= _T("");
	TCHAR				RS[MAX_PATH]		= _T("");
	TCHAR				SC1[MAX_PATH]		= _T("");
	TCHAR				SC2[MAX_PATH]		= _T("");
	TCHAR				SC3[MAX_PATH]		= _T("");
	unsigned int		L					= 0;
	float				R					= 0.0f;
	int					ISDEC1				= 0;
	int					ISDEC2				= 0;
	int					ISINT				= 0;
	int					nCompleteLength		= 0;
	int					nCommaPos			= 0;
	int					nFirstComma			= 0;
	bool				bIsNegative			= false;

	if(nNum < 0) bIsNegative = true;					// Is a negative number

	_stprintf( S, _T("%I64d"), nNum);

	if(bIsNegative == true) {
		TCHAR* szTemp = NULL;
		szTemp = S;
		_stprintf( S, _T("%s"), szTemp + 1);
	}
	
	L	= _tcslen(S);	
	R	= (L / 3.0f);

	_stprintf( RS ,		_T("%1.1f")	, R			);		// x.x			(could be a Integer 'x.0' or one of this decimal fractions 'x.7' , 'x.3')
	_stprintf( SC1,		_T("%i.7")	, (L / 3)	);		// x.6 [~x.7]	(simulate the Decimal result xxx.6 for later comparison)
	_stprintf( SC2,		_T("%i.3")	, (L / 3)	);		// x.3			(simulate the Decimal result xxx.3 for later comparison)
	_stprintf( SC3,		_T("%i.0")	, (L / 3)	);		// x.0			(simulate the Integer result xxx.0 for later comparison)

	ISDEC1	= _tcscmp( RS, SC1 );
	ISDEC2	= _tcscmp( RS, SC2 );
	ISINT	= _tcscmp( RS, SC3 );

	if(ISDEC1 != 0 && ISDEC2 != 0 && ISINT == 0) {
		R = R - 1.0f;
	}

	nCompleteLength = L + (int)R;						// Number of digits + number of commas

	if(ISDEC1 ==  0 && ISDEC2 ==  1) nFirstComma = 2;	// 0.6 [~0.7]	(.6 / .3 = 2) (place of the first comma from left to right)	
	if(ISDEC1 == -1 && ISDEC2 ==  0) nFirstComma = 1;	// 0.3 [~0.4]	(.3 / .3 = 1) (place of the first comma from left to right)
	if(ISDEC1 == -1 && ISDEC2 == -1) nFirstComma = 0;	// x.0			(.0 / .3 = 0) (no place for a initial comma so start adding after 3 digits from left to right)

	TCHAR szTmp[2]					= _T("");
	TCHAR szNumber[MAX_DIGITS]		= _T("");
	TCHAR* pszTxt					= NULL;
	TCHAR szTxt2[MAX_DIGITS][2];						// array of digits characters
	TCHAR szCommaString[MAX_DIGITS][2];
	int nCharStr					= 0;

	_tcsncpy(szNumber, S, nCompleteLength);

	for(unsigned int nChar = 0; nChar < L; nChar++) {
		pszTxt = szNumber;
		_tcsncpy(szTxt2[nChar], pszTxt + nChar, 1);
	}

	for(int nChar = 0; nChar < nCompleteLength; nChar++) {
		// ADD COMMA AFTER 3 DIGITS
		if(nFirstComma == 0 && nCommaPos == 3) {
			// ADD COMMA
			_tcsncpy(szCommaString[nCharStr], _T(","), 1);
			// ADD NEXT NUMBER WAITING ON MEMORY AND CONTINUE LOOP
			nCharStr	= nCharStr + 1;
			_tcsncpy(szCommaString[nCharStr], szTxt2[nChar], 1);		
			nCommaPos	= 1;
			nCharStr	= nCharStr	+ 1;
		} else {
			//
			if((nFirstComma != 1 && nFirstComma != 2)) {
				// NO COMMAS SO ADD NUMBER...
				_tcsncpy(szCommaString[nCharStr], szTxt2[nChar], 1);
			} else {
				// ADD COMMA AFTER 1 DIGIT
				if(nFirstComma == 1 && nCommaPos == 1) {
					_tcsncpy(szCommaString[nCharStr], _T(","), 1);
					nFirstComma = 0;
					nCommaPos	= 0;
					nCharStr	= nCharStr + 1;
				}
				// ADD COMMA AFTER 2 DIGITS
				if(nFirstComma == 2 && nCommaPos == 2) {
					_tcsncpy(szCommaString[nCharStr], _T(","), 1);
					nFirstComma = 0;
					nCommaPos	= 0;
					nCharStr	= nCharStr + 1;
				}				
				// ADD NEXT NUMBER WAITING ON MEMORY AND CONTINUE LOOP
				_tcsncpy(szCommaString[nCharStr], szTxt2[nChar], 1);
			}
			nCommaPos	= nCommaPos + 1;
			nCharStr	= nCharStr	+ 1;
		}
	}

	_stprintf(szFinalText, _T("%s"), _T(""));

	for(int nChar = 0; nChar < nCompleteLength; nChar++) {
		_tcsncpy(szTmp, szCommaString[nChar], 1);
		_stprintf(szFinalText, _T("%s%s"), szFinalText, szTmp); // x + tmp 
																// x + x + tmp
																// x + x + x + tmp ...
	}

	TCHAR szNegative[1024] = _T("");
	if(bIsNegative == true) {
		_stprintf(szNegative, _T("%s%s"), _T("-"), szFinalText);				// add negative sign
	}

	TCHAR* pszBuffer = NULL;
	if(bIsNegative == true) {
		pszBuffer = szNegative;
	} else {
		pszBuffer = szFinalText;
	}
	return (TCHAR*)pszBuffer;

	//	--------------------------------------------------------------------------------------------------------------------------------------
	//	NOTE!!!!: The following comments are personal notes I wrote here so I could keep up with my research
	//	it is not intended to be readed as standard C / C++, just see this as steps of various operations and calculations.
	//	--------------------------------------------------------------------------------------------------------------------------------------
	
	//	These are written test results I made before doing the formula ...
	/*

		[ ]				= operation or information holder
		->				= next operation step
		x				= any number
		nPreResult		= preliminary result
		Int				= this means that the preliminary result is a Integer			(Ex. 1.0)
		Dec				= this means that the preliminary result is a Decimal Fraction	(Ex. 1.3)
		nCommas			= final number of commas the number will have
	
		xxx,xxx,xxx,xxx	[ 12 digits ] -> [ nPreResult = (12 / 3.0f) = 4.0] -> [ Int ] -> [ nCommas = nPreResult - 1 ]	-> [ 3 commas ]
		xx,xxx,xxx,xxx	[ 11 digits ] -> [ nPreResult = (11 / 3.0f) = 3.6] -> [ Dec ] -> [ nCommas = 3 ]				-> [ 3 commas ]
		x,xxx,xxx,xxx	[ 10 digits ] -> [ nPreResult = (10 / 3.0f) = 3.3] -> [ Dec ] -> [ nCommas = 3 ]				-> [ 3 commas ]
		xxx,xxx,xxx		[  9 digits ] -> [ nPreResult = ( 9 / 3.0f) = 3.0] -> [ Int ] -> [ nCommas = nPreResult - 1 ]	-> [ 2 commas ]
		xx,xxx,xxx		[  8 digits ] -> [ nPreResult = ( 8 / 3.0f) = 2.6] -> [ Dec ] -> [ nCommas = 2 ]				-> [ 2 commas ]
		x,xxx,xxx		[  7 digits ] -> [ nPreResult = ( 7 / 3.0f) = 2.3] -> [ Dec ] -> [ nCommas = 2 ]				-> [ 2 commas ]
		xxx,xxx			[  6 digits ] -> [ nPreResult = ( 6 / 3.0f) = 2.0] -> [ Int ] -> [ nCommas = nPreResult - 1 ]	-> [ 1 comma  ]
		xx,xxx			[  5 digits ] -> [ nPreResult = ( 5 / 3.0f) = 1.6] -> [ Dec ] -> [ nCommas = 1 ]				-> [ 1 comma  ]
		x,xxx			[  4 digits ] -> [ nPreResult = ( 4 / 3.0f) = 1.3] -> [ Dec ] -> [ nCommas = 1 ]				-> [ 1 comma  ]
		xxx				[  3 digits ] -> [ nPreResult = ( 3 / 3.0f) = 1.0] -> [ Int ] -> [ nCommas = nPreResult - 1 ]	-> [ 0 commas ]
		xx				[  2 digits ] -> [ nPreResult = ( 2 / 3.0f) = 0.6] -> [ Dec ] -> [ nCommas = 0 ]				-> [ 0 commas ]
		x				[  1 digits ] -> [ nPreResult = ( 1 / 3.0f) = 0.3] -> [ Dec ] -> [ nCommas = 0 ]				-> [ 0 commas ]
	
		Number of Commas:

			The nCommas comes from the integer value of the decimal fraction, Example:
			
			If nPreResult is 1.6 , the final number of commas is 1 or 1.0, because where are not using the fraction value of 0.6

		Placing Commas:
			
			Placing commas from left to right is made like from right to left, BUT before starting to count 3 digits and add
			a comma we must complete one operation, this is of course if you wanna end with the commas in the right place =).
			
			Using the previous example, to start placing commas we have to use the decimal fraction value of 0.6. 
			We gonna divide (0.6 / 0.3) or (6 / 3) to get the number of	digits at the left of the first comma, for
			example:

			Lets say we wanna get the quantity of commas this number (10000) should have and we wanna place them from left to right
			following my formula, and not the regular right to left method:

			First, count the digits:

			10000 (5 digits)
			
			Second, divide the number of digits with 3

			5 / 3.0 = 1.6

			Third, because the result is a decimal fraction we will not do the substraction of 1 and
			we will separate each part of the result

			1.0 (number of commas)
			0.6 (fraction piece we must use to know the number of digits at the left of the first comma)

			Fourth, divide the fraction number with 0.3

			0.6 / 0.3 = 2 (there will be 2 digits at the left of the first comma)

			Finally!, lets put the numbers from LEFT to RIGHT using the information we have ^^

			nCommas	=	1 ( number of commas)
			nDigits	=	2 (left digits before the first comma)

			nDigits	(	1	2	)
						|	|
						1	0	,	0	0	0
								|
							(	1	)	nCommas

			The comma position operation rule applies only when we get a decimal fraction as a preliminar result,
			if we have a integer as a preliminar result then there will be 3 digits at the left of
			the first comma, for example:

			100000 (6 digits)

			6 / 3.0 = 2.0	(is integer lets follow the rule and substract 1)

			2.0 - 1 = 1.0	(final number of commas)

			nCommas = 1
			nDigits = 3		(here the number of digits at the left of the first comma will be 3)

			nDigits	(	1	2	3)
						|	|	|
						1	0	0	,	0	0	0
									|
								(	1	)	nCommas
			
	*/
	// --------------------------------------------------------------------------------------------------------------------------------------
}
