#pragma once
#include "Singleton.h"
#include "config.h"
#include "AMonster.h"
#include "MapManager.h"
#include <vector>

#define MONSTER_SPEED 3.0f

using namespace std;

class MonsterManager : public Singleton<MonsterManager>
{
private:
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
	vector<AMonster*> monsters;
	MapData* mapData;

	FLOAT	stunTime;

	bool heartbeatPlaying = false;
	void UpdateHeartbeatSound(float distance);

	vector<FPOINT> FindPath(FPOINT start,FPOINT end, vector<FPOINT>& paths);
	int CalculateHeuristic(int x1,int y1,int x2,int y2);
	bool IsNodeInList(const vector<PathNode*>& list,int x,int y);
	PathNode* GetNodeFromList(vector<PathNode*>& list,int x,int y);
	BOOL Move(POINT src, POINT dst);

public:
	HRESULT Init();
	void Release();
	void Update();

	// 몬스터 생성 및 관리
	void PutMonster(AMonster* monster);

	void Stun(FLOAT time) {	stunTime += time; }
};

