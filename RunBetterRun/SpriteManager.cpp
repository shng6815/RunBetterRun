#include "SpriteManager.h"
#include "TextureManager.h"
#include "Player.h"
#include <fstream>

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

void SpriteManager::PutSprite(LPCWCH path, FPOINT pos)
{
    Texture* texture = TextureManager::GetInstance()->GetTexture(path);
    if (texture)
        sprites.push_back(Sprite{ pos, 0, texture });
}

void SpriteManager::SortSpritesByDistance()
{
    playerPos = Player::GetInstance()->GetCameraPos();
    for (auto& sprite : sprites)
    {
        sprite.distance = sqrtf(
            powf(playerPos.x - sprite.pos.x, 2) +
            powf(playerPos.y - sprite.pos.y, 2)
        );
    }

    sprites.sort([](const Sprite& a, const Sprite& b) -> BOOL {
        return a.distance > b.distance;
        });
}