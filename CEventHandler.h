#pragma once
class CEventHandler{
public:
	virtual void OnEvent()=0;
	virtual void Release()=0;
};