#include "Key.h"
#include "TextureManager.h"
#include "SpriteManager.h"
#include "ItemManager.h"
#include "UIManager.h"
#include "PhoneUI.h"
#include "SoundManager.h"

BOOL Key::Action(void)
{
	ItemManager::GetInstance()->PopKey();
	PhoneUI* phone = (static_cast<PhoneUI*>(UIManager::GetInstance()->GetUIUnit("PhoneUI")));
	if (phone)
		phone->ShakeOnItemGet();

	return TRUE;
}

HRESULT Key::Init(FPOINT pos)
{
	sprite.id = 0;
	sprite.texture = TextureManager::GetInstance()->GetTexture(TEXT("Image/soul.bmp"));
	if(!sprite.texture)
		return E_FAIL;
	sprite.type = SpriteType::KEY;
	sprite.pos = pos;
	sprite.distance = 0;
	sprite.aniInfo = {0.1f,0.1f,{250,250},{20,1},{rand() % 20,0}};
	SpriteManager::GetInstance()->AddSprite(sprite);
	ItemManager::GetInstance()->PushKey();
	return S_OK;
}
