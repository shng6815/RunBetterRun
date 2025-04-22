#include "MapEditorRender.h"
#include "MapEditor.h" 
#include "MapEditorUI.h"

void MapEditorRender::Init(Image* sampleTileImage,int tileSize)
{
	this->sampleTileImage = sampleTileImage;
	this->tileSize = tileSize;
}

void MapEditorRender::RenderTiles(HDC hdc,const vector<Room>& tiles,int mapWidth,int mapHeight,
								 RECT mapArea,POINT mousePos,bool mouseInMapArea,
								 float zoomLevel,FPOINT viewportOffset)
{
	if(!sampleTileImage) return;


	// 정사각형 타일 크기를 보장하기 위한 계산
	int maxTilesX = VISIBLE_MAP_WIDTH / zoomLevel;
	int maxTilesY = VISIBLE_MAP_HEIGHT / zoomLevel;

	// 맵 영역 크기
	int mapAreaWidth = mapArea.right - mapArea.left;
	int mapAreaHeight = mapArea.bottom - mapArea.top;

	// 타일 크기를 계산 (가로, 세로 중 작은 값을 기준으로 정사각형 유지)
	int tileWidth = mapAreaWidth / maxTilesX;
	int tileHeight = mapAreaHeight / maxTilesY;
	int tileSize = min(tileWidth,tileHeight);  // 정사각형 보장

	// 정사각형 타일을 기준으로 실제 표시 가능한 타일 수 다시 계산
	int actualTilesX = mapAreaWidth / tileSize;
	int actualTilesY = mapAreaHeight / tileSize;

	// 맵 영역 중앙 정렬을 위한 오프셋 계산
	int offsetX = (mapAreaWidth - (actualTilesX * tileSize)) / 2;
	int offsetY = (mapAreaHeight - (actualTilesY * tileSize)) / 2;

	// 화면에 표시될 맵 영역 계산
	int startX = static_cast<int>(viewportOffset.x);
	int startY = static_cast<int>(viewportOffset.y);
	int endX = min(mapWidth,static_cast<int>(viewportOffset.x + actualTilesX) + 1);
	int endY = min(mapHeight,static_cast<int>(viewportOffset.y + actualTilesY) + 1);

	// 타일 렌더링
	for(int y = startY; y < endY; y++)
	{
		for(int x = startX; x < endX; x++)
		{
			// 화면 좌표 계산 (중앙 정렬 오프셋 포함)
			int screenX = mapArea.left + offsetX + static_cast<int>((x - viewportOffset.x) * tileSize);
			int screenY = mapArea.top + offsetY + static_cast<int>((y - viewportOffset.y) * tileSize);

			// 맵 인덱스 계산 및 범위 체크
			int index = y * mapWidth + x;
			if(index >= tiles.size()) continue;

			// 타일 정보 가져오기
			int tileIndex = tiles[index].tilePos;
			int frameX = tileIndex % SAMPLE_TILE_X;
			int frameY = tileIndex / SAMPLE_TILE_X;

			// RenderResized 함수 사용 (정사각형 타일 보장)
			sampleTileImage->RenderResized(
				hdc,
				screenX,
				screenY,
				tileSize,  // 정사각형 크기 사용
				tileSize,  // 정사각형 크기 사용
				frameX * TILE_SIZE,
				frameY * TILE_SIZE,
				TILE_SIZE,
				TILE_SIZE
			);

			// 마우스 위치에 있는 타일 표시
			if(mouseInMapArea)
			{
				POINT mapPos = ui->ScreenToMap(mousePos,mapArea,mapWidth,mapHeight,zoomLevel,viewportOffset);
				if(mapPos.x == x && mapPos.y == y)
				{
					HPEN selectPen = CreatePen(PS_SOLID,2,RGB(255,0,0));
					HPEN oldPen = (HPEN)SelectObject(hdc,selectPen);
					SelectObject(hdc,GetStockObject(NULL_BRUSH));

					Rectangle(hdc,
						screenX,
						screenY,
						screenX + tileSize,
						screenY + tileSize);

					SelectObject(hdc,oldPen);
					DeleteObject(selectPen);
				}
			}
		}
	}
}

