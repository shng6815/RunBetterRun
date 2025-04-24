#pragma once
#include "Singleton.h"
#include "structs.h"

class DataManager: public Singleton<DataManager>
{
private:
	FileHeader header;
	vector<Room> tiles;
	vector<ItemData> items;
	vector<MonsterData> monsters;
	vector<ObstacleData> obstacles;  // 장애물 데이터 목록

public:
	HRESULT Init();
	void Release();

	// 맵 파일 저장/로드
	bool SaveMapFile(LPCWCH filePath);
	bool LoadMapFile(LPCWCH filePath);

	// 에디터에서 데이터 설정
	void SetMapData(const vector<Room>& tiles,int width,int height);
	void SetTextureInfo(LPCWCH texturePath,DWORD tileSize,DWORD rowSize,DWORD columnSize);
	void SetStartPosition(const FPOINT& pos);
	void AddItemData(const ItemData& item);
	void AddMonsterData(const MonsterData& monster);
	void AddObstacleData(const ObstacleData& obstacle);
	void ClearAllData();

	// 게임에서 데이터 가져오기
	bool GetMapData(MapData& outMapData);
	const FPOINT& GetStartPosition() const { return header.startPos; }
	const vector<ItemData>& GetItems() const { return items; }
	const vector<MonsterData>& GetMonsters() const { return monsters; }
	const vector<ObstacleData>& GetObstacles() const { return obstacles; }
private:
	// 내부 사용 메서드
	bool ValidateHeader(const FileHeader& header);
	bool WriteHeader(HANDLE hFile);
	bool ReadHeader(HANDLE hFile);
	bool WriteTiles(HANDLE hFile);
	bool ReadTiles(HANDLE hFile);
	bool WriteItems(HANDLE hFile);
	bool ReadItems(HANDLE hFile);
	bool WriteMonsters(HANDLE hFile);
	bool ReadMonsters(HANDLE hFile);
	bool WriteObstacles(HANDLE hFile);
	bool ReadObstacles(HANDLE hFile);
};