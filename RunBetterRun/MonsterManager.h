#pragma once
#include "Singleton.h"
#include "config.h"
#include <vector>

using namespace std;
class Texture;
class Player;
class Monster;
class MonsterManager : public Singleton<MonsterManager>
{
private:
	Texture* monsterTexture;
	Player* playerPos;
	vector<Monster*> monsters;
	bool isCatchPlayer;

public:
	//TODO: 길 찾기 알고리즘
	void FindPlayer(Player* playerPos);

	//TODO: 텍스처 적용?
	void SetMonsterTexture(vector<Monster*> monsters);

	//TODO: 적이 플레이어를 잡았을 때의 메소드
	void CatchPlayer();

	//TODO:
	void SetisCatchPlayer(bool SetisCatchPlayer) { this->isCatchPlayer = isCatchPlayer; }
	bool GetisCatchPlayer() { return this->isCatchPlayer; }
};

