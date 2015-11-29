/*
 Copyright (c) 2015, OpenEmu Team

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

import Foundation

extension NSDate {
    
    func OE_compareDMYTranslatingNilToDistantPast(otherDate: NSDate?) -> NSComparisonResult {
        
        guard let otherDate = otherDate else {
            return compare(NSDate.distantPast())
        }
        
        let gregorian = NSCalendar(calendarIdentifier: NSCalendarIdentifierGregorian)!
        
        let selfDMY  = gregorian.components([.Day, .Month, .Year], fromDate: self)
        let otherDMY = gregorian.components([.Day, .Month, .Year], fromDate: otherDate)
        
        if selfDMY.year != otherDMY.year {
            return selfDMY.year > otherDMY.year ? .OrderedDescending : .OrderedAscending
        }
        if selfDMY.month != otherDMY.month {
            return selfDMY.month > otherDMY.month ? .OrderedDescending : .OrderedAscending
        }
        if selfDMY.day != otherDMY.day {
            return selfDMY.day > otherDMY.day ? .OrderedDescending : .OrderedAscending
        }
        
        return .OrderedSame
    }
}
