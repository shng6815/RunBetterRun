#pragma once
#include "Singleton.h"
#include "config.h"
#include "Monster.h"
#include "MapManager.h"
#include <vector>

using namespace std;

class MonsterManager : public Singleton<MonsterManager>
{
private:
	FPOINT playerPos;
	vector<Monster*> monsters;
	bool isCatchPlayer;
	MapData* mapData;

public:
	HRESULT Init();
	void Release();
	void Update();
	//TODO: 길 찾기 알고리즘
	void FindPlayer();


	//TODO: 적이 플레이어를 잡았을 때의 메소드
	void CatchPlayer();

	//TODO:
	void SetisCatchPlayer(bool SetisCatchPlayer) { this->isCatchPlayer = isCatchPlayer; }
	bool GetisCatchPlayer() { return this->isCatchPlayer; }

	// 몬스터 생성 및 관리
	Monster* CreateMonster(FPOINT position, float speed = 0.5f);
	void RemoveMonster(int index);
	void RemoveAllMonsters();

	// 간단한 경로 찾기 (직선 이동)
	FPOINT GetDirectionToPlayer(const FPOINT& monsterPos);
	bool CanMoveToPosition(const FPOINT& pos);
};

