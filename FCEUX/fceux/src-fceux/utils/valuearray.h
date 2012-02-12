#ifndef _VALUEARRAY_H_
#define _VALUEARRAY_H_

template<typename T, int N>
struct ValueArray
{
	T data[N];
	T &operator[](int index) { return data[index]; }
	static const int size = N;
	bool operator!=(ValueArray<T,N> &other) { return !operator==(other); }
	bool operator==(ValueArray<T,N> &other)
	{
		for(int i=0;i<size;i++)
			if(data[i] != other[i])
				return false;
		return true;
	}
};

#endif

