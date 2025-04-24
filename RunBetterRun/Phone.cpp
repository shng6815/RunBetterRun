#include "Phone.h"
#include "TextureManager.h"
#include "SpriteManager.h"
#include "UIManager.h"
#include "PhoneUI.h"

BOOL Phone::Action(void)
{
	PhoneUI* uiUnit = new PhoneUI();
	uiUnit->Init(UIType::PLAYING,FPOINT{100,WINSIZE_Y - 500},FPOINT{300,400},0);
	UIManager::GetInstance()->AddUIUnit("PhoneUI",uiUnit);
	return TRUE;
}

HRESULT Phone::Init(FPOINT pos)
{
	sprite.texture = TextureManager::GetInstance()->GetTexture(TEXT("Image/phone.bmp"));
	if(!sprite.texture)
		return E_FAIL;
	sprite.type = SpriteType::ITEM;
	sprite.pos = pos;
	sprite.distance = 0;
	sprite.aniInfo = {0, 0,{250,250},{1,1},{0, 0}};
	SpriteManager::GetInstance()->AddSprite(sprite);
	return S_OK;
}
