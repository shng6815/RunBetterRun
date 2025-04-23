#include "Elevator.h"
#include "TextureManager.h"

Elevator::Elevator(POINT pos,Direction dir)
{
	obstacle.texture = TextureManager::GetInstance()->GetTexture(TEXT("Image/elevator.bmp"));
	obstacle.aniInfo = {0.2f,0.2f,{128,128},{8,1},{0,0}};
	obstacle.pos = pos;
	obstacle.dir = dir;
	obstacle.block = TRUE;
	obstacle.distance = 0;
	status = DoorStatus::Close;
}

BOOL Elevator::Action(void)
{
	if(obstacle.distance <= OBSTACLE_SIZE && KeyManager::GetInstance()->IsOnceKeyDown('E'))
	{
		switch(status)
		{
			case Elevator::DoorStatus::Close:
			status = DoorStatus::Opening;
			return TRUE;
			case Elevator::DoorStatus::Open:
			obstacle.block = TRUE;
			status = DoorStatus::Closing;
			return TRUE;
			case Elevator::DoorStatus::Final:
			obstacle.aniInfo.frameTime = 0.4f;
			obstacle.aniInfo.currentTime = 0.4f;
			status = DoorStatus::Opening;
			return TRUE;
			default:
			break;
		}
	}
	return FALSE;
}

void Elevator::Update(void)
{
	float deltaTime = TimerManager::GetInstance()->GetDeltaTime();
	obstacle.aniInfo.currentTime -= deltaTime;

	if(obstacle.aniInfo.currentTime < 0)
	{
		obstacle.aniInfo.currentTime = obstacle.aniInfo.frameTime;
		switch(status)
		{
		case Elevator::DoorStatus::Closing:
		if(obstacle.aniInfo.currentFrame.x > 0)
			--obstacle.aniInfo.currentFrame.x;
		else
			status = DoorStatus::Close;
		break;
		case Elevator::DoorStatus::Opening:
		++obstacle.aniInfo.currentFrame.x;
		if(obstacle.aniInfo.currentFrame.x + 1 == obstacle.aniInfo.maxFrame.x)
		{
			obstacle.block = FALSE;
			status = DoorStatus::Open;
		}
		break;
		default:
		break;
		}
	}
	
}

void Elevator::Lock(void)
{
	if(status == DoorStatus::Open || status == DoorStatus::Close)
	{
		obstacle.aniInfo.currentFrame.x = 0;
		obstacle.block == TRUE;
		status = DoorStatus::Lock;
	}
}

void Elevator::UnLock(void)
{
	if (status == DoorStatus::Lock)
		status = DoorStatus::Final;
}