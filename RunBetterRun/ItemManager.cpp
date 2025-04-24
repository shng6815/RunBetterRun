#include "ItemManager.h"
#include "SpriteManager.h"
#include "TextureManager.h"
#include "AItem.h"
#include "SoundManager.h"

HRESULT ItemManager::LoadFile(LPCWCH path)
{
    return S_OK;
}

HRESULT ItemManager::Init(void)
{
	keyCount = 0;
	items.clear();

	SoundManager::GetInstance()->LoadSound("CollectKey","Sounds/SFX_GetItem.wav");

    return S_OK;
}

HRESULT ItemManager::Init(LPCWCH path)
{
	keyCount = 0;
	items.clear();

	SoundManager::GetInstance()->LoadSound("CollectKey","Sounds/SFX_GetItem.wav");

    return S_OK;
}

void ItemManager::Release(void)
{
	for(auto& item : items)
		delete item;
    items.clear();
}

void ItemManager::Update(void)
{
    auto iter = items.begin();
    while (iter != items.end())
    {
		if ((*iter)->Update())
		{
			delete *iter;
			iter = items.erase(iter);
		}
        else
            iter++;
    }
}

void ItemManager::PutItem(AItem* item)
{
    items.push_back(item);
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
		SoundManager::GetInstance()->PlaySoundW("CollectKey");
	}
}
