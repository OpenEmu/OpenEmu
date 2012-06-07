/* Copyright (C) 1991, 1992, 1997, 1999 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

/* Modified for Mednafen to provide a world-compatible(;)) strtod function,
   that recognizes both "," and "." as valid radix characters.
*/

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <errno.h>
#ifndef errno
extern int errno;
#endif

#include <ctype.h>

#if defined (STDC_HEADERS) || (!defined (isascii) && !defined (HAVE_ISASCII))
# define IN_CTYPE_DOMAIN(c) 1
#else
# define IN_CTYPE_DOMAIN(c) isascii(c)
#endif

#define ISSPACE(c) (IN_CTYPE_DOMAIN (c) && isspace (c))
#define ISDIGIT(c) (IN_CTYPE_DOMAIN (c) && isdigit (c))
#define TOLOWER(c) (IN_CTYPE_DOMAIN (c) ? tolower(c) : (c))

#include <math.h>

#if HAVE_FLOAT_H
# include <float.h>
#else
# define DBL_MAX 1.7976931348623159e+308
# define DBL_MIN 2.2250738585072010e-308
#endif

#if STDC_HEADERS
# include <stdlib.h>
# include <string.h>
#else
# define NULL 0
# ifndef HUGE_VAL
#  define HUGE_VAL HUGE
# endif
#endif

#include "world_strtod.h"

/* Convert NPTR to a double.  If ENDPTR is not NULL, a pointer to the
   character after the last one used in the number is put in *ENDPTR.  */
double world_strtod (const char *nptr, char **endptr)
{
  register const char *s;
  short int sign;

  /* The number so far.  */
  double num;

  int got_dot;			/* Found a decimal point.  */
  int got_digit;		/* Seen any digits.  */

  /* The exponent of the number.  */
  long int exponent;

  if (nptr == NULL)
    {
      errno = EINVAL;
      goto noconv;
    }

  s = nptr;

  /* Eat whitespace.  */
  while (ISSPACE (*s))
    ++s;

  /* Get the sign.  */
  sign = *s == '-' ? -1 : 1;
  if (*s == '-' || *s == '+')
    ++s;

  num = 0.0;
  got_dot = 0;
  got_digit = 0;
  exponent = 0;
  for (;; ++s)
    {
      if (ISDIGIT (*s))
	{
	  got_digit = 1;

	  /* Make sure that multiplication by 10 will not overflow.  */
	  if (num > DBL_MAX * 0.1)
	    /* The value of the digit doesn't matter, since we have already
	       gotten as many digits as can be represented in a `double'.
	       This doesn't necessarily mean the result will overflow.
	       The exponent may reduce it to within range.

	       We just need to record that there was another
	       digit so that we can multiply by 10 later.  */
	    ++exponent;
	  else
	    num = (num * 10.0) + (*s - '0');

	  /* Keep track of the number of digits after the decimal point.
	     If we just divided by 10 here, we would lose precision.  */
	  if (got_dot)
	    --exponent;
	}
      else if (!got_dot && (*s == '.' || *s == ','))
	/* Record that we have found the decimal point.  */
	got_dot = 1;
      else
	/* Any other character terminates the number.  */
	break;
    }

  if (!got_digit)
    goto noconv;

  if (TOLOWER (*s) == 'e')
    {
      /* Get the exponent specified after the `e' or `E'.  */
      int save = errno;
      char *end;
      long int exp;

      errno = 0;
      ++s;
      exp = strtol (s, &end, 10);
      if (errno == ERANGE)
	{
	  /* The exponent overflowed a `long int'.  It is probably a safe
	     assumption that an exponent that cannot be represented by
	     a `long int' exceeds the limits of a `double'.  */
	  if (endptr != NULL)
	    *endptr = end;
	  if (exp < 0)
	    goto underflow;
	  else
	    goto overflow;
	}
      else if (end == s)
	/* There was no exponent.  Reset END to point to
	   the 'e' or 'E', so *ENDPTR will be set there.  */
	end = (char *) s - 1;
      errno = save;
      s = end;
      exponent += exp;
    }

  if (endptr != NULL)
    *endptr = (char *) s;

  if (num == 0.0)
    return 0.0;

  /* Multiply NUM by 10 to the EXPONENT power,
     checking for overflow and underflow.  */

  if (exponent < 0)
    {
      if (num < DBL_MIN * pow (10.0, (double) -exponent))
	goto underflow;
    }
  else if (exponent > 0)
    {
      if (num > DBL_MAX * pow (10.0, (double) -exponent))
	goto overflow;
    }

  num *= pow (10.0, (double) exponent);

  return num * sign;

overflow:
  /* Return an overflow error.  */
  errno = ERANGE;
  return HUGE_VAL * sign;

underflow:
  /* Return an underflow error.  */
  if (endptr != NULL)
    *endptr = (char *) nptr;
  errno = ERANGE;
  return 0.0;

noconv:
  /* There was no number.  */
  if (endptr != NULL)
    *endptr = (char *) nptr;
  return 0.0;
}
