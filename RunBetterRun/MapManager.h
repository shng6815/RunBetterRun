#pragma once
#include "Singleton.h"
#include "structs.h"

#define MAP_COLUME 24
#define MAP_ROW 24

class MapManager : public Singleton<MapManager>
{
private:
	MapData mapData;

public:
	HRESULT Init();					//기본	   맵 생성
	HRESULT Init(LPCWCH filePath);  //파일경로 맵 로드
	void Release();

	bool LoadMap(const LPCWCH filePath);
	bool SaveMap(const LPCWCH filePath);
	
	bool CreateNewMap(int width, int height); //새로운 맵 생성
	bool CreateMazeMap(int width, int height);
	void SetTile(int x, int y, RoomType tileType, int index); //특정타입의 타일 설정

	MapData* GetMapData();
};