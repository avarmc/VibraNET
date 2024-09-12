#ifndef _vMMX_h_
#define _vMMX_h_

#include "stdafx.h"
#include <mmintrin.h>
#include <xmmintrin.h>

#define MMX_READY __declspec(align(16))



inline void SSEmemcpyU(void * to, const void * from, size_t sz)
{
	char *ps = (char *)from;
	char *pt = (char *)to;
	
	size_t szA = (sz&(~15)),szU = sz-szA;

	char *pe = ps + szA;

	__m128 v;
	while(ps != pe)
	{
		v = _mm_loadu_ps((float *)ps);

		_mm_storeu_ps((float *)pt,v);

		ps += 16;
		pt += 16;
	}


	if( szU )
		memcpy(pt,ps,szU);
}

inline void SSEmemcpy(void * to, const void * from, size_t sz)
{
	bool bTo = (((uintptr_t)to)&15) ? true:false;
	bool bFrom = (((uintptr_t)from)&15) ? true:false;
	if( !bTo && !bFrom )
	{
		char *ps = (char *)from;
		size_t szA = (sz&(~15)),szU = sz-szA;

		char *pt = (char *)to;
		char *pe = ps + szA;

		__m128 v;
		while(ps != pe)
		{
			v = _mm_load_ps((float *)ps);

			_mm_store_ps((float *)pt,v);

			ps += 16;
			pt += 16;
		}

		
		if( szU )
			memcpy(pt,ps,szU);
	} else
	{
		SSEmemcpyU(to,from,sz);
	}
}


inline void SSEmemsetU(void *to,int sv,size_t sz)
{
//	memset(to,(int)sv,sz);
//	return;

	size_t szA = (sz&(~15)),szU = sz-szA;

	char *ps = (char *)to;
	char *pe = ps + szA;

	__m128 v = _mm_load_ps1((float *)&sv);

	while(ps != pe)
	{
		_mm_storeu_ps((float *)ps,v);
		ps += 16;
	}


	
	if( szU )
		memset(ps,sv,szU);
}

inline void SSEmemset(void *to,int sv,size_t sz)
{
	if( !(((uintptr_t)to)&15) )
	{
		size_t szA = (sz&(~15)),szU = sz-szA;

		char *ps = (char *)to;
		char *pe = ps + szA;

		__m128 v = _mm_load_ps1((float *)&sv);

		while(ps != pe)
		{
			_mm_store_ps((float *)ps,v);
			ps += 16;
		}



		if( szU )
			memset(ps,sv,szU);
	} else
		SSEmemsetU(to,sv,sz);
}

#define SSESafeMemset(to,v,sz) SSEmemset(to,v,sz)
#define SSESafeMemcpy(to,from,sz) SSEmemcpy(to,from,sz)

#define MMXalloc(s) _aligned_malloc(((s+31)&(~31)),32)
#define MMXfree(s) _aligned_free(s)

#endif	// _vMMX_h_