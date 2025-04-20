#include "Tentacle.h"
#include "TextureManager.h"
#include "SpriteManager.h"
#include "Player.h"

void Tentacle::Action(void)
{
	
}

void Tentacle::Catch(void)
{
	Player*player = Player::GetInstance();
	player->LossPlayerLife();

	if(player->GetPlayerLife() <= 0)
	{
		SceneManager::GetInstance()->ChangeScene("DeadScene");
	}

	else {
		SceneManager::GetInstance()->ChangeScene("LossLifeScene");
	}
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
	speed = 0.5f;
	return S_OK;
}