void MapEditorRender::RenderSampleTiles(HDC hdc,RECT sampleArea,POINT selectedTile)
{
	if(!sampleTileImage) return;

	// 타일 크기 계산
	int sampleTileWidth = (sampleArea.right - sampleArea.left) / SAMPLE_TILE_X;
	int sampleTileHeight = (sampleArea.bottom - sampleArea.top) / SAMPLE_TILE_Y;

	// 정사각형 타일 보장
	int sampleTileSize = min(sampleTileWidth,sampleTileHeight);

	// 중앙 정렬 오프셋
	int offsetX = (sampleArea.right - sampleArea.left - (SAMPLE_TILE_X * sampleTileSize)) / 2;
	int offsetY = (sampleArea.bottom - sampleArea.top - (SAMPLE_TILE_Y * sampleTileSize)) / 2;

	// 타일 렌더링
	for(int y = 0; y < SAMPLE_TILE_Y; y++)
	{
		for(int x = 0; x < SAMPLE_TILE_X; x++)
		{
			// 타일 중앙 좌표 계산 (오프셋 적용)
			int posX = sampleArea.left + offsetX + x * sampleTileSize + sampleTileSize / 2;
			int posY = sampleArea.top + offsetY + y * sampleTileSize + sampleTileSize / 2;

			// 타일 렌더링
			sampleTileImage->FrameRender(hdc,posX,posY,x,y,false,true);

			// 선택된 타일 강조 표시
			if(x == selectedTile.x && y == selectedTile.y)
			{
				HPEN selectPen = CreatePen(PS_SOLID,3,RGB(255,0,0));
				HBRUSH selectBrush = CreateSolidBrush(RGB(255,230,230));

				HPEN oldPen = (HPEN)SelectObject(hdc,selectPen);
				HBRUSH oldBrush = (HBRUSH)SelectObject(hdc,selectBrush);

				Rectangle(hdc,
						 posX - sampleTileSize/2,
						 posY - sampleTileSize/2,
						 posX + sampleTileSize/2,
						 posY + sampleTileSize/2);

				SelectObject(hdc,oldPen);
				SelectObject(hdc,oldBrush);
				DeleteObject(selectPen);
				DeleteObject(selectBrush);
			}
		}
	}
}
	
void MapEditorRender::RenderSprites(HDC hdc,const vector<Sprite>& sprites,RECT mapArea,
								   float zoomLevel,FPOINT viewportOffset)
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

	for(const auto& sprite : sprites)
	{
		// 스프라이트 위치 (타일 좌표)
		float spriteX = sprite.pos.x;
		float spriteY = sprite.pos.y;

		// 뷰포트 상대적 위치 계산
		float relX = spriteX - viewportOffset.x;
		float relY = spriteY - viewportOffset.y;

		// 화면에 보이는지 확인
		if(relX >= -0.5f && relX < maxTilesX + 0.5f &&
		   relY >= -0.5f && relY < maxTilesY + 0.5f)
		{
			// 스크린 좌표 계산 - 정확한 위치 반영
			int screenX = mapArea.left + offsetX + static_cast<int>(relX * tileSize);
			int screenY = mapArea.top + offsetY + static_cast<int>(relY * tileSize);

			// 스프라이트 타입에 따라 색상 설정
			COLORREF color = (sprite.type == SpriteType::KEY) ? RGB(0,0,255) :
				(sprite.type == SpriteType::MONSTER) ? RGB(255,0,0) :
				RGB(128,128,128);

			// 스프라이트 렌더링 (원의 중심이 타일 중심에 오도록)
			int spriteSize = static_cast<int>(tileSize * 0.6f);
			HBRUSH spriteBrush = CreateSolidBrush(color);
			HBRUSH oldBrush = (HBRUSH)SelectObject(hdc,spriteBrush);

			Ellipse(hdc,
				   screenX - spriteSize/2,
				   screenY - spriteSize/2,
				   screenX + spriteSize/2,
				   screenY + spriteSize/2);

			SelectObject(hdc,oldBrush);
			DeleteObject(spriteBrush);
		}
	}
}

