#pragma once
#include "IMainLoopHandler.h"
#include <mutex>
#include <list>
#include <string>

class CEventHandler;
class CMainLoopHandler : public IMainLoopHandler
{
public:
	static const std::string SchedName;
	typedef std::list<CEventHandler*> LstEventHandler_t;

private:
	CMainLoopHandler();
	virtual ~CMainLoopHandler();
	virtual void MainLoop();

public:
	void AddEvent(CEventHandler* pEvent);
	static CMainLoopHandler& Inst();
	void SetIgnore(bool bFlag);
	void AddToMainLoop();

private:
	LstEventHandler_t	m_lstCurrentEvent;
	LstEventHandler_t	m_lstWaitedEvent;
	bool				m_bIgnore;
	std::mutex      m_ImageInfoMutex;
};