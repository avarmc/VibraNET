#pragma once

#include <vector>
#include <list>

#include "vmmx.h"
// Note: This file/project expects to run in an environment
// where no float operations are being done. Therefore, we
// don't require resetting the registers states with an EMMS.
// So we disable the warning that tells us we haven't used
// an EMMS instruction
#pragma warning(disable: 4799)

#ifdef _DEBUG
	#define MMX_ARRAY_MEM 0x1FFFF
#else
	#define MMX_ARRAY_MEM 0x1FFFF
#endif

#ifndef LPVOID
	#define LPVOID void*
#endif

template <class T>
class mmx_array
{
public:
	typedef mmx_array<T> Type;
	typedef T* pT;
	typedef T* iterator;
public:
	pT		p;
	size_t	s;
public:
	mmx_array(size_t ns=0):p(0),s(0) { if(ns) resize(ns); }
	mmx_array(const Type& v):p(0),s(0) { *this = v; }
	mmx_array(const std::vector<T>& v):p(0),s(0) { *this = v; }
	mmx_array(const std::list<T>& v):p(0),s(0) { *this = v; }

	virtual ~mmx_array() { clear(); }
public:
	bool empty() { return (p==0); }
	void clear(); 
	size_t size() { return s; }

	void set32(int a);
	void set16(short a);
	void set8(unsigned char a);
	void set0();
	void set(T *adr,T v,int cnt);
	void set(T v) { set(p,v,(int)s); }

	void resize(size_t ns,bool cl=true,int addMem=0);
	void exportto(void *dst);
	
	unsigned checksumm();
	void reverse();

	void swap(Type& v);
	void push(size_t l);

	pT begin() { return p; }
	pT end() { return (p+s); }
public:
	operator pT() { return (pT)p; }
	operator LPVOID() { return p; }
	const Type& operator = (const Type& v) { resize(v.s,false); if(s) SSEmemcpy(p,v.p,s*sizeof(T)); return v; }

	const Type& operator = (const std::vector<T>& v);
	const Type& operator = (const std::list<T>& v);

	friend bool operator == (const mmx_array<T>& a,const mmx_array<T>& b);
	friend bool operator != (const mmx_array<T>& a,const mmx_array<T>& b);

	int	compare(const mmx_array<T>& a) const;

	operator std::vector<T>();
	operator std::list<T>();
public:
	T get_min();
	T get_max();
};

template <class T>
class mmx_array2 : public mmx_array<T>
{
public:
	typedef mmx_array2<T> Type2;
public:
	int w,h;
public:
	mmx_array2():w(0),h(0) {}
	mmx_array2(int nw,int nh) :w(0),h(0) { resize(nw,nh); }
	mmx_array2(const Type2& v) :w(0),h(0) { *this = v; }
	mmx_array2(const Type& v) :w(0),h(0) { *this = v; }
	virtual ~mmx_array2() { clear(); };
public:
	void clear() { Type::clear(); w=h=0; }
	void resize(int nw,int nh,bool cl=true,int addMem=0);
public:
	void import(Type2& v);
public:
	pT operator [](int y) 
	{
#ifdef _DEBUG
#ifdef ASSERT
		ASSERT(y >= 0 && y < h);
#endif
#endif
		return p + y*w;
	}
	const Type2& operator = (const Type2& v) 
	{
		resize(v.w,v.h,false);
		if(s) SSEmemcpy(p,v.p,s*sizeof(T));
		return v;
	}

	const Type2& operator = (const Type& v) 
	{
		resize(v.s,1,false);
		if(s) SSEmemcpy(p,v.p,v.s*sizeof(T));
		return *this;
	}

};


template <class T>
class mmx_array_large
{
public:
	typedef mmx_array_large<T> Type;
	typedef T* pT;
	typedef T* iterator;
public:
	std::vector< mmx_array<T> > data;
	size_t  s,block;
public:
	mmx_array_large(size_t ns=0):block(0),s(0) { if(ns) resize(ns); }
	mmx_array_large(const Type& v):block(0),s(0) { *this = v; }


	virtual ~mmx_array_large() { clear(); }
public:
	bool empty() { return (s==0); }
	size_t size() { return s; }

	void clear(); 
	void set32(int a);
	void set16(short a);
	void set8(unsigned char a);
	void set0();
	void set(T *adr,T v,int cnt);
	void set(T v);

