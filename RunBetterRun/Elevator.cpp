#include "Elevator.h"
#include "TextureManager.h"

Elevator::Elevator(POINT pos,Direction dir)
{
	obstacle.id = 9;
	obstacle.texture = TextureManager::GetInstance()->GetTexture(TEXT("Image/elevator.bmp"));
	obstacle.aniInfo = {0.2f,0.2f,{128,128},{8,1},{0,0}};
	obstacle.pos = pos;
	obstacle.dir = dir;
	obstacle.block = TRUE;
	obstacle.distance = 0;
	status = DoorStatus::Init;
	waitTime = 3.5f;
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
				status = DoorStatus::Closing;
				return TRUE;

			case Elevator::DoorStatus::UnLock:
				obstacle.aniInfo.frameTime = 0.3f;
				obstacle.aniInfo.currentTime = 0.3f;
				status = DoorStatus::FinalOpening;
				return TRUE;

			case Elevator::DoorStatus::FinalOpen:
				status = DoorStatus::FinalClosing;
				return TRUE;

			default:
				break;
		}
	}
	return FALSE;
}

void Elevator::Update(void)
{

	FLOAT deltaTime = TimerManager::GetInstance()->GetDeltaTime();

	switch(status)
	{
	case Elevator::DoorStatus::Init:
		if (waitTime > 0)
			waitTime -= deltaTime;
		else
			status = DoorStatus::Opening;
			break;

	case Elevator::DoorStatus::Closing:
		if (Close(deltaTime))
			status = DoorStatus::Close;
			break;

	case Elevator::DoorStatus::Opening:
		if (Open(deltaTime))
			status = DoorStatus::Open;
			break;

	case Elevator::DoorStatus::FinalClosing:
		if (Close(deltaTime))
		{
			waitTime = 3.5f;
			status = DoorStatus::Exit;
		}
		break;

	case Elevator::DoorStatus::FinalOpening:
		if (Open(deltaTime))
			status = DoorStatus::FinalOpen;
		break;

	case Elevator::DoorStatus::Exit:
		if(waitTime > 0)
			waitTime -= deltaTime;
		else
			/*SceneManager::GetInstance()->ChangeScene();*/
		break;

	default:
		break;
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
		status = DoorStatus::UnLock;
}

BOOL Elevator::Open(FLOAT deltaTime)
{
	obstacle.aniInfo.currentTime -= deltaTime;

	if(obstacle.aniInfo.currentTime < 0)
	{
		obstacle.aniInfo.currentTime = obstacle.aniInfo.frameTime;
		++obstacle.aniInfo.currentFrame.x;
		if(obstacle.aniInfo.currentFrame.x + 1 >= obstacle.aniInfo.maxFrame.x)
		{
			obstacle.block = FALSE;
			return TRUE;
		}
		else if (obstacle.aniInfo.currentFrame.x == 3)
			obstacle.block = FALSE;
	}
	return FALSE;
}

BOOL Elevator::Close(FLOAT deltaTime)
{
	obstacle.aniInfo.currentTime -= deltaTime;

	if(obstacle.aniInfo.currentTime < 0)
	{
		obstacle.aniInfo.currentTime = obstacle.aniInfo.frameTime;
		if(obstacle.aniInfo.currentFrame.x > 0)
			--obstacle.aniInfo.currentFrame.x;
		else if(obstacle.aniInfo.currentFrame.x == 3)
			obstacle.block = TRUE;
		else
		{
			obstacle.block = TRUE;
			return TRUE;
		}
	}
	return FALSE;
}
