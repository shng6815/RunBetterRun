#include "AItem.h"
#include "TextureManager.h"
#include "SpriteManager.h"
#include "ItemManager.h"


BOOL AItem::Update(void)
{
	if(sprite.distance < ITEM_SIZE)
	{
		Action();
		SpriteManager::GetInstance()->DeleteSprite(sprite);
		return true;
	}
	return false;
}