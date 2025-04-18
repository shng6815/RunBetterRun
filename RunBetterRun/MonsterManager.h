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
	// A* 알고리즘을 위한 노드 구조체
	struct PathNode {
		int x, y;           // 위치
		int g;              // 시작점에서 현재 노드까지의 비용
		int h;              // 현재 노드에서 목표까지의 추정 비용
		int f;              // f = g + h
		PathNode* parent;   // 부모 노드

		PathNode(int x, int y) : x(x), y(y), g(0), h(0), f(0), parent(nullptr) {}
	};

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
	void FindPlayer(FPOINT monsterPos, FPOINT targetPos, float deletaTime);


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

	// TODO: Astar
	vector<FPOINT> FindPath(FPOINT start, FPOINT end);
	int CalculateHeuristic(int x1, int y1, int x2, int y2);
	bool IsNodeInList(const vector<PathNode*>& list, int x, int y);
	PathNode* GetNodeFromList(vector<PathNode*>& list, int x, int y);
};

