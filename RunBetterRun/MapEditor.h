#pragma once
#include "GameObject.h"
#include "structs.h"

// 맵 에디터 샘플 타일 정보
constexpr int SAMPLE_TILE_X{11};      // 타일맵 가로 타일 개수
constexpr int SAMPLE_TILE_Y{11};     // 타일맵 세로 타일 개수
constexpr int TILE_SIZE{128};     // 타일 하나의 크기(픽셀)

// 에디터에서 편집할 맵 크기
constexpr int MAP_EDITOR_WIDTH{24};    // 맵 가로 타일 개수
constexpr int MAP_EDITOR_HEIGHT{24};    // 맵 세로 타일 개수

// 에디터 모드 정의
enum class EditorMode {
	TILE,    
	ITEM,     
	START,    
	MONSTER   
};

// 타일 정보
typedef struct tagEditorTile {
	RECT rc;
	int tileIndex;    // 타일 시트에서의 위치
	RoomType type;    
} EditorTile;

class Image;
class Button;

class MapEditor: public GameObject {
private:

	EditorMode mode;

	// 샘플 타일
	Image* sampleTileImage;    // 타일맵 이미지
	RECT rcSampleArea;         // 샘플 타일 영역
	POINT selectedTile;        // 선택된 타일 위치
	RoomType selectedRoomType; // 선택된 타일 유형

	// 맵 그리기 영역
	RECT rcMapArea;            // 맵 영역
	EditorTile mapTiles[MAP_EDITOR_HEIGHT][MAP_EDITOR_WIDTH]; // 맵 타일 정보

	// 아이템/몬스터
	vector<Sprite> editorSprites;  // 배치된 스프라이트들

	vector<Button*> buttons;   // 버튼들

	POINT mousePos;

public:
	virtual HRESULT Init() override;
	virtual void Release() override;
	virtual void Update() override;
	virtual void Render(HDC hdc) override;

	// 맵 관련 함수
	void InitTiles();
	void RenderTiles(HDC hdc);
	void RenderSampleTiles(HDC hdc);
	void RenderSprites(HDC hdc);

	// 에디터 모드 변경
	void ChangeMode(EditorMode newMode);

	void InitButtons();

	void PlaceTile(int x,int y);
	void PlaceItem(int x,int y);
	void PlaceMonster(int x,int y);
	void PlaceStartPoint(int x,int y);

	void AddSprite(FPOINT position,Texture* texture,SpriteType type);
	void RemoveSprite(int x,int y);
	int FindSpriteAt(int x,int y);

	void SaveMap();
	void LoadMap();
};