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

		// 각도 정규화 (0-360도 범위로)
		if(moveAngle < 0)
			moveAngle += 360.0f;

		// 플레이어 카메라 정보 가져오기
		auto player = Player::GetInstance();
		FPOINT playerPos = player->GetCameraPos();
		FPOINT cameraDir = player->GetCameraVerDir();

		// 플레이어-텐타클 상대 위치 벡터 계산
		float pdx = playerPos.x - sprite.pos.x;
		float pdy = -(playerPos.y - sprite.pos.y); // Y축 뒤집기

		// 플레이어 시점에서 본 텐타클의 각도 계산
		float viewAngle = RAD_TO_DEG(atan2(pdy,pdx));
		if(viewAngle < 0) viewAngle += 360.0f;

		// 플레이어 카메라 방향 각도 계산
		float cameraAngle = RAD_TO_DEG(atan2(-cameraDir.y,cameraDir.x));
		if(cameraAngle < 0) cameraAngle += 360.0f;

		// 텐타클의 이동 방향과 플레이어 시점의 각도 차이 계산
		float angleDiff = (moveAngle - viewAngle)/2;

		// 최종 프레임 각도 계산 (카메라 각도 고려)
		float finalAngle = (cameraAngle - viewAngle + angleDiff + 180.0f);

		// 각도 정규화
		while(finalAngle >= 360.0f) finalAngle -= 360.0f;
		if(finalAngle < 0) finalAngle += 360.0f;

		// 프레임 인덱스 계산 (10도당 1프레임)
		int frameY = static_cast<int>(finalAngle / 10.0f) % 36;

		// 애니메이션 프레임 업데이트
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