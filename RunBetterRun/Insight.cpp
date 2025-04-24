#include "Insight.h"
#include "TextureManager.h"
#include "SpriteManager.h"
#include "UIManager.h"
#include "PhoneUI.h"

BOOL Insight::Action(void)
{
	PhoneUI* phoneUI = static_cast<PhoneUI*>(UIManager::GetInstance()->GetUIUnit("PhoneUI"));
	if(phoneUI) {
		phoneUI->GetInsight(10.0f); // 10초 동안 인사이트 기능 활성화
	}
	return TRUE;
}

HRESULT Insight::Init(FPOINT pos)
{
	sprite.texture = TextureManager::GetInstance()->GetTexture(TEXT("Image/dongseonambuk.bmp"));
	if (!sprite.texture)
		return E_FAIL;
	sprite.type = SpriteType::ITEM;
	sprite.pos = pos;
	sprite.distance = 0;
	sprite.aniInfo = { 0,0,{250,250},{1,1},{0,0} };
	SpriteManager::GetInstance()->AddSprite(sprite);
	return S_OK;
}