void MapEditorRender::RenderObstacles(HDC hdc,const vector<Obstacle>& obstacles,RECT mapArea,
									 float zoomLevel,FPOINT viewportOffset)
{
	// 타일 크기 계산 (정사각형 보장)
	int mapAreaWidth = mapArea.right - mapArea.left;
	int mapAreaHeight = mapArea.bottom - mapArea.top;

	int maxTilesX = VISIBLE_MAP_WIDTH / zoomLevel;
	int maxTilesY = VISIBLE_MAP_HEIGHT / zoomLevel;

	int tileWidth = mapAreaWidth / maxTilesX;
	int tileHeight = mapAreaHeight / maxTilesY;
	int tileSize = min(tileWidth,tileHeight);

	// 화면 중앙 정렬 오프셋
	int offsetX = (mapAreaWidth - (maxTilesX * tileSize)) / 2;
	int offsetY = (mapAreaHeight - (maxTilesY * tileSize)) / 2;

	for(const auto& obstacle : obstacles)
	{
		int tileX = obstacle.pos.x;
		int tileY = obstacle.pos.y;

		// 현재 보이는 영역 내에 있는지 확인
		if(tileX >= viewportOffset.x && tileX < viewportOffset.x + maxTilesX &&
		   tileY >= viewportOffset.y && tileY < viewportOffset.y + maxTilesY)
		{
			// 스크린 좌표 계산
			int screenX = mapArea.left + offsetX + static_cast<int>((tileX - viewportOffset.x) * tileSize);
			int screenY = mapArea.top + offsetY + static_cast<int>((tileY - viewportOffset.y) * tileSize);

			// 장애물 표시
			COLORREF color = RGB(255,128,0);  // 주황색
			HBRUSH obstacleBrush = CreateSolidBrush(color);
			HPEN obstaclePen = CreatePen(PS_SOLID,2,RGB(200,0,0));

			HBRUSH oldBrush = (HBRUSH)SelectObject(hdc,obstacleBrush);
			HPEN oldPen = (HPEN)SelectObject(hdc,obstaclePen);

			// 방향 표시 화살표 (크기는 타일 사이즈에 비례)
			int arrowSize = tileSize / 3;
			POINT arrowPoints[3];

			switch(obstacle.dir)
			{
			case Direction::NORTH:
			arrowPoints[0] = {screenX + tileSize/2,screenY + tileSize/4};
			arrowPoints[1] = {screenX + tileSize/4,screenY + tileSize*3/4};
			arrowPoints[2] = {screenX + tileSize*3/4,screenY + tileSize*3/4};
			break;
			case Direction::SOUTH:
			arrowPoints[0] = {screenX + tileSize/2,screenY + tileSize*3/4};
			arrowPoints[1] = {screenX + tileSize/4,screenY + tileSize/4};
			arrowPoints[2] = {screenX + tileSize*3/4,screenY + tileSize/4};
			break;
			case Direction::WEST:
			arrowPoints[0] = {screenX + tileSize/4,screenY + tileSize/2};
			arrowPoints[1] = {screenX + tileSize*3/4,screenY + tileSize/4};
			arrowPoints[2] = {screenX + tileSize*3/4,screenY + tileSize*3/4};
			break;
			case Direction::EAST:
			arrowPoints[0] = {screenX + tileSize*3/4,screenY + tileSize/2};
			arrowPoints[1] = {screenX + tileSize/4,screenY + tileSize/4};
			arrowPoints[2] = {screenX + tileSize/4,screenY + tileSize*3/4};
			break;
			}

			Polygon(hdc,arrowPoints,3);

			SelectObject(hdc,oldPen);
			SelectObject(hdc,oldBrush);
			DeleteObject(obstaclePen);
			DeleteObject(obstacleBrush);
		}
	}
}

