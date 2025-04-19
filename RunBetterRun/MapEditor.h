#pragma once
#include "GameObject.h"
#include "structs.h"
#include <vector>

// 샘플 타일 정보
// 1408 x 1408
#define SAMPLE_TILE_X	11
#define SAMPLE_TILE_Y	11
#define TILE_SIZE	32

// 메인 그리기 공간 정보
#define TILE_X	20
#define TILE_Y	20

typedef struct tagTile
{
	RECT rc;
	int frameX;
	int frameY;
} TILE_INFO;

//맵 에디터 기능 
// 
//타일,몬스터,아이템을 선택해서 배치할 수 있어야 함.
//인게임 에디터 
//에디터에서 맵 저장/로드 기능 있어야함
class Image;
class Button;
class MapEditor : public GameObject
{
private:
	Image* sampleTile;
	RECT rcSampleTile;

	TILE_INFO tileInfo[TILE_X * TILE_Y];
	RECT rcMain;

	POINT selectedTile;
	vector<MapData*> Monsters;
	Button* saveButton;
	Button* loadButton;
	Button* randomGen;

public:
	virtual HRESULT Init() override;
	virtual void Release() override;
	virtual void Update() override;
	virtual void Render(HDC hdc) override;

	void Save();
	void Load();
	void RandomMapGenerate();

	MapEditor() {};
	virtual ~MapEditor() {};
};

