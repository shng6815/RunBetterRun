#include "SpriteManager.h"
#include "TextureManager.h"
#include "Player.h"
#include "TimerManager.h"
#include <fstream>
#include <algorithm>

HRESULT SpriteManager::Init()
{
    static bool isInitialized = false;
    if (isInitialized)
        return S_OK;
    sprites.clear();
    playerPos = { 0, 0 };
    isInitialized = true;
    return S_OK;
}

void SpriteManager::Release()
{
    sprites.clear();
}

void SpriteManager::SortSpritesByDistance()
{
    playerPos = Player::GetInstance()->GetCameraPos();
    float deltaTime = TimerManager::GetInstance()->GetDeltaTime();
    for (auto& sprite : sprites)
    {
		float distance = sqrtf(
			powf(playerPos.x - sprite->pos.x,2) +
			powf(playerPos.y - sprite->pos.y,2)
		);

		if(sprite->type != SpriteType::MONSTER
			|| fabs(sprite->distance - distance) > 1e-3f)
		{
			sprite->aniInfo.currentTime -= deltaTime;
			if(sprite->aniInfo.currentTime < 0)
			{
				sprite->aniInfo.currentTime = sprite->aniInfo.frameTime;
				sprite->aniInfo.currentFrame.x = (sprite->aniInfo.currentFrame.x + 1) % sprite->aniInfo.maxFrame.x;
			}
		} else
			sprite->aniInfo.currentFrame.x = 0;

		sprite->distance = distance;
    }

    sprites.sort([](Sprite* a, Sprite* b) -> BOOL {
        return a->distance > b->distance;
        });
}

void SpriteManager::AddSprite(Sprite& sprite)
{
    auto iter = find(sprites.begin(), sprites.end(), &sprite);
    if (iter == sprites.end())
        sprites.push_back(&sprite);
}

void SpriteManager::DeleteSprite(Sprite& sprite)
{
    auto iter = find(sprites.begin(), sprites.end(), &sprite);
    if (iter != sprites.end())
        sprites.erase(iter);
}
