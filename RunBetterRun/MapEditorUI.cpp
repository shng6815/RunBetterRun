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
			// 샘플 영역 크기
			int sampleWidth = sampleArea.right - sampleArea.left;
			int sampleHeight = sampleArea.bottom - sampleArea.top;

			// 정사각형 타일 크기 계산
			int sampleTileWidth = sampleWidth / SAMPLE_TILE_X;
			int sampleTileHeight = sampleHeight / SAMPLE_TILE_Y;
			int sampleTileSize = min(sampleTileWidth,sampleTileHeight);

			// 중앙 정렬 오프셋
			int offsetX = (sampleWidth - (SAMPLE_TILE_X * sampleTileSize)) / 2;
			int offsetY = (sampleHeight - (SAMPLE_TILE_Y * sampleTileSize)) / 2;

			// 상대 좌표 계산 (오프셋 고려)
			int relX = mousePos.x - (sampleArea.left + offsetX);
			int relY = mousePos.y - (sampleArea.top + offsetY);

			// 타일 인덱스 계산
			selectedTile.x = relX / sampleTileSize;
			selectedTile.y = relY / sampleTileSize;

			// 범위를 벗어나지 않도록 보정
			selectedTile.x = max(0,min(selectedTile.x,SAMPLE_TILE_X - 1));
			selectedTile.y = max(0,min(selectedTile.y,SAMPLE_TILE_Y - 1));

			// 디버그 출력
			TCHAR szDebug[100];
			wsprintf(szDebug,L"선택된 타일: (%d, %d)\n",selectedTile.x,selectedTile.y);
			OutputDebugString(szDebug);

			return true;
		}
	}
	return false;
}

POINT MapEditorUI::ScreenToMap(POINT screenPos,RECT mapArea,int mapWidth,int mapHeight,
							  float zoomLevel,FPOINT viewportOffset) const
{
	POINT result = {0,0};

	// 정사각형 타일 크기 계산
	int mapAreaWidth = mapArea.right - mapArea.left;
	int mapAreaHeight = mapArea.bottom - mapArea.top;

	int maxTilesX = VISIBLE_MAP_WIDTH / zoomLevel;
	int maxTilesY = VISIBLE_MAP_HEIGHT / zoomLevel;

	int tileWidth = mapAreaWidth / maxTilesX;
	int tileHeight = mapAreaHeight / maxTilesY;
	int tileSize = min(tileWidth,tileHeight);

	// 중앙 정렬 오프셋
	int offsetX = (mapAreaWidth - (maxTilesX * tileSize)) / 2;
	int offsetY = (mapAreaHeight - (maxTilesY * tileSize)) / 2;

	// 스크린 좌표가 맵 영역 안에 있는지 확인
	if(screenPos.x >= mapArea.left + offsetX &&
	   screenPos.x < mapArea.right - offsetX &&
	   screenPos.y >= mapArea.top + offsetY &&
	   screenPos.y < mapArea.bottom - offsetY)
	{
		// 맵 영역 내 상대 좌표 계산 (오프셋 고려)
		float relX = static_cast<float>(screenPos.x - (mapArea.left + offsetX)) / tileSize;
		float relY = static_cast<float>(screenPos.y - (mapArea.top + offsetY)) / tileSize;

		// 타일 좌표 계산 (뷰포트 오프셋 고려)
		result.x = static_cast<int>(viewportOffset.x + relX);
		result.y = static_cast<int>(viewportOffset.y + relY);

		// 범위를 벗어나지 않도록 보정
		result.x = max(0,min(result.x,mapWidth - 1));
		result.y = max(0,min(result.y,mapHeight - 1));

		// 디버그 출력
		TCHAR szDebug[100];
		wsprintf(szDebug,L"ScreenToMap: 화면(%d, %d) -> 상대(%.2f, %.2f) -> 맵(%d, %d)\n",
				 screenPos.x,screenPos.y,relX,relY,result.x,result.y);
		OutputDebugString(szDebug);
	}

	return result;
}

POINT MapEditorUI::MapToScreen(POINT mapPos,RECT mapArea,int mapWidth,int mapHeight,
								float zoomLevel,FPOINT viewportOffset) const
{
	POINT result = {0,0};

	// 맵 좌표가 유효한 범위 내에 있는지 확인
	if(mapPos.x >= 0 && mapPos.x < mapWidth &&
	   mapPos.y >= 0 && mapPos.y < mapHeight)
	{
		// 정사각형 타일 크기 계산
		int mapAreaWidth = mapArea.right - mapArea.left;
		int mapAreaHeight = mapArea.bottom - mapArea.top;

		int maxTilesX = VISIBLE_MAP_WIDTH / zoomLevel;
		int maxTilesY = VISIBLE_MAP_HEIGHT / zoomLevel;

		int tileWidth = mapAreaWidth / maxTilesX;
		int tileHeight = mapAreaHeight / maxTilesY;
		int tileSize = min(tileWidth,tileHeight);

		// 중앙 정렬 오프셋
		int offsetX = (mapAreaWidth - (maxTilesX * tileSize)) / 2;
		int offsetY = (mapAreaHeight - (maxTilesY * tileSize)) / 2;

		// 맵 좌표를 스크린 좌표로 변환 (오프셋 및 뷰포트 고려)
		float relX = mapPos.x - viewportOffset.x;
		float relY = mapPos.y - viewportOffset.y;

		result.x = mapArea.left + offsetX + static_cast<int>(relX * tileSize);
		result.y = mapArea.top + offsetY + static_cast<int>(relY * tileSize);

		// 디버그 출력
		TCHAR szDebug[100];
		wsprintf(szDebug,L"MapToScreen: 맵(%d, %d) -> 상대(%.2f, %.2f) -> 화면(%d, %d)\n",
				 mapPos.x,mapPos.y,relX,relY,result.x,result.y);
		OutputDebugString(szDebug);
	}

	return result;
}