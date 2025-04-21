#pragma once
#include "structs.h"
#include "Image.h"

// 전방 선언
enum class EditorMode;

class MapEditorRenderer {
private:
	Image* sampleTileImage;  // 샘플 타일 이미지
	int tileSize;            // 타일 사이즈

public:
	MapEditorRenderer();
	~MapEditorRenderer();

	// 초기화 함수
	void Init(Image* sampleTileImage,int tileSize);

	// 타일 관련 렌더링
	void RenderTiles(HDC hdc,const vector<Room>& tiles,int mapWidth,int mapHeight,
					RECT mapArea,POINT mousePos,bool mouseInMapArea);
	void RenderSampleTiles(HDC hdc,RECT sampleArea,POINT selectedTile);

	// 엔티티 렌더링
	void RenderSprites(HDC hdc,const vector<Sprite>& sprites,RECT mapArea);
	void RenderObstacles(HDC hdc,const vector<EditorObstacle>& obstacles,RECT mapArea);

	// 특수 위치 렌더링
	void RenderStartPosition(HDC hdc,FPOINT startPos,const vector<Room>& tiles,
							int mapWidth,RECT mapArea);

	// UI 요소 렌더링
	void RenderModeInfo(HDC hdc,EditorMode mode);
	void RenderControlGuide(HDC hdc);
	void RenderSelectedTilePreview(HDC hdc,POINT selectedTile,RECT sampleArea);

	Image* GetSampleTileImage() const {
		return sampleTileImage;
	}
};