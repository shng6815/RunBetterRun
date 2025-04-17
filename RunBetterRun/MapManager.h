#pragma once
#include "Singleton.h"
#include "structs.h"
#include <stack>
#include <algorithm>

#define MAP_COLUME 24
#define MAP_ROW 24

class MapManager : public Singleton<MapManager>
{
private:
	MapData mapData;
	void GenerateMaze(int startX, int startY, int width, int height); //미로 생성할때 쓸 함수
	void ShuffleDirections(int directions[4]);
public:
	HRESULT Init();					//기본	   맵 생성
	HRESULT Init(LPCWCH filePath);  //파일경로 맵 로드
	void Release();

	bool LoadMap(const LPCWCH filePath); 
	bool SaveMap(const LPCWCH filePath);
	
	bool CreateNewMap(int width, int height); //새로운 맵 생성
	bool CreateMazeMap(int width, int height); //미로 맵 생성
	bool CreateSimpleMazeMap(int width, int height);
	bool CreateFpsStyleMap(int width, int height);  // FPS 스타일 맵 생성
	void SetTile(int x, int y, RoomType tileType, int index); //특정타입의 타일 설정

	MapData* GetMapData();
};