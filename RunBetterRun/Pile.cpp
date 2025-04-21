#include "Pile.h"
#include "TextureManager.h"
#include "KeyManager.h"
#include "TimerManager.h"

BOOL Pile::Action(void)
{
	if(KeyManager::GetInstance()->IsStayKeyDown('E'))
	{
		float deltaTime = TimerManager::GetInstance()->GetDeltaTime();
		obstacle.aniInfo.currentTime -= deltaTime;

		if(obstacle.aniInfo.currentTime < 0)
		{
			obstacle.aniInfo.currentTime = obstacle.aniInfo.frameTime;
			obstacle.aniInfo.currentFrame.x = (obstacle.aniInfo.currentFrame.x + 1) % obstacle.aniInfo.maxFrame.x;
		}
		return TRUE;
	}
	return FALSE;
}

Pile::Pile(POINT pos,Direction dir)
{
	obstacle.texture = TextureManager::GetInstance()->GetTexture(TEXT("Image/jewel.bmp"));
	obstacle.aniInfo = {0.1f,0.1f,{456,488},{10,1},{0,0}};
	Init(pos,dir);
}
