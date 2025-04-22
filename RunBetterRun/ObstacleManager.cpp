#include "ObstacleManager.h"
#include "MapManager.h"
#include "Player.h"

HRESULT ObstacleManager::Init(void)
{
	if(!obstacles.empty())
		Release();
	if(elevator)
	{
		elevator->Release();
		delete elevator;
	}
	elevator = nullptr;

    return S_OK;
}

void ObstacleManager::Release(void)
{
	MapData* md = MapManager::GetInstance()->GetMapData();

	for(auto& obstacle: obstacles)
	{
		obstacle->Release();
		delete obstacle;
		obstacle = nullptr;
	}
	obstacles.clear();

	if(elevator)
	{
		elevator->Release();
		delete elevator;
	}
	elevator = nullptr;
}

void ObstacleManager::Update(void)
{
	FPOINT player = Player::GetInstance()->GetCameraPos();
	for(auto& obstacle: obstacles)
	{
		obstacle->Distance(player);
		obstacle->Update();
	}

	for(auto& obstacle: obstacles)
	{
		if(obstacle->Action())
			break;
	}
}

void ObstacleManager::PutObstacle(AObstacle* obstacle)
{
	if(obstacle)
	{
		if(FAILED(obstacle->Init()))
			obstacle->Release();
		else
			obstacles.push_back(obstacle);
	}
}

void ObstacleManager::PutObstacle(Elevator* elevator)
{
	if (elevator)
	{
		elevator->Init();
		if(this->elevator)
		{
			this->elevator->Release();
			delete this->elevator;
		}
		this->elevator = elevator;
	}
}

void ObstacleManager::LockElevator(void)
{
	if(elevator)
		elevator->Lock();
}

void ObstacleManager::UnlockElevator(void)
{
	if(elevator)
		elevator->UnLock();
}
