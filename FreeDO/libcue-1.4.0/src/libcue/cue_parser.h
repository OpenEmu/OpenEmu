
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     NUMBER = 258,
     STRING = 259,
     CATALOG = 260,
     CDTEXTFILE = 261,
     FFILE = 262,
     BINARY = 263,
     MOTOROLA = 264,
     AIFF = 265,
     WAVE = 266,
     MP3 = 267,
     TRACK = 268,
     AUDIO = 269,
     MODE1_2048 = 270,
     MODE1_2352 = 271,
     MODE2_2336 = 272,
     MODE2_2048 = 273,
     MODE2_2342 = 274,
     MODE2_2332 = 275,
     MODE2_2352 = 276,
     TRACK_ISRC = 277,
     FLAGS = 278,
     PRE = 279,
     DCP = 280,
     FOUR_CH = 281,
     SCMS = 282,
     PREGAP = 283,
     INDEX = 284,
     POSTGAP = 285,
     TITLE = 286,
     PERFORMER = 287,
     SONGWRITER = 288,
     COMPOSER = 289,
     ARRANGER = 290,
     MESSAGE = 291,
     DISC_ID = 292,
     GENRE = 293,
     TOC_INFO1 = 294,
     TOC_INFO2 = 295,
     UPC_EAN = 296,
     ISRC = 297,
     SIZE_INFO = 298,
     DATE = 299,
     REPLAYGAIN_ALBUM_GAIN = 300,
     REPLAYGAIN_ALBUM_PEAK = 301,
     REPLAYGAIN_TRACK_GAIN = 302,
     REPLAYGAIN_TRACK_PEAK = 303
   };
#endif
/* Tokens.  */
#define NUMBER 258
#define STRING 259
#define CATALOG 260
#define CDTEXTFILE 261
#define FFILE 262
#define BINARY 263
#define MOTOROLA 264
#define AIFF 265
#define WAVE 266
#define MP3 267
#define TRACK 268
#define AUDIO 269
#define MODE1_2048 270
#define MODE1_2352 271
#define MODE2_2336 272
#define MODE2_2048 273
#define MODE2_2342 274
#define MODE2_2332 275
#define MODE2_2352 276
#define TRACK_ISRC 277
#define FLAGS 278
#define PRE 279
#define DCP 280
#define FOUR_CH 281
#define SCMS 282
#define PREGAP 283
#define INDEX 284
#define POSTGAP 285
#define TITLE 286
#define PERFORMER 287
#define SONGWRITER 288
#define COMPOSER 289
#define ARRANGER 290
#define MESSAGE 291
#define DISC_ID 292
#define GENRE 293
#define TOC_INFO1 294
#define TOC_INFO2 295
#define UPC_EAN 296
#define ISRC 297
#define SIZE_INFO 298
#define DATE 299
#define REPLAYGAIN_ALBUM_GAIN 300
#define REPLAYGAIN_ALBUM_PEAK 301
#define REPLAYGAIN_TRACK_GAIN 302
#define REPLAYGAIN_TRACK_PEAK 303




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 1676 of yacc.c  */
#line 59 "cue_parser.y"

	long ival;
	char *sval;



/* Line 1676 of yacc.c  */
#line 155 "cue_parser.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;