void MapEditorRender::RenderStartPosition(HDC hdc,FPOINT startPos,const vector<Room>& tiles,
										 int mapWidth,RECT mapArea,float zoomLevel,FPOINT viewportOffset)
{
	// 타일 크기 계산 (정사각형 보장)
	int mapAreaWidth = mapArea.right - mapArea.left;
	int mapAreaHeight = mapArea.bottom - mapArea.top;

	int maxTilesX = VISIBLE_MAP_WIDTH / zoomLevel;
	int maxTilesY = VISIBLE_MAP_HEIGHT / zoomLevel;

	int tileWidth = mapAreaWidth / maxTilesX;
	int tileHeight = mapAreaHeight / maxTilesY;
	int tileSize = min(tileWidth,tileHeight);

	// 화면 중앙 정렬 오프셋
	int offsetX = (mapAreaWidth - (maxTilesX * tileSize)) / 2;
	int offsetY = (mapAreaHeight - (maxTilesY * tileSize)) / 2;

	// 시작 위치 계산
	int startX = static_cast<int>(startPos.x - 0.5f);
	int startY = static_cast<int>(startPos.y - 0.5f);

	// 현재 보이는 영역 내에 있는지 확인
	if(startX >= viewportOffset.x && startX < viewportOffset.x + maxTilesX &&
	   startY >= viewportOffset.y && startY < viewportOffset.y + maxTilesY)
	{
		// 스크린 좌표 계산
		int screenX = mapArea.left + offsetX + static_cast<int>((startX - viewportOffset.x) * tileSize);
		int screenY = mapArea.top + offsetY + static_cast<int>((startY - viewportOffset.y) * tileSize);

		// 시작 위치 표시
		HBRUSH greenBrush = CreateSolidBrush(RGB(0,255,0)); // 녹색
		HPEN greenPen = CreatePen(PS_SOLID,3,RGB(0,160,0));

		HBRUSH oldBrush = (HBRUSH)SelectObject(hdc,greenBrush);
		HPEN oldPen = (HPEN)SelectObject(hdc,greenPen);

		// 원 그리기 (크기는 타일 사이즈의 2/3)
		int circleSize = tileSize * 2/3;
		Ellipse(hdc,
			   screenX + (tileSize - circleSize) / 2,
			   screenY + (tileSize - circleSize) / 2,
			   screenX + (tileSize + circleSize) / 2,
			   screenY + (tileSize + circleSize) / 2);

		// "S" 표시 추가
		SetTextColor(hdc,RGB(0,0,0));
		SetBkMode(hdc,TRANSPARENT);
		HFONT font = CreateFont(tileSize/3,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,
							   DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
							   DEFAULT_QUALITY,DEFAULT_PITCH | FF_DONTCARE,TEXT("Arial"));
		HFONT oldFont = (HFONT)SelectObject(hdc,font);

		RECT textRect = {
			screenX + tileSize/4,
			screenY + tileSize/4,
			screenX + tileSize*3/4,
			screenY + tileSize*3/4
		};
		DrawText(hdc,TEXT("S"),-1,&textRect,DT_CENTER | DT_VCENTER | DT_SINGLELINE);

		SelectObject(hdc,oldFont);
		DeleteObject(font);
		SelectObject(hdc,oldPen);
		SelectObject(hdc,oldBrush);
		DeleteObject(greenPen);
		DeleteObject(greenBrush);
	}
}

void MapEditorRender::RenderModeInfo(HDC hdc,EditorMode mode, RoomType selectedRoomType,float zoomLevel)
{
	TCHAR szText[128];
	LPCWSTR modeName = TEXT("Tile");
	COLORREF modeColor = RGB(255,255,255);

	if(mode == EditorMode::TILE) {
		modeName = TEXT("TILE MODE (select tile)");
		modeColor = RGB(255,255,0);
	} else if(mode == EditorMode::START) {
		modeName = TEXT("START MODE (place player start position)");
		modeColor = RGB(120,255,120);
	} else if(mode == EditorMode::ITEM) {
		modeName = TEXT("ITEM MODE (place item)");
		modeColor = RGB(200,200,255);
	} else if(mode == EditorMode::MONSTER) {
		modeName = TEXT("MONSTER MODE (place monster)");
		modeColor = RGB(255,100,100);
	} else if(mode == EditorMode::OBSTACLE) {
		modeName = TEXT("OBSTACLE MODE (place obstacle)");
		modeColor = RGB(255,160,0);
	}

	// 모드 텍스트 출력 (폰트 크기 증가)
	SetTextColor(hdc,modeColor);
	HFONT hFont = CreateFont(24,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,
						  DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
						  DEFAULT_QUALITY,DEFAULT_PITCH | FF_DONTCARE,TEXT("Arial"));
	HFONT oldFont = (HFONT)SelectObject(hdc,hFont);

	wsprintf(szText,TEXT("Current Mode: %s"),modeName);
	TextOut(hdc,20,20,szText,lstrlen(szText));

	// 선택된 타일 타입 표시 추가
	LPCWSTR roomTypeName;
	COLORREF roomTypeColor;

	switch(selectedRoomType) {
	case RoomType::FLOOR:
	roomTypeName = TEXT("FLOOR");
	roomTypeColor = RGB(0,255,0);
	break;
	case RoomType::WALL:
	roomTypeName = TEXT("WALL");
	roomTypeColor = RGB(0,255,0);
	break;
	case RoomType::START:
	roomTypeName = TEXT("START");
	roomTypeColor = RGB(0,255,0); 
	break;
	default:
	roomTypeName = TEXT("NONE");
	roomTypeColor = RGB(0,255,0); 
	}

	SetTextColor(hdc,roomTypeColor);
	wsprintf(szText,TEXT("Tile Type: %s"),roomTypeName);
	TextOut(hdc,20,50,szText,lstrlen(szText));

	// 확대/축소 비율 표시 추가
	TCHAR szZoom[32];
	wsprintf(szZoom,TEXT("Zoom: %.0f%%"),zoomLevel * 100.0f);
	TextOut(hdc,20,80,szZoom,lstrlen(szZoom));

	SelectObject(hdc,oldFont);
	DeleteObject(hFont);
	SetTextColor(hdc,RGB(0,0,0)); 
}

