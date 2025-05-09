#include "Tentacle.h"
#include "TextureManager.h"
#include "SpriteManager.h"
#include "MainGameScene.h"
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
	// 플레이어의 생명력 감소
	Player* player = Player::GetInstance();

	this->sprite.aniInfo.currentFrame.y = 0;

	// 회전 애니메이션 시작
	MainGameScene* mainScene = dynamic_cast<MainGameScene*>(SceneManager::GetInstance()->currentScene);
	if(mainScene) {
		mainScene->StartMonsterCatchAnimation(sprite.pos);
	}

}

HRESULT Tentacle::Init(FPOINT pos)
{
	sprite.id = 100;
	sprite.texture = TextureManager::GetInstance()->GetTexture(TEXT("Image/Ballman.bmp"));
	if(!sprite.texture)
		return E_FAIL;
	sprite.type = SpriteType::MONSTER;
	sprite.pos = pos;
	sprite.distance = 0;
	//sprite.aniInfo = {0,0,{423,437},{1,1},{0,0}};
	sprite.aniInfo = {0.18f,0.18f,{215,246},{10,36},{0,0}};
	SpriteManager::GetInstance()->AddSprite(sprite);
	targetPosition = pos;
	speed = 1.5f;
	return S_OK;
}