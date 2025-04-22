#include "AMonster.h"
#include "TextureManager.h"
#include "TimerManager.h"
#include "MapManager.h"

void AMonster::Update(void)
{
	if(sprite.distance < AMONSTER_SIZE)
		return Catch();

	FLOAT timePerSpeed = TimerManager::GetInstance()->GetDeltaTime() * speed;
	FLOAT dx = targetPosition.x - sprite.pos.x;
	FLOAT dy = targetPosition.y - sprite.pos.y;
	FLOAT distance = sqrt(powf(dx, 2)+ powf(dy, 2));

	if(distance < timePerSpeed && CanMoveToPosition(targetPosition))
		sprite.pos = targetPosition;
	else
	{
		FLOAT dirX = dx / distance;
		FLOAT dirY = dy / distance;

		FPOINT newPos = {
			sprite.pos.x + dirX * timePerSpeed,
			sprite.pos.y + dirY * timePerSpeed
		};

		if(CanMoveToPosition(newPos)){
			UpdateDirection(newPos);
			sprite.pos = newPos;
		}
	}

	Action();
}

void AMonster::Action(void)
{
	// Something Special
}

void AMonster::Catch(void)
{
	
}

BOOL AMonster::CanMoveToPosition(FPOINT pos)
{
	MapData* md = MapManager::GetInstance()->GetMapData();
	if(!md)
		return false;

	int x = static_cast<int>(pos.x);
	int y = static_cast<int>(pos.y);

	if(x < 0 || x >= md->width || y < 0 || y >= md->height)
		return false;

	Room& tile = md->tiles[y * md->width + x];
	return (tile.roomType == RoomType::FLOOR
		|| tile.roomType == RoomType::START);

	FLOAT timePerSpeed = TimerManager::GetInstance()->GetDeltaTime() * speed;
	FLOAT dx = targetPosition.x - sprite.pos.x;
	FLOAT dy = targetPosition.y - sprite.pos.y;
	FLOAT distance = sqrt(powf(dx,2)+ powf(dy,2));

	if(distance == 0.0f) return false;
}

void AMonster::UpdateDirection(FPOINT newPos)
{
	float dx,dy;

	dx = newPos.x - sprite.pos.x;
	dy = newPos.y - sprite.pos.y;

	if(abs(dx) > abs(dy))
	{
		if(dx>0) 
		{
			direction = Direction::EAST;
		}
		else 
		{
			direction = Direction::WEST;
		}
	}
	else
	{
		if(dy>0)
		{
			direction=Direction::SOUTH;
		}

		else
		{
			direction=Direction::NORTH;
		}
	}

	switch(direction)
	{
		case Direction::NORTH:
			// 여기다가 스프라이트 업데이트?
			break;
		case Direction::EAST:
			//
			break;
		case Direction::SOUTH:
			//
			break;
		case Direction::WEST:
			//
			break;
	}
}
