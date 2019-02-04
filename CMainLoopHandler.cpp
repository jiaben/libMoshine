#include "CMainLoopHandler.h"
#include "CEventHandler.h"
#include "cocos2d.h"

USING_NS_CC;

const std::string CMainLoopHandler::SchedName = "MainLoopForDirector";

CMainLoopHandler::CMainLoopHandler()
	:m_bIgnore(false)
{

}

CMainLoopHandler::~CMainLoopHandler(){
	SetIgnore(true);
	while(!m_lstWaitedEvent.empty()){
		CEventHandler* pEvent = m_lstWaitedEvent.front();
		m_lstWaitedEvent.pop_front();
        pEvent->Release();
	}

	while(!m_lstCurrentEvent.empty()){
		CEventHandler* pEvent = m_lstCurrentEvent.front();
		m_lstCurrentEvent.pop_front();
		pEvent->Release();
	}
}

void CMainLoopHandler::SetIgnore(bool bFlag){
	m_bIgnore = bFlag;
}

void CMainLoopHandler::MainLoop(){
	m_ImageInfoMutex.lock();
	std::swap(m_lstCurrentEvent, m_lstWaitedEvent);
	m_ImageInfoMutex.unlock();

	while(!m_lstCurrentEvent.empty()){
		CEventHandler* pEvent = m_lstCurrentEvent.front();
		m_lstCurrentEvent.pop_front();
		if (!m_bIgnore)
			pEvent->OnEvent();
		pEvent->Release();
	}
}

void CMainLoopHandler::AddToMainLoop(){
	auto scheduler = Director::getInstance()->getScheduler();
	
	scheduler->schedule([&](float) {
		CMainLoopHandler::Inst().MainLoop();
	}, &CMainLoopHandler::Inst(), 0, false, CMainLoopHandler::SchedName);
}

CMainLoopHandler& CMainLoopHandler::Inst(){
	static CMainLoopHandler _inst;
	return _inst;
}

void CMainLoopHandler::AddEvent(CEventHandler* pEvent){
	if (m_bIgnore)
		return;
	m_ImageInfoMutex.lock();
	m_lstWaitedEvent.push_back(pEvent);
	m_ImageInfoMutex.unlock();
}
