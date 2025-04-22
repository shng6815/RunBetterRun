#pragma once
#include "AObstacle.h"

class Pile: public AObstacle
{

public:
	Pile(POINT pos,Direction dir);
	virtual BOOL Action(void) override;

};

