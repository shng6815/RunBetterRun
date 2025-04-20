#pragma once
#include "GameObject.h"
#include "structs.h"

#define MAP_EDITOR_WIDTH  100       // 전체 맵의 가로 타일 수
#define MAP_EDITOR_HEIGHT 100       // 전체 맵의 세로 타일 수
#define VISIBLE_MAP_WIDTH  60       // 화면에 보이는 맵의 가로 타일 수 
#define VISIBLE_MAP_HEIGHT 60       // 화면에 보이는 맵의 세로 타일 수
#define TILE_SIZE 32                // 타일 하나의 픽셀 크기
#define SAMPLE_TILE_X 11            // 샘플 타일 가로 개수
#define SAMPLE_TILE_Y 11            // 샘플 타일 세로 개수

enum class EditorMode
{
	TILE,       // 타일 배치 모드
	START,      // 시작 위치 배치 모드
	ITEM,       // 아이템 배치 모드
	MONSTER     // 몬스터 배치 모드
};

// 에디터용 구조체
typedef struct EditorTile
{
	RECT rc;                // 타일 영역
	int tileIndex;          // 타일 인덱스
	RoomType type;          // 타일 유형
} editTile;

typedef struct UILayout {
	RECT mapViewArea;       // 맵이 보여지는 영역
	RECT sampleTileArea;    // 샘플 타일이 보여지는 영역
	RECT controlPanel;      // 버튼 등 컨트롤이 위치한 영역
	RECT statusBar;         // 상태 정보를 보여주는 영역
};

class Image;
class Button;

class MapEditor: public GameObject
{
private:
	EditorMode mode;
	POINT selectedTile;            // 선택된 샘플 타일 좌표
	RoomType selectedRoomType;     // 선택된 타일 유형

	Image* sampleTileImage;
	RECT sampleArea;
	RECT mapArea;                  // 맵 영역

	editTile mapTiles[MAP_EDITOR_HEIGHT][MAP_EDITOR_WIDTH];  // 맵 타일 배열
	vector<Sprite> editorSprites;  // 맵에 배치된 스프라이트
	vector<Button*> buttons;
	POINT mousePos;

	UILayout layout;
	POINT mapOffset;  // 맵 스크롤 오프셋 (맵의 어느 부분이 보이는지)
	float zoomLevel = 1.0f;

public:
	virtual HRESULT Init() override;
	virtual void Release() override;
	virtual void Update() override;
	virtual void Render(HDC hdc) override;

	void InitTiles();
	void InitButtons();
	void InitLayout(); 
	void HandleTileSelect();
	void HandleMapEdit();
	void HandleShortcut();

	void RenderTiles(HDC hdc);
	void RenderSampleTiles(HDC hdc);
	void RenderSprites(HDC hdc);

	void ChangeMode(EditorMode newMode);
	void PlaceTile(int x,int y);
	void PlaceStartPoint(int x,int y);
	void PlaceItem(int x,int y);
	void PlaceMonster(int x,int y);

	// 스프라이트 관리
	void AddSprite(FPOINT position,Texture* texture,SpriteType type);
	void RemoveSprite(int x,int y);
	int FindSprite(int x,int y);

	void SaveMap();
	void LoadMap();
	void ClearMap();

	void HandleScroll();              // 맵 스크롤 처리
	void HandleZoom();                // 확대/축소 처리
	POINT ScreenToMapCoord(POINT screenPos); // 화면 좌표를 맵 좌표로 변환

	int GetVisibleMapColumns();
	int GetVisibleMapRows();
};