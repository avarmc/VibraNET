#include "stdafx.h"
#include "NewVarEvent.h"


CCriticalSection				CNewVarEvent::m_lock;
std::list<CNewVarEvent *>		CNewVarEvent::m_listeners;

CNewVarEvent::CNewVarEvent()
{
	CSingleLock lock(&m_lock);
	m_listeners.push_back(this);
}


CNewVarEvent::~CNewVarEvent()
{
	CSingleLock lock(&m_lock);
	m_listeners.remove(this);
}

void CNewVarEvent::PushVar(int id, int subID)
{
	for (std::list<CNewVarEvent *>::iterator i = m_listeners.begin(), ie = m_listeners.end(); i != ie; ++i)
		(*i)->OnNewVar(id, subID);
}