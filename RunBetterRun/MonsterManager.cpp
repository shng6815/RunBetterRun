#include "MonsterManager.h"
#include "SpriteManager.h"
#include "Player.h"
#include "MapManager.h"
#include <cmath>

HRESULT MonsterManager::Init()
{
	playerPos = Player::GetInstance()->GetCameraPos();
	mapData = MapManager::GetInstance()->GetMapData();
	isCatchPlayer = false;

	Texture monsterTexture;
	SpriteManager::GetInstance()->LoadTexture(TEXT("Image/Rocket.bmp"), monsterTexture);

	CreateMonster({ playerPos.x + 2.0f, playerPos.y }, 0.5f);
	CreateMonster({ playerPos.x, playerPos.y + 2.0f }, 0.3f);

	// 즉시 스프라이트 등록
	for (auto& monster : monsters) {
		if (monster->GetIsActive()) {
			SpriteManager::GetInstance()->PutSprite(TEXT("Image/Rocket.bmp"), monster->GetPostion());
			// 디버깅용 로그
			FPOINT pos = monster->GetPostion();
			WCHAR buffer[100];
			swprintf_s(buffer, L"Monster created at: %.2f, %.2f\n", pos.x, pos.y);
			OutputDebugString(buffer);
		}
	}

	return S_OK;
}

void MonsterManager::Release()
{
	for (auto& monster : monsters)
	{
		monster->Release();
		delete monster;
	}
	monsters.clear();
}

void MonsterManager::Update()
{
	// 플레이어 위치 업데이트
	playerPos = Player::GetInstance()->GetCameraPos();
	float deltaTime = TimerManager::GetInstance()->GetDeltaTime();

	// 각 몬스터 업데이트
	for (auto& monster : monsters)
	{
		if (!monster->GetIsActive()) continue;

		// 플레이어 방향 계산
		FPOINT monsterPos = monster->GetPostion();
		FPOINT direction = GetDirectionToPlayer(monsterPos);

		// 다음 위치 계산
		FPOINT nextPos = {
			monsterPos.x + direction.x * monster->GetSpeed() * deltaTime,
			monsterPos.y + direction.y * monster->GetSpeed() * deltaTime
		};

		// 이동 가능 여부 확인
		if (CanMoveToPosition(nextPos))
		{
			monster->SetPosition(nextPos);
		}
	}
	
	
	// 이동 후 스프라이트 업데이트
	// 매 프레임마다 스프라이트를 지우고 다시 생성
	//SpriteManager::GetInstance()->ClearSprites();

	//// 몬스터 스프라이트 등록
	//for (auto& monster : monsters) {
	//	if (monster->GetIsActive()) {
	//		SpriteManager::GetInstance()->PutSprite(TEXT("Image/Rocket.bmp"), monster->GetPostion());
	//	}
	//}
	

	// 스프라이트 초기화는 첫 프레임에만 한 번 수행
	static bool isFirstFrame = true;
	if (isFirstFrame) {
		// 스프라이트 초기 등록
		for (auto& monster : monsters) {
			SpriteManager::GetInstance()->PutSprite(TEXT("Image/Rocket.bmp"), monster->GetPostion());
		}
		isFirstFrame = false;
	}
	else {
		//// 이후에는 위치만 업데이트
		//// 스프라이트 매니저에 관련 기능 추가 필요
		//SpriteManager::GetInstance()->ClearSprites();
		//for (auto& monster : monsters) {
		//	SpriteManager::GetInstance()->PutSprite(TEXT("Image/Rocket.bmp"), monster->GetPostion());
		//}
	}
}

void MonsterManager::FindPlayer()
{
}


void MonsterManager::CatchPlayer()
{
}

Monster* MonsterManager::CreateMonster(FPOINT position, float speed)
{
	Monster* monster = new Monster();
	monster->Init(position);
	monsters.push_back(monster);
	return monster;
}

void MonsterManager::RemoveMonster(int index)
{
	//TODO
}

void MonsterManager::RemoveAllMonsters()
{
	//TODO
}

FPOINT MonsterManager::GetDirectionToPlayer(const FPOINT& monsterPos)
{
	FPOINT direction = {
		playerPos.x - monsterPos.x,
		playerPos.y - monsterPos.y
	};

	// 정규화 (단위 벡터로 변환)
	float length = sqrt(direction.x * direction.x + direction.y * direction.y);
	if (length > 0.001f) {  // 0으로 나누기 방지
		direction.x /= length;
		direction.y /= length;
	}

	return direction;
}

bool MonsterManager::CanMoveToPosition(const FPOINT& pos)
{
	// 맵 경계 확인
	int x = static_cast<int>(pos.x);
	int y = static_cast<int>(pos.y);

	if (x < 0 || x >= MAP_COLUME || y < 0 || y >= MAP_ROW)
		return false;

	// 타일 타입 확인 (바닥인 경우만 이동 가능)
	if (!mapData) return false;

	Room& tile = mapData->tiles[y * mapData->width + x];
	return (tile.roomType == RoomType::FLOOR || tile.roomType == RoomType::START);
}
