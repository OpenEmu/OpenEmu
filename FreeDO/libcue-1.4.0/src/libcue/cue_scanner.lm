%{
/*
 * Copyright (c) 2004, 2005, 2006, 2007, Svend Sorensen
 * Copyright (c) 2009, 2010 Jochen Keil
 * For license terms, see the file COPYING in this distribution.
 */

#include <stdlib.h>
#include <string.h>

#include "config.h"

#include "cd.h"
#include "cue_parser.h"

char buffer[PARSER_BUFFER];

int yylex(void);
%}

ws		[ \t\r]
nonws		[^ \t\r\n]

%option yylineno
%option noyywrap
%option noinput
%option nounput

%s NAME
%x REM
%x RPG
%x SKIP

%%

\'([^\']|\\\')*\'	|
\"([^\"]|\\\")*\"	{
		yylval.sval = strncpy(	buffer,
					++yytext,
					(yyleng > sizeof(buffer) ? sizeof(buffer) : yyleng));
		yylval.sval[yyleng - 2] = '\0';
		BEGIN(INITIAL);
		return STRING;
		}

<NAME>{nonws}+	{
		yylval.sval = strncpy(	buffer,
					yytext,
					(yyleng > sizeof(buffer) ? sizeof(buffer) : yyleng));
		yylval.sval[yyleng] = '\0';
		BEGIN(INITIAL);
		return STRING;
		}

CATALOG		{ BEGIN(NAME); return CATALOG; }
CDTEXTFILE	{ BEGIN(NAME); return CDTEXTFILE; }

FILE		{ BEGIN(NAME); return FFILE; }
BINARY		{ return BINARY; }
MOTOROLA	{ return MOTOROLA; }
AIFF		{ return AIFF; }
WAVE		{ return WAVE; }
MP3		{ return MP3; }

TRACK		{ return TRACK; }
AUDIO		{ yylval.ival = MODE_AUDIO; return AUDIO; }
MODE1\/2048	{ yylval.ival = MODE_MODE1; return MODE1_2048; }
MODE1\/2352	{ yylval.ival = MODE_MODE1_RAW; return MODE1_2352; }
MODE2\/2336	{ yylval.ival = MODE_MODE2; return MODE2_2336; }
MODE2\/2048	{ yylval.ival = MODE_MODE2_FORM1; return MODE2_2048; }
MODE2\/2342	{ yylval.ival = MODE_MODE2_FORM2; return MODE2_2342; }
MODE2\/2332	{ yylval.ival = MODE_MODE2_FORM_MIX; return MODE2_2332; }
MODE2\/2352	{ yylval.ival = MODE_MODE2_RAW; return MODE2_2352; }

FLAGS		{ return FLAGS; }
PRE		{ yylval.ival = FLAG_PRE_EMPHASIS; return PRE; }
DCP		{ yylval.ival = FLAG_COPY_PERMITTED; return DCP; }
4CH		{ yylval.ival = FLAG_FOUR_CHANNEL; return FOUR_CH; }
SCMS		{ yylval.ival = FLAG_SCMS; return SCMS; }

PREGAP		{ return PREGAP; }
INDEX		{ return INDEX; }
POSTGAP		{ return POSTGAP; }

TITLE		{ BEGIN(NAME); yylval.ival = PTI_TITLE;  return TITLE; }
PERFORMER	{ BEGIN(NAME); yylval.ival = PTI_PERFORMER;  return PERFORMER; }
SONGWRITER	{ BEGIN(NAME); yylval.ival = PTI_SONGWRITER;  return SONGWRITER; }
COMPOSER	{ BEGIN(NAME); yylval.ival = PTI_COMPOSER;  return COMPOSER; }
ARRANGER	{ BEGIN(NAME); yylval.ival = PTI_ARRANGER;  return ARRANGER; }
MESSAGE		{ BEGIN(NAME); yylval.ival = PTI_MESSAGE;  return MESSAGE; }
DISC_ID		{ BEGIN(NAME); yylval.ival = PTI_DISC_ID;  return DISC_ID; }
GENRE		{ BEGIN(NAME); yylval.ival = PTI_GENRE;  return GENRE; }
TOC_INFO1	{ BEGIN(NAME); yylval.ival = PTI_TOC_INFO1;  return TOC_INFO1; }
TOC_INFO2	{ BEGIN(NAME); yylval.ival = PTI_TOC_INFO2;  return TOC_INFO2; }
UPC_EAN		{ BEGIN(NAME); yylval.ival = PTI_UPC_ISRC;  return UPC_EAN; }
ISRC/{ws}+\"	{ BEGIN(NAME); yylval.ival = PTI_UPC_ISRC;  return ISRC; }
SIZE_INFO	{ BEGIN(NAME); yylval.ival = PTI_SIZE_INFO;  return SIZE_INFO; }

ISRC		{ BEGIN(NAME); return TRACK_ISRC; }

REM		{ BEGIN(REM); /* exclusive rules for special exceptions */ }

<REM>DATE			{ BEGIN(NAME); yylval.ival = REM_DATE; return DATE; }
<REM>REPLAYGAIN_ALBUM_GAIN 	{ BEGIN(RPG); yylval.ival = REM_REPLAYGAIN_ALBUM_GAIN;
							return REPLAYGAIN_ALBUM_GAIN; }
<REM>REPLAYGAIN_ALBUM_PEAK	{ BEGIN(RPG); yylval.ival = REM_REPLAYGAIN_ALBUM_PEAK;
							return REPLAYGAIN_ALBUM_PEAK; }
<REM>REPLAYGAIN_TRACK_GAIN	{ BEGIN(RPG); yylval.ival = REM_REPLAYGAIN_TRACK_GAIN;
							return REPLAYGAIN_TRACK_GAIN; }
<REM>REPLAYGAIN_TRACK_PEAK	{ BEGIN(RPG); yylval.ival = REM_REPLAYGAIN_TRACK_PEAK;
							return REPLAYGAIN_TRACK_PEAK; }

<REM>{ws}+	{ BEGIN(REM); }
<REM>.		{ BEGIN(REM); }
<REM>\n		{ BEGIN(INITIAL); }

<RPG>{nonws}+	{
		yylval.sval = strncpy(	buffer,
					yytext,
					(yyleng > sizeof(buffer) ? sizeof(buffer) : yyleng));
		yylval.sval[yyleng] = '\0';
		BEGIN(SKIP);
		return STRING;
		}

<RPG>{ws}+	{ BEGIN(RPG); }

<SKIP>.*\n	{ BEGIN(INITIAL); yylineno++; return '\n'; }

{ws}+		{ /* ignore whitespace */ }

[[:digit:]]+	{ yylval.ival = atoi(yytext); return NUMBER; }
:		{ return yytext[0]; }

^{ws}*\n	{ yylineno++; /* blank line */ }
\n		{ yylineno++; return '\n'; }
.		{ fprintf(stderr, "bad character '%c'\n", yytext[0]); }

%%
