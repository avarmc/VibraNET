#pragma once

class CInc
{
LONG *p;
LONG cur;
public:
	CInc(unsigned int *_p):p((LONG*)_p) { if(p) cur = InterlockedIncrement(p); }
	CInc(int *_p):p((LONG*)_p) { if(p) cur = InterlockedIncrement(p); }
	CInc(long *_p):p((LONG*)_p) { if(p) cur = InterlockedIncrement(p); }
	CInc(unsigned long *_p):p((LONG*)_p) { if(p) cur = InterlockedIncrement(p); }

	~CInc() {if(p) InterlockedDecrement(p);}
	LONG get() { return cur; } 
};