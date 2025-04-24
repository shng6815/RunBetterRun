#pragma once
#include "AObstacle.h"
class Elevator: public AObstacle
{
	enum class DoorStatus
	{ Init, Close, Closing, Open, Opening, Lock, UnLock, FinalOpening, FinalOpen, FinalClosing, Exit};

	DoorStatus  status;
	FLOAT		waitTime;

	BOOL Open(FLOAT deltaTime);
	BOOL Close(FLOAT deltaTime);

public:
	Elevator(POINT pos,Direction dir,DWORD id);
	virtual BOOL Action(void) override;
	virtual void Update(void) override;

	void Lock(void);
	void UnLock(void);
};


