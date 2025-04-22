#pragma once
#include "structs.h"
#include "Image.h"

enum class EditorMode;
class MapEditorUI;

class MapEditorRender {
private:
	Image* sampleTileImage;  
	int tileSize;           
	MapEditorUI* ui;

public:
	void Init(Image* sampleTileImage,int tileSize);

	// 타일 관련 렌더링
	void RenderTiles(HDC hdc,const vector<Room>& tiles,int mapWidth,int mapHeight,
					RECT mapArea,POINT mousePos,bool mouseInMapArea,
					float zoomLevel = 1.0f,FPOINT viewportOffset = {0.0f,0.0f});
	void RenderSampleTiles(HDC hdc,RECT sampleArea,POINT selectedTile);

	// 배치 요소 렌더링
	void RenderSprites(HDC hdc,const vector<Sprite>& sprites,RECT mapArea,float zoomLevel,FPOINT viewportOffset);
	void RenderObstacles(HDC hdc,const vector<Obstacle>& obstacles,RECT mapArea,
						float zoomLevel,FPOINT viewportOffset);

	// 시작 위치 렌더링
	void RenderStartPosition(HDC hdc,FPOINT startPos,const vector<Room>& tiles,int mapWidth,RECT mapArea,
							float zoomLevel,FPOINT viewportOffset);

	// UI 요소 렌더링
	void RenderModeInfo(HDC hdc,EditorMode mode, RoomType selectedRoomType,float zoomLevel = 1.0f);
	void RenderControlGuide(HDC hdc);
	void RenderSelectedTilePreview(HDC hdc,POINT selectedTile,RECT sampleArea);

	Image* GetSampleTileImage() const {
		return sampleTileImage;
	}

	MapEditorRender();
	~MapEditorRender();
};