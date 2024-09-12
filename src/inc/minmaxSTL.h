#pragma once

#ifdef min
	#undef min
#endif
#ifdef max
	#undef max
#endif

template <class T>
const T& minT(const T& a,const T& b) 
{
	return a<b ? a : b;
}

template <class T>
const T& maxT(const T& a,const T& b) 
{
	return a>b ? a : b;
}

template <class T>
const T& rangeT(const T& a,const T& lo,const T& hi) 
{
	if(a < lo) return lo;
	if(a > hi) return hi;
	
	return a;
}

template <class T>
inline void swap(T& a, T& b)
{
	T d = a;
	a = b;
	b = d;
}

#define min(a,b) minT(a,b)
#define max(a,b) maxT(a,b)


