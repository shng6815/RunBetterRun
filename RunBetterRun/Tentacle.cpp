#include "Tentacle.h"
#include "TextureManager.h"
#include "SpriteManager.h"
#include "Player.h"


void Tentacle::Action(void)
{
	float dx = targetPosition.x - sprite.pos.x;
	float dy = targetPosition.y - sprite.pos.y;

	float distance = sqrt(dx*dx + dy*dy);
	if(distance > 0.01f) // 미세한 움직임은 무시
	{
		dy = -dy;

		float moveAngle = RAD_TO_DEG(atan2(dy,dx));

		if(moveAngle < 0)
			moveAngle += 360.0f;

		auto player = Player::GetInstance();
		FPOINT playerPos = player->GetCameraPos();
		FPOINT cameraDir = player->GetCameraVerDir();

		float pdx = playerPos.x - sprite.pos.x;
		float pdy = -(playerPos.y - sprite.pos.y);

		float viewAngle = RAD_TO_DEG(atan2(pdy,pdx));
		if(viewAngle < 0) viewAngle += 360.0f;

		float cameraAngle = RAD_TO_DEG(atan2(-cameraDir.y,cameraDir.x));
		if(cameraAngle < 0) cameraAngle += 360.0f;

		float angleDiff = (moveAngle - viewAngle);

		float finalAngle = (cameraAngle - viewAngle + angleDiff + 180.0f);

		while(finalAngle >= 360.0f) finalAngle -= 360.0f;
		if(finalAngle < 0) finalAngle += 360.0f;

		int frameY = static_cast<int>(finalAngle / 10.0f) % 36;

		sprite.aniInfo.currentFrame.y = frameY;
	}
}

void Tentacle::Catch(void)
{
	/*Player*player = Player::GetInstance();
	player->LossPlayerLife();

	if(player->GetPlayerLife() == 0)
	{
		SceneManager::GetInstance()->ChangeScene("DeadScene");
	}

	else {
		SceneManager::GetInstance()->ChangeScene("LossLifeScene");
	}*/
}

HRESULT Tentacle::Init(FPOINT pos)
{
	sprite.texture = TextureManager::GetInstance()->GetTexture(TEXT("Image/Enemy2.bmp"));
	if(!sprite.texture)
		return E_FAIL;
	sprite.type = SpriteType::MONSTER;
	sprite.pos = pos;
	sprite.distance = 0;
	//sprite.aniInfo = {0,0,{423,437},{1,1},{0,0}};
	sprite.aniInfo = {0.1f,0.1f,{431,495},{10,36},{0,0}};
	SpriteManager::GetInstance()->AddSprite(sprite);
	targetPosition = pos;
	speed = 1.5f;
	return S_OK;
}