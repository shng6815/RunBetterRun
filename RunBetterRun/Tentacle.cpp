#include "Tentacle.h"
#include "TextureManager.h"
#include "SpriteManager.h"

void Tentacle::Action(void)
{
	
}

void Tentacle::Catch(void)
{
	//MessageBox(g_hWnd, TEXT("YOU ARE DEAD"),TEXT("Run Better Run"),MB_OK);
}

HRESULT Tentacle::Init(FPOINT pos)
{
	sprite.texture = TextureManager::GetInstance()->GetTexture(TEXT("Image/boss.bmp"));
	if(!sprite.texture)
		return E_FAIL;
	sprite.type = SpriteType::MONSTER;
	sprite.pos = pos;
	sprite.distance = 0;
	sprite.aniInfo = {0,0,{423,437},{1,1},{0,0}};
	SpriteManager::GetInstance()->AddSprite(sprite);
	targetPosition = pos;
	speed = 2.2f;
	return S_OK;
}