	void resize(size_t ns,bool cl=true,int addMem=0);

	T& operator[](size_t i);

public:
	const Type& operator = (const Type& v) { s=v.s,block=v.block; data = v.data; return v; }
};

template <class T>
inline void mmx_array<T>::clear() 
{ 
	if(p)
	{
		if(s*sizeof(T) > MMX_ARRAY_MEM)
			VirtualFree(p,0,MEM_RELEASE);
		else
			MMXfree(p);
	}
	s=0; 
	p=0; 
}

template <class T>
inline void mmx_array<T>::resize(size_t ns,bool cl,int addMem) 
{ 
	if(s == ns)
	{
		if(cl) set0();
		return; 
	}
	clear();
	if(! ns) return;

	if(ns*sizeof(T) > MMX_ARRAY_MEM)
		p = (pT)VirtualAlloc(0,ns*sizeof(T)+addMem,MEM_COMMIT,PAGE_READWRITE);
	else
		p = (pT)MMXalloc(ns*sizeof(T)+addMem);

#if defined _DEBUG && defined ASSERT
	ASSERT( p != 0);
#endif

	if(p) s=ns;
	if(cl) set0();

#ifdef _DEBUG
	if(!p)
	{
		int breakpoint = 0;
	}
#endif
}

template <class T>
inline void mmx_array<T>::swap(mmx_array<T>& v)
{
	void *tp = p;
	size_t ts = s;
	p = v.p;
	s = v.s;
	v.p = tp;
	v.s = ts;
}

template <class T>
inline void mmx_array<T>::push(size_t l)
{
	if( !l ) return;

	Type t;
	t.resize(s + l,false );
	if(p)
		SSEmemcpy(t.p,p,s);
	swap(t);
}

template <class T>
inline void mmx_array<T>::set32(int a) 
{ 
	if(!s) return;

	SSEmemset(p,a,s*sizeof(T)); 
}
template <class T>
inline void mmx_array<T>::set16(short a) 
{ 
	if(!s) return;
	short tmp[2] = {a,a};
	SSEmemset(p,*(int*)tmp,s*sizeof(T)); 
}
template <class T>
inline void mmx_array<T>::set8(unsigned char a) 
{
	if(!s) return;
	short tmp[4] = {a,a,a,a};
	SSEmemset(p,*(int*)tmp,s*sizeof(T)); 
}

template <class T>
inline void mmx_array<T>::set0() 
{ 
	if(s) SSESafeMemset(p,0,s*sizeof(T));
}

template <class T>
inline void mmx_array<T>::set(T *adr,T v,int cnt)
{
	pT e = adr + cnt;
	while(adr != e)	*adr = v,++adr;
}

template <class T>
inline void mmx_array<T>::exportto(void *dst)
{
	if(empty()) return;
		SSEmemcpy(dst,p,size()*sizeof(T));
}

template <class T>
inline void mmx_array<T>::reverse()
{
	for(int k = 0,c2 = s/2; k < c2; ++k)
	{
		int k2 = s-k-1;
		T d = p[k];
		p[k] = p[k2];
		p[k2] = d;
	}
}

template <class T>
inline unsigned mmx_array<T>::checksumm()
{
	int ns = (s*sizeof(T))/4;
	if(! ns ) return 0;
	unsigned * pu = (unsigned *)p,summ = 0;
	for(int k=0;k<ns;++k)
		summ += pu[k];
	return summ;
}

template <class T>
inline const mmx_array<T>& mmx_array<T>::operator = (const std::vector<T>& v) 
{ 
	resize(v.size(),false); 
	if(! s) return *this;
	if(((DWORD)&v[0]) & 0x0f)
		memcpy(p,&v[0],s*sizeof(T));
	else
		SSEmemcpy(p,&v[0],s*sizeof(T));
	return *this;
}

template <class T>
inline mmx_array<T>::operator std::vector<T>()
{
	std::vector<T> v;
	if(!s) return v;
	v.resize(s);

	if(((DWORD)&v[0]) & 0x0f)
		memcpy(&v[0],p,s*sizeof(T));
	else
		SSEmemcpy(&v[0],p,s*sizeof(T));

	return v;
}

template <class T>
inline const mmx_array<T>& mmx_array<T>::operator = (const std::list<T>& v) 
{ 
	resize(v.size(),false);
	pT a = p;
	std::list<T>::const_iterator i;
	for(i=v.begin();i!=v.end();++i,++a)
		*a = *i;
	return *this;
}

