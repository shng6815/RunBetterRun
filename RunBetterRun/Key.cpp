#include "Key.h"
#include "TextureManager.h"
#include "SpriteManager.h"
#include "ItemManager.h"
#include "UIManager.h"
#include "PhoneUI.h"

BOOL Key::Action(void)
{
	ItemManager::GetInstance()->PopKey();
	(static_cast<PhoneUI*>(UIManager::GetInstance()->GetUIUnit("PhoneUI")))->ShakeOnItemGet();
	return TRUE;
}

HRESULT Key::Init(FPOINT pos)
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
