/*
Copyright (C) 2003 Rice1964

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef _SORTED_LIST_H_
#define _SORTED_LIST_H_

#include <cstring>

template<class Key, class Element>
class CSortedList
{
private:
    Key *keys;
    Element *elements;
    int curSize;
    int maxSize;

public:
    CSortedList(int size=1000)
    {
        maxSize = size;
        curSize = 0;
        keys = new Key[size];
        elements = new Element[size];
    }

    ~CSortedList()
    {
        delete [] keys;
        delete [] elements;
    }

    int size()
    {
        return curSize;
    }

    void clear()
    {
        curSize = 0;
    }

    void add(Key key, Element ele)
    {
        int i = find(key);
        if( i >= 0 )
        {
            elements[i] = ele;
            return;
        }

        if( curSize == maxSize )
        {
            // Need to increase maxSize
            Key *oldkeys = keys;
            Element *oldelements = elements;
            int oldmaxsize = maxSize;
            maxSize *= 2;

            keys = new Key[maxSize];
            elements = new Element[maxSize];
            std::memcpy(keys,oldkeys,oldmaxsize*sizeof(Key));
            std::memcpy(elements,oldelements,oldmaxsize*sizeof(Element));
        }

        for( i=0; i<curSize; i++ )
        {
            if( keys[i] > key )
            {
                // Found the spot
                break;
            }
        }

        for( int j=curSize; j>i; j-- )
        {
            keys[j] = keys[j-1];
            elements[j] = elements[j-1];
        }

        keys[i] = key;
        elements[i] = ele;
        curSize++;
    }

    Element operator[](int index)
    {
        if( index >= curSize )
            index = curSize-1;
        else if( index < 0 )
            index = 0;

        return elements[index];
    }

    Element get(Key key)
    {
        int index = Find(key);
        return this->operator[](index);
    }

    // Binary search
    int find(Key key)
    {
        if( curSize <= 0 )
            return -1;

        int dwMin = 0;
        int dwMax = curSize - 1;
        int index = -1;

        int dwRange;
        int dwIndex;

        while (true)
        {
            dwRange = dwMax - dwMin;
            dwIndex = dwMin + (dwRange/2);

            if( keys[dwIndex] == key )
            {
                index = dwIndex;
                break;
            }

            // If the range is 0, and we didn't match above, then it must be unmatched
            if (dwRange == 0) 
                break;

            // If lower, check from min..index
            // If higher, check from index..max
            if (key < keys[dwIndex])
            {
                // Lower
                dwMax = dwIndex;
            }
            else
            {
                // Higher
                dwMin = dwIndex + 1;
            }
        }

        return index;
    }
};

#endif