void MapEditorRender::RenderControlGuide(HDC hdc)
{
	// 조작 설명 표시 (하단에 명확하게)
	HFONT guideFont = CreateFont(18,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,
						  DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
						  DEFAULT_QUALITY,DEFAULT_PITCH | FF_DONTCARE,TEXT("Arial"));
	HFONT oldFont = (HFONT)SelectObject(hdc,guideFont);

	// 검은색 배경에 흰색 텍스트로 표시하여 가독성 높이기
	RECT guideRect = {0,WINSIZE_Y - 80,WINSIZE_X,WINSIZE_Y};
	FillRect(hdc,&guideRect,(HBRUSH)GetStockObject(BLACK_BRUSH));
	SetTextColor(hdc,RGB(255,255,255));

	TextOut(hdc,20,TILEMAPTOOL_Y - 70,TEXT("Left Click: Place / Right Click: Delete"),
			lstrlen(TEXT("Left Click: Place / Right Click: Delete")));
	TextOut(hdc,20,TILEMAPTOOL_Y - 50,TEXT("ESC: Return to Game, Arrow Keys: Change Obstacle Direction"),
			lstrlen(TEXT("ESC: Return to Game, Arrow Keys: Change Obstacle Direction")));
	TextOut(hdc,20,TILEMAPTOOL_Y - 30,TEXT("1-5: Change Mode, S: Save, L: Load, C: Clear, F: Floor, W: Wall, G: Start"),
			lstrlen(TEXT("1-5: Change Mode, S: Save, L: Load, C: Clear, F: Floor, W: Wall, G: Start")));

	SelectObject(hdc,oldFont);
	DeleteObject(guideFont);
}

void MapEditorRender::RenderSelectedTilePreview(HDC hdc,POINT selectedTile,RECT sampleArea)
{
	if(!sampleTileImage) return;

	int selectedPreviewSize = 64; // 선택된 타일 미리보기 크기
	RECT selectedTileRect = {
		sampleArea.left,
		sampleArea.bottom + 20, // 여백 증가
		sampleArea.left + selectedPreviewSize,
		sampleArea.bottom + 20 + selectedPreviewSize
	};

	// 선택된 타일 배경 및 테두리
	Rectangle(hdc,selectedTileRect.left - 1,selectedTileRect.top - 1,
			 selectedTileRect.right + 1,selectedTileRect.bottom + 1);

	// 선택된 타일 렌더링
	sampleTileImage->FrameRender(
		hdc,
		(selectedTileRect.left + selectedTileRect.right) / 2,
		(selectedTileRect.top + selectedTileRect.bottom) / 2,
		selectedTile.x,selectedTile.y);
}

MapEditorRender::MapEditorRender(): sampleTileImage(nullptr),tileSize(TILE_SIZE)
{}

MapEditorRender::~MapEditorRender()
{}