template <class T>
inline mmx_array<T>::operator std::list<T>()
{
	std::list<T> v;
	if(!s) return v;
	for(int k=0;k<s;++k)
		v.push_back(p[k]);
	return v;
}


template <class T>
inline int mmx_array<T>::compare(const mmx_array<T>& a) const
{
	if(a.s != s)
		return 1;
	if(!a.p && !p)
		return 0;
	if(!a.p || !p)
		return 1;

//	bool isEq = MMXstrcmp((const char*)a.p,(const char*)p,s);
//	return isEq ? 0 : 1;
	return memcmp(a.p,p,s);
}

template <class T>
inline bool operator == (const mmx_array<T>& a,const mmx_array<T>& b)
{
	return (a.compare(b) == 0)
}

template <class T>
inline bool operator != (const mmx_array<T>& a,const mmx_array<T>& b)
{
	return (a.compare(b) != 0)
}

template <class T>
inline void mmx_array2<T>::resize(int nw,int nh,bool cl,int addMem)
{
	if(w == nw && h == nh)
	{
		if(cl) set0();
		return;
	}
	clear();
	Type::resize(nw*nh,cl,addMem);
	if(p) w = nw,h = nh;
}



template <class T>
inline T mmx_array<T>::get_min()
{
	if( empty() ) return T();
	T m = p[0];
	for(size_t k=1;k<s;++k)
		if(p[k] < m) m = p[k];
	return m;
}

template <class T>
inline T mmx_array<T>::get_max()
{
	if( empty() ) return T();
	T m = p[0];
	for(size_t k=1;k<s;++k)
		if(p[k] > m) m = p[k];
	return m;
}

template <class T>
inline void mmx_array2<T>::import(Type2& v)
{
	if( (w == v.w && h == v.h) || !v.w || !v.h)
	{
		*this = v;
	}else
	{
		for(int y=0;y<h;++y)
		{
			int ny=y*v.h/h;
			pT lc = (*this)[y];
			pT lv = v[ny];
			
			if(v.w != w)
			{
				for(int x=0;x<w;++x)
				{
					int nx=x*v.w/w;
					lc[x] = lv[nx];	
				}
			} else
				SSEmemcpy(lc,lv,w*sizeof(T));
		}
	}
}

template <class T>
inline void mmx_array_large<T>::clear()
{
	s = block = 0;
	data.clear();
}

template <class T>
inline void mmx_array_large<T>::set32(int a)
{
	size_t s = data.size();
	for(size_t i = 0; i < s; ++i)
		data[i].set32(a);
}

template <class T>
inline void mmx_array_large<T>::set16(short a)
{
	size_t s = data.size();
	for(size_t i = 0; i < s; ++i)
		data[i].set16(a);
}

template <class T>
inline void mmx_array_large<T>::set8(unsigned char a)
{
	size_t s = data.size();
	for(size_t i = 0; i < s; ++i)
		data[i].set8(a);
}

template <class T>
inline void mmx_array_large<T>::set0()
{
	size_t s = data.size();
	for(size_t i = 0; i < s; ++i)
		data[i].set0();
}

template <class T>
inline void mmx_array_large<T>::set(T v)
{
	size_t s = data.size();
	for(size_t i = 0; i < s; ++i)
		data[i].set(v);
}

template <class T>
inline void mmx_array_large<T>::resize(size_t ns,bool cl=true,int addMem=0)
{
	if(ns == s)
	{
		if(cl) set0();
		return;
	}

	if(ns == 0)
	{
		clear();
		return;
	}

	size_t fs = ns*sizeof(T)+addMem;
	size_t bs = min( (1024*1024/sizeof(T))*sizeof(T), fs );
	size_t nb = (fs+bs-1)/bs;

	s = ns;
	block = bs/sizeof(T);

	if(data.size() != nb)
		data.resize(nb);

	size_t as = fs/sizeof(T);

	for(size_t i = 0; i < nb; ++i)
	{
		mmx_array<T>& a = data[i];
		size_t s = min(as,block);
		a.resize(s,cl);
		as -= s;
	}
}

template <class T>
T& mmx_array_large<T>::operator[](size_t i)
{
	size_t nb = i/block;
	size_t pos = i%block;
	return data[nb][pos];
}