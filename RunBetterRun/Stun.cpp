#include "Stun.h"
#include "TextureManager.h"
#include "SpriteManager.h"
#include "MonsterManager.h"

BOOL Stun::Action(void)
{
	MonsterManager::GetInstance()->Stun(STUN_TIME);
    return TRUE;
}

HRESULT Stun::Init(FPOINT pos)
{
	sprite.texture = TextureManager::GetInstance()->GetTexture(TEXT("Image/soul.bmp"));
	if(!sprite.texture)
		return E_FAIL;
	sprite.type = SpriteType::ITEM;
	sprite.pos = pos;
	sprite.distance = 0;
	sprite.aniInfo = {0.1f,0.1f,{250,250},{20,1},{rand() % 20,0}};
	SpriteManager::GetInstance()->AddSprite(sprite);
	return S_OK;
}
