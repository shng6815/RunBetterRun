#include "Item.h"
#include "TextureManager.h"
#include "SpriteManager.h"
#include "ItemManager.h"

HRESULT Item::Init(FPOINT pos)
{
	sprite.texture = TextureManager::GetInstance()->GetTexture(TEXT("Image/jewel.bmp"));
	if(!sprite.texture)
		return E_FAIL;
	sprite.type = SpriteType::KEY;
	sprite.pos = pos;
	sprite.distance = 0;
	sprite.aniInfo = {0.1f,0.1f,{456,488},{10,1},{0,0}};
	SpriteManager::GetInstance()->AddSprite(sprite);
	ItemManager::GetInstance()->PushKey();
	return S_OK;
}

HRESULT Item::Init(FPOINT pos, LPCWCH path,AnimationInfo ani)
{
	sprite.texture = TextureManager::GetInstance()->GetTexture(path);
	if(!sprite.texture)
		return E_FAIL;
	sprite.type = SpriteType::ITEM;
	sprite.pos = pos;
	sprite.distance = 0;
	sprite.aniInfo = ani;
	SpriteManager::GetInstance()->AddSprite(sprite);
	return S_OK;
}

BOOL Item::Update(void)
{
	if(sprite.distance < ITEM_SIZE)
	{
		Action();
		SpriteManager::GetInstance()->DeleteSprite(sprite);
		return true;
	}
	return false;
}

void Item::Action(void)
{
	ItemManager::GetInstance()->PopKey();
}
