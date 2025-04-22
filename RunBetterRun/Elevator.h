#pragma once
#include "AObstacle.h"
class Elevator: public AObstacle
{
	enum class DoorStatus
	{ Close, Closing, Open, Opening, Lock, Final };

	DoorStatus status;

public:
	Elevator(POINT pos,Direction dir);
	virtual BOOL Action(void) override;
	virtual void Update(void) override;

	void Lock(void);
	void UnLock(void);
};


