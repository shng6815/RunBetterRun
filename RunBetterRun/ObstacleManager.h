#pragma once
#include "Singleton.h"
#include "structs.h"
#include "AObstacle.h"

class ObstacleManager: public Singleton<ObstacleManager>
{
	vector<AObstacle*> obstacles;

public:
	HRESULT Init(void);
	void Release(void);
	void Update(void);

	void PutObstacle(AObstacle* obstacle);
};

