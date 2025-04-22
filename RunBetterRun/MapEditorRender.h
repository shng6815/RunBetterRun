#pragma once
#include "structs.h"
#include "Image.h"

enum class EditorMode;

class MapEditorRender {
private:
	Image* sampleTileImage;  
	int tileSize;           

public:
	void Init(Image* sampleTileImage,int tileSize);

	// 타일 관련 렌더링
	void RenderTiles(HDC hdc,const vector<Room>& tiles,int mapWidth,int mapHeight,
					RECT mapArea,POINT mousePos,bool mouseInMapArea);
	void RenderSampleTiles(HDC hdc,RECT sampleArea,POINT selectedTile);

	// 배치 요소 렌더링
	void RenderSprites(HDC hdc,const vector<Sprite>& sprites,RECT mapArea);
	void RenderObstacles(HDC hdc,const vector<Obstacle>& obstacles,RECT mapArea);

	// 시작 위치 렌더링
	void RenderStartPosition(HDC hdc,FPOINT startPos,const vector<Room>& tiles,
							int mapWidth,RECT mapArea);

	// UI 요소 렌더링
	void RenderModeInfo(HDC hdc,EditorMode mode, RoomType selectedRoomType);
	void RenderControlGuide(HDC hdc);
	void RenderSelectedTilePreview(HDC hdc,POINT selectedTile,RECT sampleArea);

	Image* GetSampleTileImage() const {
		return sampleTileImage;
	}

	MapEditorRender();
	~MapEditorRender();
};