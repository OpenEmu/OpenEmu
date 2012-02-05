/*
 * Copyright 2011 X-Scale. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright 
 *       notice, this list of conditions and the following disclaimer in the 
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY X-Scale ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO 
 * EVENT SHALL X-Scale OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are 
 * those of the authors and should not be interpreted as representing official 
 * policies, either expressed or implied, of X-Scale.
 *
 * This software provides an algorithm that emulates the protection scheme of 
 * N64 PIF/CIC-NUS-6105, by determining the proper response to each challenge. 
 * It was synthesized after a careful, exhaustive and detailed analysis of the 
 * challenge/response pairs stored in the 'pif2.dat' file from Project 64. 
 * These challenge/response pairs were the only resource used during this 
 * project. There was no kind of physical access to N64 hardware.
 *
 * This project would have never been possible without the contribuitions of 
 * the following individuals and organizations:
 *
 * - Oman: For being at the right place at the right time and being brave 
 *       enough to pay a personal price so we could understand in a much deeper
 *       way how this magical console really works. We owe you so much.
 *
 * - Jovis: For all the positive energy and impressive hacking spirit that you
 *       shared with the N64 community. You were absolutely instrumental in 
 *       several key events that shaped the N64 community in the last 14 years.
 *       Even if you're not physically with us anymore, your heritage, your 
 *       knowledge and your attitude will never be forgotten. 
 *
 *          'The candle that burns twice as bright burns half as long.'
 *
 * - LaC: For the endless contributions that you've given to the N64 community 
 *       since the early days, when N64 was the next big thing. I've always 
 *       admired the deep knowledge that you've gathered about the most little 
 *       hardware details. Recently, you challanged us to find a small and 
 *       concise algorithm that would emulate the behaviour of CIC-NUS-6105
 *       challenge/response protection scheme and here is the final result. 
 *       LaC, Oman and Jovis were definitly the dream team of N64 reversing in 
 *       the late 90's. Without your contributions, we would be much poorer.
 *
 * - marshall: For keeping the N64 scene alive during the last decade, when 
 *       most people lost interest and moved along to different projects. You 
 *       are the force that has been keeping us all together in the later 
 *       years. When almost nobody cared about N64 anymore, you were always 
 *       there, spreading the word, developing in the console, and lately, 
 *       making impressive advances on the hardware side. I wish the best 
 *       success to your new 64drive project. 
 *
 * - hcs: For your contributions to the better understanding of the inner 
 *       workings of the Reality Co-Processor (RCP). Your skills have impressed
 *       me for a long time now. And without your precious help by sharing your
 *       kownledge, I would have never understood the immense importance of 
 *       Oman, Jovis and LaC achievements. Thank you !
 *
 * - Azimer & Tooie: For sharing with the N64 community your findings about the
 *       challenge/response pair used in 'Jet Force Gemini' and the 267 
 *       challenge/response pairs used in 'Banjo Tooie', all stored in the 
 *       'pif2.dat' file of Project 64. They were instrumental to the final
 *       success of this endeavour.
 *
 * - Silicon Graphics, Inc. (SGI): For creating MIPS R4000, MIPS R4300 and 
 *       Reality Co-Processor (RCP). You were the ultimate dream creator during
 *       the late 80's and early 90's. A very special word of gratitude goes to
 *       the two teams that during those years created RCP and MIPS R4300. They
 *       were technological breakthroughs back then.
 * 
 * On a personal note, I would like to show my deepest gratitude to _Bijou_, 
 * for being always a source of endless hope and inspiration. 
 *
 *   -= X-Scale =- (#n64dev@EFnet)
 */

#include "n64_cic_nus_6105.h"

void n64_cic_nus_6105(char chl[], char rsp[], int len)
{
    static char lut0[0x10] = {
        0x4, 0x7, 0xA, 0x7, 0xE, 0x5, 0xE, 0x1, 
        0xC, 0xF, 0x8, 0xF, 0x6, 0x3, 0x6, 0x9
    };
    static char lut1[0x10] = {
        0x4, 0x1, 0xA, 0x7, 0xE, 0x5, 0xE, 0x1, 
        0xC, 0x9, 0x8, 0x5, 0x6, 0x3, 0xC, 0x9
    };
    char key, *lut;
    int i, sgn, mag, mod;
        
    for (key = 0xB, lut = lut0, i = 0; i < len; i++) {
        rsp[i] = (key + 5 * chl[i]) & 0xF;
        key = lut[(int) rsp[i]];
        sgn = (rsp[i] >> 3) & 0x1;
        mag = ((sgn == 1) ? ~rsp[i] : rsp[i]) & 0x7;
        mod = (mag % 3 == 1) ? sgn : 1 - sgn;
        if (lut == lut1 && (rsp[i] == 0x1 || rsp[i] == 0x9))
            mod = 1;
        if (lut == lut1 && (rsp[i] == 0xB || rsp[i] == 0xE))
            mod = 0;
        lut = (mod == 1) ? lut1 : lut0;
    }
}

