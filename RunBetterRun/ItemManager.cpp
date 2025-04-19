#include "ItemManager.h"
#include "SpriteManager.h"
#include "TextureManager.h"

HRESULT ItemManager::LoadFile(LPCWCH path)
{
    return S_OK;
}

HRESULT ItemManager::Init(void)
{
	keyCount = 0;
	items.clear();
    return S_OK;
}

HRESULT ItemManager::Init(LPCWCH path)
{
	keyCount = 0;
	items.clear();
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
        if (iter->Update())
            iter = items.erase(iter);
        else
            iter++;
    }
}

void ItemManager::PutItem(FPOINT pos)
{
    items.emplace_back();
	items.back().Init(pos);
}

void ItemManager::PushKey(void)
{
	++keyCount;
}

void ItemManager::PopKey(void)
{
	if(--keyCount == 0)
	{
		keyCount;
		// Escape Event
	}
}
