#include "AItem.h"
#include "TextureManager.h"
#include "SpriteManager.h"
#include "ItemManager.h"
#include "UIManager.h"
#include "PhoneUI.h"


BOOL AItem::Update(void)
{
	if(sprite.distance < ITEM_SIZE)
	{
		if (Action())
		{
			SpriteManager::GetInstance()->DeleteSprite(sprite);
			return true;
		}
	}
	return false;
}