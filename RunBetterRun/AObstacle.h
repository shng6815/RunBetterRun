#pragma once
#include "structs.h"

#define OBSTACLE_SIZE 0.3f

class AObstacle
{
protected:
	Obstacle	obstacle;
	BOOL		active;

public:
	virtual HRESULT Init(void);
	virtual void Release(void);
	virtual BOOL Action(void) = 0;
	virtual void Update(void);

	void Distance(FPOINT player);
};

