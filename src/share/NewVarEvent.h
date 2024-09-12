#pragma once

class CNewVarEvent
{
public:
	CNewVarEvent();
	virtual ~CNewVarEvent();
public:
	static CCriticalSection				m_lock;
	static std::list<CNewVarEvent *>	m_listeners;

	static void PushVar(int id, int subID);

	virtual void OnNewVar(int id, int subID) {}
};

