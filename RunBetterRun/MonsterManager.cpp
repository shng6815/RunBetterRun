#include "MonsterManager.h"
#include "SpriteManager.h"
#include "TextureManager.h"
#include "Player.h"
#include "MapManager.h"
#include "SceneManager.h"
#include <cmath>

HRESULT MonsterManager::Init()
{
	playerPos = Player::GetInstance()->GetCameraPos();
	mapData = MapManager::GetInstance()->GetMapData();
	stunTime = 0;
	return S_OK;
}

void MonsterManager::Release()
{
	for (auto& monster : monsters)
		delete monster;
	monsters.clear();
}

void MonsterManager::Update()
{
	float deltaTime = TimerManager::GetInstance()->GetDeltaTime();
	if(stunTime > 0)
		stunTime -= deltaTime;
	else
	{
		playerPos = Player::GetInstance()->GetCameraPos();
		vector<FPOINT> paths;

		for(auto& monster : monsters)
		{
			FPOINT monsterPos = monster->GetPostion();
			vector<FPOINT> path = FindPath(monsterPos,playerPos,paths);

			if(path.size() >= 2)
			{
				monster->SetTargetPosition(path[1]);
				paths.insert(paths.end(),path.begin() + 1,path.end() - 1);
			}

			monster->Update();
		}
	}
}


void MonsterManager::PutMonster(AMonster* monster)
{
	monsters.push_back(monster);
}

vector<FPOINT> MonsterManager::FindPath(FPOINT start, FPOINT end, vector<FPOINT>& paths)
{
    // 결과 경로 저장
    vector<FPOINT> path;

    // 맵 데이터 확인
    if (!mapData) return path;

    // 시작점과 목표점의 정수 좌표
    int startX = static_cast<int>(start.x);
    int startY = static_cast<int>(start.y);
    int endX = static_cast<int>(end.x);
    int endY = static_cast<int>(end.y);

    // 시작점과 목표점이 맵 범위를 벗어나는지 확인
    if (startX < 0 || startX >= mapData->width || startY < 0 || startY >= mapData->height ||
        endX < 0 || endX >= mapData->width || endY < 0 || endY >= mapData->height)
        return path;

    // 시작점이나 목표점이 벽인 경우 처리
    if (mapData->tiles[startY * mapData->width + startX].roomType == RoomType::WALL ||
        mapData->tiles[endY * mapData->width + endX].roomType == RoomType::WALL)
        return path;

    // 열린 목록과 닫힌 목록 초기화
    vector<PathNode*> openList;
    vector<PathNode*> closedList;

    // 시작 노드 생성
    PathNode* startNode = new PathNode(startX, startY);
    startNode->h = CalculateHeuristic(startX, startY, endX, endY);
    startNode->f = startNode->h;

	for(auto& p : paths)
		closedList.push_back(new PathNode(p.x,p.y));

    // 열린 목록에 시작 노드 추가
    openList.push_back(startNode);

    // 상하좌우 이동 방향 (직선 이동만)
    const int dx[4] = { 0, 1, 0, -1 };
    const int dy[4] = { -1, 0, 1, 0 };

    // A* 알고리즘 메인 루프
    while (!openList.empty())
    {
        // f값이 가장 작은 노드 찾기
        auto it = min_element(openList.begin(), openList.end(), [](PathNode* a, PathNode* b) {
            return a->f < b->f;
            });

        PathNode* current = *it;

        // 목표 도달 확인
        if (current->x == endX && current->y == endY)
        {
            // 경로 역추적
            PathNode* temp = current;
            while (temp)
            {
                path.push_back({ static_cast<float>(temp->x) + 0.5f, static_cast<float>(temp->y) + 0.5f });
                temp = temp->parent;
            }

            // 경로 순서 뒤집기
            reverse(path.begin(), path.end());

            // 메모리 정리
            for (auto node : openList)
                delete node;
            for (auto node : closedList)
                delete node;

            // 경로 반환
            return path;
        }

        // 현재 노드를 열린 목록에서 제거하고 닫힌 목록에 추가
        openList.erase(it);
        closedList.push_back(current);

        // 상하좌우 이웃 노드 처리
        for (int i = 0; i < 4; i++)
        {
            int nx = current->x + dx[i];
            int ny = current->y + dy[i];

            //// 맵 경계 확인
            //if (nx < 0 || nx >= mapData->width || ny < 0 || ny >= mapData->height)
            //    continue;

            //// 이동 가능한 타일인지 확인 (FLOOR 또는 START)
            //Room& tile = mapData->tiles[ny * mapData->width + nx];
            //if (tile.roomType != RoomType::FLOOR && tile.roomType != RoomType::START)
            //    continue;

			if(!Move({current->x,current->y},{nx,ny}))
				continue;

            // 이미 닫힌 목록에 있는지 확인
            if (IsNodeInList(closedList, nx, ny))
                continue;

            // 새 g값 계산 (직선 이동은 비용 1)
            int newG = current->g + 1;

            // 이미 열린 목록에 있는지 확인
            PathNode* neighbor = GetNodeFromList(openList, nx, ny);

            if (!neighbor)
            {
                // 새 노드 생성
                neighbor = new PathNode(nx, ny);
                neighbor->g = newG;
                neighbor->h = CalculateHeuristic(nx, ny, endX, endY);
                neighbor->f = neighbor->g + neighbor->h;
                neighbor->parent = current;

                // 열린 목록에 추가
                openList.push_back(neighbor);
            }
            else if (newG < neighbor->g)
            {
                // 더 좋은 경로를 찾음
                neighbor->g = newG;
                neighbor->f = neighbor->g + neighbor->h;
                neighbor->parent = current;
            }
        }
    }

    // 경로를 찾지 못함, 메모리 정리
    for (auto node : openList)
        delete node;
    for (auto node : closedList)
        delete node;

    // 빈 경로 반환
    return path;
}

