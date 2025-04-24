#pragma once
#include "Singleton.h"
#include "structs.h"
#include "AObstacle.h"
#include "Elevator.h"

class ObstacleManager: public Singleton<ObstacleManager>
{
	vector<AObstacle*>	obstacles;
	Elevator*			elevator;

public:
	HRESULT Init(void);
	void Release(void);
	void Update(void);

	void PutObstacle(AObstacle* obstacle);
	void PutObstacle(Elevator* elevator);

	void LockElevator(void);
	void UnlockElevator(void);

	POINT GetElevatorPosition(void) {
		return elevator->GetPosition();
	}
};

