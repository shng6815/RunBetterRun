#pragma once
#include "GameObject.h"
#include "structs.h"

class MapEditorUI;
class MapEditorRender;

enum class EditorMode {
	TILE,
	START,
	ITEM,
	MONSTER,
	OBSTACLE
};

class MapEditor: public GameObject {
private:
	// 컴포넌트
	MapEditorUI* ui;
	MapEditorRender* render;

	// 에디터 상태
	EditorMode mode;
	RoomType selectedRoomType;
	Direction selectedObstacleDir;
	FPOINT startPosition;

	// 맵 데이터
	vector<Room> tiles;
	int mapWidth,mapHeight;
	vector<Sprite> editorSprites;
	vector<Obstacle> editorObstacles;

	// UI 리소스
	Image* sampleTileImage;
	RECT sampleArea;
	RECT mapArea;

	//마우스 스크롤, 확대/축소
	float zoomLevel;         // 확대/축소 비율
	FPOINT viewportOffset;   // 스크롤 오프셋 (맵의 어느 부분을 보여줄지)
	bool isDragging;       
	POINT lastMousePos;      

	// 내부 기능 함수들
	void TileSelect();
	void MapEdit();
	void Shortcut();

	// 배치 함수들
	void PlaceTile(int x,int y);
	void PlaceStartPoint(int x,int y);
	void PlaceItem(int x,int y);
	void PlaceMonster(int x,int y);
	void PlaceObstacle(int x,int y);

	// DataManager 연동
	void PrepareDataForSave();
	void LoadFromDataManager();

public:
	MapEditor();
	virtual ~MapEditor();

	virtual HRESULT Init() override;
	virtual void Release() override;
	virtual void Update() override;
	virtual void Render(HDC hdc) override;

	void InitTiles();
	void ChangeMode(EditorMode newMode);
	void ChangeObstacleDirection(Direction dir);
	
	// 드래그, 확대 /축소
	void Zoom(float delta);
	void StartDrag(POINT mousePos);
	void EndDrag();
	void UpdateDrag(POINT mousePos);

	// 스프라이트 관리
	int FindSprite(int x,int y);
	void AddSprite(FPOINT position,Texture* texture,SpriteType type);
	void RemoveSprite(int x,int y);

	// 장애물 관리
	int FindObstacle(int x,int y);
	void AddObstacle(POINT position,Texture* texture,Direction dir);
	void RemoveObstacle(int x,int y);

	void MouseWheel(int delta);     
	void VerticalScroll(int delta);  
	void HorizontalScroll(int delta); 

	// 파일 관리
	void SaveMap();
	void LoadMap();
	void ClearMap();
};