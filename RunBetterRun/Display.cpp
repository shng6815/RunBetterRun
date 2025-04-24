#include "Display.h"
#include "TextureManager.h"
#include "SpriteManager.h"

map<DWORD,LPCWCH> Display::ids;

BOOL Display::Action(void)
{
	return FALSE;
}

Display::Display(DWORD id,FPOINT pos)
{
	if(ids.empty())
	{
		ids[7] = TEXT("Image/drumtong.bmp");
        ids[6] = TEXT("Image/pipe.bmp");
        ids[11] = TEXT("Image/trash.bmp");
        ids[4] = TEXT("Image/poo.bmp");
        ids[5] = TEXT("Image/sohwa.bmp");
	}
	sprite.id = id;
	sprite.texture = TextureManager::GetInstance()->GetTexture(ids[id]);
	sprite.type = SpriteType::NONE;
	Init(pos);
}

HRESULT Display::Init(FPOINT pos)
{
	if(!sprite.texture)	
	{
		sprite.texture = TextureManager::GetInstance()->GetTexture(TEXT("Image/drumtong.bmp"));
		sprite.id = 7;
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
