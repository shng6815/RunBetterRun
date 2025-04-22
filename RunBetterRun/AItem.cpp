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
		Action();
		SpriteManager::GetInstance()->DeleteSprite(sprite);

		(static_cast<PhoneUI*>(UIManager::GetInstance()->GetUIUnit("PhoneUI")))->ShakeOnItemGet();

		return true;
	}
	return false;
}