/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - helpfunc.h                                              *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2008 Marshallh                                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Reimplemented standard math functions. */

float pow(float inp, int to);
float atan(float inp);
float fabs(float inp);
float sgn(float inp);
float ceil(float inp);

/* Returns the arctangent, if given angle input is between -1 and 1 radians.
 * Uses an approximation of the Taylor series. Warning, slow.
 */
float atan(float inp)
{
    float result = inp;
    int addsub = 0;
    int i = 0;

    for(i=3; i<17; i+=2)
        {
        if(addsub == 0)
            {
            result -= pow(inp,i)/i;
            addsub = 1;
            }
        else
            {
            result += pow(inp,i)/i;
            addsub = 0;
            }
        }

    return result;
}

/* Take a base inp to a (whole number) power to. */
float pow(float inp, int to)
{
    float result = inp;
    int i = 0;

    for(i = 1; i<to; ++i)
        result *= inp;

    return result;
}

/* Returns -1 if inp is negative or 1 if inp is positive. */
float sgn(float inp)
{
    if(inp > 0)
        return 1.0;
    if(inp < 0)
        return -1.0;
    if(inp == 0)
        return 0.0;
}

/* Returns absolute value. |inp| */
float fabs(float inp)
{
    if (inp < 0)
        return -inp;
    else
        return inp;
}

/* Rounds up to the next whole number. */
float ceil(float inp)
{
    int temp = 0;
    temp = (int)inp;
    if ((float)temp > inp)
        return (float)temp;
    else
        return (float)temp+1;
}

