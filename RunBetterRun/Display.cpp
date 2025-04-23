#include "Display.h"
#include "TextureManager.h"
#include "SpriteManager.h"

BOOL Display::Action(void)
{
	return FALSE;
}

Display::Display(FPOINT pos,LPCWCH texturePath)
{
	sprite.texture = TextureManager::GetInstance()->GetTexture(texturePath);
	sprite.type = SpriteType::NONE;
	Init(pos);
}

HRESULT Display::Init(FPOINT pos)
{
	if(!sprite.texture)	
	{
		sprite.texture = TextureManager::GetInstance()->GetTexture(TEXT("Image/drumtong.bmp"));
		if(!sprite.texture)
			return E_FAIL;
	}
	sprite.type = SpriteType::NONE;
	sprite.pos = pos;
	sprite.distance = 0;
	sprite.aniInfo = {0,0,{128,128},{1,1},{0,0}};
	SpriteManager::GetInstance()->AddSprite(sprite);
	return S_OK;
}
