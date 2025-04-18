#include "ItemManager.h"
#include "SpriteManager.h"
#include "TextureManager.h"

HRESULT ItemManager::LoadFile(LPCWCH path)
{
    return S_OK;
}

HRESULT ItemManager::Init(void)
{
    texture = TextureManager::GetInstance()->GetTexture(TEXT("Image/jewel.bmp"));
    if (!texture)
        return E_FAIL;
    aniInfo = { 0.1f, 0.1f, {456, 488}, {10, 1}, {0, 0}};
    return S_OK;
}

HRESULT ItemManager::Init(LPCWCH path)
{
    return S_OK;
}

void ItemManager::Release(void)
{
    items.clear();
}

void ItemManager::Update(void)
{
    auto iter = items.begin();
    while (iter != items.end())
    {
        if ((*iter).distance < 0.1f)
        {
            SpriteManager::GetInstance()->DeleteSprite(*iter);
            iter = items.erase(iter);
        }
        else
            iter++;
    }
}

void ItemManager::PutItem(FPOINT pos)
{
    Sprite item{ pos, 0, texture, aniInfo };
    items.push_back(item);
    SpriteManager::GetInstance()->AddSprite(items.back());
}
