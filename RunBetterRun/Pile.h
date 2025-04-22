#pragma once
#include "AObstacle.h"

class Pile: public AObstacle
{
	virtual BOOL Action(void) override;

public:
	Pile(POINT pos,Direction dir);

};

