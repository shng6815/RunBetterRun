#include "ObstacleManager.h"
#include "MapManager.h"
#include "Player.h"

HRESULT ObstacleManager::Init(void)
{
	if(!obstacles.empty())
		Release();

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
}

void ObstacleManager::Update(void)
{
	FPOINT player = Player::GetInstance()->GetCameraPos();
	for(auto& obstacle: obstacles)
		obstacle->Distance(player);

	for(auto& obstacle: obstacles)
	{
		if(obstacle->Update())
			break;
	}
}

void ObstacleManager::PutObstacle(AObstacle* obstacle)
{
	obstacles.push_back(obstacle);
}