int MonsterManager::CalculateHeuristic(int x1, int y1, int x2, int y2)
{
	return abs(x1 - x2) + abs(y1 - y2);
}

bool MonsterManager::IsNodeInList(const vector<PathNode*>& list, int x, int y)
{
	for (auto node : list)
	{
		if (node->x == x && node->y == y)
			return true;
	}
	return false;
}

MonsterManager::PathNode* MonsterManager::GetNodeFromList(vector<PathNode*>& list, int x, int y)
{
	for (auto node : list)
	{
		if (node->x == x && node->y == y)
			return node;
	}
	return nullptr;
}

BOOL MonsterManager::Move(POINT src, POINT dst)
{
	if(dst.x < 0 || dst.x >= mapData->width
		|| dst.y < 0 || dst.y >= mapData->height)
		return FALSE;

	Room& tile = mapData->tiles[dst.y * mapData->width + dst.x];
	if (tile.roomType != RoomType::FLOOR && tile.roomType != RoomType::START)
		return FALSE;

	Obstacle* newMove = mapData->tiles[dst.y * mapData->width + dst.x].obstacle;
	Obstacle* oldMove = mapData->tiles[src.y * mapData->width + src.x].obstacle;
	
	if(newMove && newMove->block)
	{
		if((src.x < dst.x && newMove->dir == Direction::EAST)
			|| (src.x > dst.x && newMove->dir == Direction::WEST)
			|| (src.y < dst.y && newMove->dir == Direction::SOUTH)
			|| (src.y > dst.y && newMove->dir == Direction::NORTH))
			return FALSE;
	}
	
	if(oldMove && oldMove->block)
	{
		if((src.x < dst.x && oldMove->dir == Direction::WEST)
			|| (src.x > dst.x && oldMove->dir == Direction::EAST)
			|| (src.y < dst.y && oldMove->dir == Direction::NORTH)
			|| (src.y > dst.y && oldMove->dir == Direction::SOUTH))
			return FALSE;
	}

	return TRUE;
}

