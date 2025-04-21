#include "MapEditorUI.h"
#include "KeyManager.h"

MapEditorUI::MapEditorUI(): mouseInMapArea(false)
{
	mousePos = {0,0};
	selectedTile = {0,0};
}

MapEditorUI::~MapEditorUI()
{
}

void MapEditorUI::Init(RECT sampleArea,RECT mapArea)
{
	this->sampleArea = sampleArea;
	this->mapArea = mapArea;
}

void MapEditorUI::UpdateMousePosition(POINT mousePos)
{
	this->mousePos = mousePos;
	mouseInMapArea = PtInRect(&mapArea,mousePos);
}

bool MapEditorUI::HandleTileSelection(POINT mousePos,RECT sampleArea)
{
	if(PtInRect(&sampleArea,mousePos))
	{
		if(KeyManager::GetInstance()->IsOnceKeyDown(VK_LBUTTON))
		{
			int relX = mousePos.x - sampleArea.left;
			int relY = mousePos.y - sampleArea.top;

			// 샘플 타일 크기 계산
			int sampleWidth = sampleArea.right - sampleArea.left;
			int sampleHeight = sampleArea.bottom - sampleArea.top;
			int sampleTileSize = sampleWidth / SAMPLE_TILE_X;

			selectedTile.x = relX / sampleTileSize;
			selectedTile.y = relY / sampleTileSize;

			// 범위를 벗어나지 않도록 보정
			selectedTile.x = max(0,min(selectedTile.x,SAMPLE_TILE_X - 1));
			selectedTile.y = max(0,min(selectedTile.y,SAMPLE_TILE_Y - 1));

			return true;
		}
	}
	return false;
}

POINT MapEditorUI::ScreenToMap(POINT screenPos,RECT mapArea,int mapWidth,int mapHeight) const
{
	POINT result = {0,0};

	// 스크린 좌표가 맵 영역 안에 있는지 확인
	if(screenPos.x >= mapArea.left && screenPos.x < mapArea.right &&
		screenPos.y >= mapArea.top && screenPos.y < mapArea.bottom)
	{
		// 맵 영역 내 상대 좌표 계산
		int relX = screenPos.x - mapArea.left;
		int relY = screenPos.y - mapArea.top;

		// 타일 크기 계산
		int tileWidth = (mapArea.right - mapArea.left) / VISIBLE_MAP_WIDTH;
		int tileHeight = (mapArea.bottom - mapArea.top) / VISIBLE_MAP_HEIGHT;

		// 타일 좌표 직접 계산
		result.x = relX / tileWidth;
		result.y = relY / tileHeight;

		// 범위를 벗어나지 않도록 보정
		result.x = max(0,min(result.x,mapWidth - 1));
		result.y = max(0,min(result.y,mapHeight - 1));
	}

	return result;
}

POINT MapEditorUI::MapToScreen(POINT mapPos,RECT mapArea,int mapWidth,int mapHeight) const
{
	POINT result = {0,0};

	// 맵 좌표가 유효한 범위 내에 있는지 확인
	if(mapPos.x >= 0 && mapPos.x < mapWidth &&
		mapPos.y >= 0 && mapPos.y < mapHeight)
	{
		// 타일 크기 계산
		int tileWidth = (mapArea.right - mapArea.left) / VISIBLE_MAP_WIDTH;
		int tileHeight = (mapArea.bottom - mapArea.top) / VISIBLE_MAP_HEIGHT;

		// 맵 좌표를 스크린 좌표로 변환 (타일 중앙 기준)
		result.x = mapArea.left + mapPos.x * tileWidth + (tileWidth / 2);
		result.y = mapArea.top + mapPos.y * tileHeight + (tileHeight / 2);
	}
	else
	{
		// 유효하지 않은 맵 좌표인 경우 맵 영역 밖으로 설정
		result.x = mapArea.left - 100;
		result.y = mapArea.top - 100;
	}

	return result;
}