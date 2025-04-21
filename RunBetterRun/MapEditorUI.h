#pragma once
#include "structs.h"

class MapEditorUI {
private:
	RECT sampleArea;       // 샘플 타일 영역
	RECT mapArea;          // 맵 편집 영역
	POINT mousePos;        // 마우스 위치
	bool mouseInMapArea;   // 마우스가 맵 영역에 있는지
	POINT selectedTile;    // 선택된 타일

public:
	MapEditorUI();
	~MapEditorUI();

	// 초기화 함수
	void Init(RECT sampleArea,RECT mapArea);

	// 마우스 관련 기능
	void UpdateMousePosition(POINT mousePos);
	bool IsMouseInMapArea() const {
		return mouseInMapArea;
	}

	// 타일 선택 관련
	POINT GetSelectedTile() const {
		return selectedTile;
	}
	void SetSelectedTile(POINT tile) {
		selectedTile = tile;
	}
	bool HandleTileSelection(POINT mousePos,RECT sampleArea);

	// 좌표 변환 기능
	POINT ScreenToMap(POINT screenPos,RECT mapArea,int mapWidth,int mapHeight) const;
	POINT MapToScreen(POINT mapPos,RECT mapArea,int mapWidth,int mapHeight) const;

	// Getter/Setter
	RECT GetSampleArea() const {
		return sampleArea;
	}
	RECT GetMapArea() const {
		return mapArea;
	}
	POINT GetMousePos() const {
		return mousePos;
	}
};