#include "AObstacle.h"
#include "MapManager.h"


HRESULT AObstacle::Init(void)
{
	MapData* md = MapManager::GetInstance()->GetMapData();

	DWORD mapCoordinate = obstacle.pos.y * md->width + obstacle.pos.x;
	if(mapCoordinate < md->width * md->height && !md->tiles[mapCoordinate].obstacle)
	{
		md->tiles[mapCoordinate].obstacle = &obstacle;
		active = TRUE;
		return S_OK;
	}
	return E_FAIL;
}

void AObstacle::Release(void)
{
	MapData* md = MapManager::GetInstance()->GetMapData();

	DWORD mapCoordinate = obstacle.pos.y * md->width + obstacle.pos.x;
	if(active && mapCoordinate < md->width * md->height)
		md->tiles[mapCoordinate].obstacle = nullptr;
}

void AObstacle::Update(void)
{
	// Update things... ex: Animation or Opening door things
}

void AObstacle::Distance(FPOINT player)
{
	FPOINT pos;
	pos.x = obstacle.pos.x;
	pos.y = obstacle.pos.y;

	switch(obstacle.dir)
	{
	case Direction::EAST:
	pos.y += +0.5;
	break;
	case Direction::WEST:
	pos.y += +0.5;
	pos.x += 1;
	break;
	case Direction::SOUTH:
	pos.x += +0.5;
	break;
	case Direction::NORTH:
	pos.x += +0.5;
	pos.y += 1;
	break;
	}

	obstacle.distance = sqrtf(powf(pos.x - player.x,2)
	+ powf(pos.y - player.y,2));
}
