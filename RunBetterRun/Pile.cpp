#include "Pile.h"
#include "TextureManager.h"
#include "KeyManager.h"
#include "TimerManager.h"

BOOL Pile::Action(void)
{
	if(active && obstacle.distance <= OBSTACLE_SIZE && KeyManager::GetInstance()->IsOnceKeyDown('E'))
	{
		++obstacle.aniInfo.currentFrame.x;
		if(obstacle.aniInfo.currentFrame.x + 1 == obstacle.aniInfo.maxFrame.x)
		{
			active = FALSE;
			obstacle.block = FALSE;
		}
		return TRUE;
	}
	return FALSE;
}

Pile::Pile(POINT pos,Direction dir)
{
	obstacle.texture = TextureManager::GetInstance()->GetTexture(TEXT("Image/pile.bmp"));
	obstacle.aniInfo = {0.0f,0.0f,{128,128},{8,1},{0,0}};
	obstacle.pos = pos;
	obstacle.dir = dir;
	obstacle.block = TRUE;
	obstacle.distance = 0;
}
