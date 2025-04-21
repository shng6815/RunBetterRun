#include "MapEditorRender.h"
#include "MapEditor.h" 

void MapEditorRender::Init(Image* sampleTileImage,int tileSize)
{
	this->sampleTileImage = sampleTileImage;
	this->tileSize = tileSize;
}

void MapEditorRender::RenderTiles(HDC hdc,const vector<Room>& tiles,int mapWidth,int mapHeight,
								  RECT mapArea,POINT mousePos,bool mouseInMapArea)
{
	if(!sampleTileImage) return;

	int visibleWidth = VISIBLE_MAP_WIDTH;
	int visibleHeight = VISIBLE_MAP_HEIGHT;

	int tileWidth = (mapArea.right - mapArea.left) / visibleWidth;
	int tileHeight = (mapArea.bottom - mapArea.top) / visibleHeight;

	// 화면에 표시될 타일 수 계산
	for(int y = 0; y < visibleHeight; y++)
	{
		for(int x = 0; x < visibleWidth; x++)
		{
			// 타일 중앙 좌표 계산
			int screenX = mapArea.left + x * tileWidth + (tileWidth / 2);
			int screenY = mapArea.top + y * tileHeight + (tileHeight / 2);

			// 1차원 배열에서 해당 타일의 인덱스 계산
			int index = y * mapWidth + x;
			if(index >= tiles.size()) continue;  // 타일 범위를 벗어나면 건너뜀

			// 해당 타일의 타입 정보 가져오기
			int tileIndex = tiles[index].tilePos;

			// 타일시트에서의 프레임 좌표 계산
			int frameX = tileIndex % SAMPLE_TILE_X;  // 열(X) 계산
			int frameY = tileIndex / SAMPLE_TILE_X;  // 행(Y) 계산

			// 타일 이미지 렌더
			sampleTileImage->FrameRender(
				hdc,
				screenX,
				screenY,
				frameX,frameY,
				false,
				true
			);

			// 현재 편집 중인 타일 표시 (마우스 위치에 있는 타일)
			if(mouseInMapArea)
			{
				// 마우스 좌표를 타일 인덱스로 변환
				int mouseMapX = (mousePos.x - mapArea.left) * visibleWidth / (mapArea.right - mapArea.left);
				int mouseMapY = (mousePos.y - mapArea.top) * visibleHeight / (mapArea.bottom - mapArea.top);

				bool isEdge = (x == 0 || x == mapWidth - 1 || y == 0 || y == mapHeight - 1);

				if(isEdge)
				{
					HPEN edgePen = CreatePen(PS_SOLID,2,RGB(255,0,0));  // 빨간색 테두리
					HPEN oldPen = (HPEN)SelectObject(hdc,edgePen);
					SelectObject(hdc,GetStockObject(NULL_BRUSH));

					RECT edgeRect = {
						screenX - tileWidth/2,
						screenY - tileHeight/2,
						screenX + tileWidth/2,
						screenY + tileHeight/2
					};

					Rectangle(hdc,edgeRect.left,edgeRect.top,edgeRect.right,edgeRect.bottom);

					SelectObject(hdc,oldPen);
					DeleteObject(edgePen);
				}
			}
		}
	}
}

void MapEditorRender::RenderSampleTiles(HDC hdc,RECT sampleArea,POINT selectedTile)
{
	if(!sampleTileImage) return;

	// 타일 크기 계산 수정
	int sampleTileWidth = (sampleArea.right - sampleArea.left) / SAMPLE_TILE_X;
	int sampleTileHeight = (sampleArea.bottom - sampleArea.top) / SAMPLE_TILE_Y;
	// 디버깅 메시지
	TCHAR szDebug[128];
	wsprintf(szDebug,TEXT("Sample area: (%d,%d) - (%d,%d), Tile size: %dx%d\n"),
			 sampleArea.left,sampleArea.top,sampleArea.right,sampleArea.bottom,
			 sampleTileWidth,sampleTileHeight);
	OutputDebugString(szDebug);

	// 타일 렌더링
	for(int y = 0; y < SAMPLE_TILE_Y; y++)
	{
		for(int x = 0; x < SAMPLE_TILE_X; x++)
		{
			// 타일 중앙 좌표 계산
			int posX = sampleArea.left + x * sampleTileWidth + (sampleTileWidth / 2);
			int posY = sampleArea.top + y * sampleTileHeight + (sampleTileHeight / 2);

			// 타일 렌더링
			sampleTileImage->FrameRender(hdc,posX,posY,x,y,false,true);

			// 선택된 타일 강조 표시
			if(x == selectedTile.x && y == selectedTile.y)
			{
				// 더 눈에 띄는 선택 표시
				HPEN selectPen = CreatePen(PS_SOLID,3,RGB(255,0,0));
				HBRUSH selectBrush = CreateSolidBrush(RGB(255,230,230)); // 연한 빨강 배경

				HPEN oldPen = (HPEN)SelectObject(hdc,selectPen);
				HBRUSH oldBrush = (HBRUSH)SelectObject(hdc,selectBrush);

				Rectangle(hdc,
						 posX - (sampleTileWidth / 2) - 2,
						 posY - (sampleTileHeight / 2) - 2,
						 posX + (sampleTileWidth / 2) + 2,
						 posY + (sampleTileHeight / 2) + 2);

				SelectObject(hdc,oldPen);
				SelectObject(hdc,oldBrush);
				DeleteObject(selectPen);
				DeleteObject(selectBrush);
			}

			// 타일 이미지 렌더링
			sampleTileImage->FrameRender(hdc,posX,posY,x,y,false,true);
		}
	}
}

void MapEditorRender::RenderSprites(HDC hdc,const vector<Sprite>& sprites,RECT mapArea)
{
	int visibleWidth = VISIBLE_MAP_WIDTH;
	int visibleHeight = VISIBLE_MAP_HEIGHT;

	int tileWidth = (mapArea.right - mapArea.left) / visibleWidth;
	int tileHeight = (mapArea.bottom - mapArea.top) / visibleHeight;

	for(const auto& sprite : sprites)
	{
		int tileX = static_cast<int>(sprite.pos.x - 0.5f);
		int tileY = static_cast<int>(sprite.pos.y - 0.5f);

		if(tileX >= 0 && tileX < visibleWidth && tileY >= 0 && tileY < visibleHeight)
		{
			int screenX = mapArea.left + tileX * tileWidth + (tileWidth / 2);
			int screenY = mapArea.top + tileY * tileHeight + (tileHeight / 2);

			COLORREF color = (sprite.type == SpriteType::KEY) ? RGB(0,0,255) :
				(sprite.type == SpriteType::MONSTER) ? RGB(255,0,0) :
				RGB(128,128,128);

			HBRUSH spriteBrush = CreateSolidBrush(color);
			HBRUSH oldBrush = (HBRUSH)SelectObject(hdc,spriteBrush);

			Ellipse(hdc,
				   screenX - (tileWidth / 3),
				   screenY - (tileHeight / 3),
				   screenX + (tileWidth / 3),
				   screenY + (tileHeight / 3));

			SelectObject(hdc,oldBrush);
			DeleteObject(spriteBrush);
		}
	}

	// 스프라이트 카운트 정보
	int itemCount = 0,monsterCount = 0;
	for(const auto& sprite : sprites)
	{
		if(sprite.type == SpriteType::KEY) itemCount++;
		else if(sprite.type == SpriteType::MONSTER) monsterCount++;
	}

	TCHAR szCount[128];
	wsprintf(szCount,TEXT("Items: %d, Monsters: %d"),itemCount,monsterCount);
	TextOut(hdc,mapArea.left,mapArea.bottom + 10,szCount,lstrlen(szCount));
}

void MapEditorRender::RenderObstacles(HDC hdc,const vector<Obstacle>& obstacles,RECT mapArea)
{
	int visibleWidth = VISIBLE_MAP_WIDTH;
	int visibleHeight = VISIBLE_MAP_HEIGHT;

	int tileWidth = (mapArea.right - mapArea.left) / visibleWidth;
	int tileHeight = (mapArea.bottom - mapArea.top) / visibleHeight;

	for(const auto& obstacle : obstacles)
	{
		int tileX = obstacle.pos.x;
		int tileY = obstacle.pos.y;

		if(tileX >= 0 && tileX < visibleWidth && tileY >= 0 && tileY < visibleHeight)
		{
			int screenX = mapArea.left + tileX * tileWidth + (tileWidth / 2);
			int screenY = mapArea.top + tileY * tileHeight + (tileHeight / 2);

			COLORREF color = RGB(255,128,0);  // 주황색으로 장애물 표시

			HBRUSH obstacleBrush = CreateSolidBrush(color);
			HPEN obstaclePen = CreatePen(PS_SOLID,2,RGB(200,0,0));

			HBRUSH oldBrush = (HBRUSH)SelectObject(hdc,obstacleBrush);
			HPEN oldPen = (HPEN)SelectObject(hdc,obstaclePen);

			// 방향 표시
			POINT arrowPoints[3];
			int arrowSize = tileWidth / 3;

			switch(obstacle.dir)
			{
			case Direction::NORTH:
			arrowPoints[0] = {screenX,screenY - arrowSize};
			arrowPoints[1] = {screenX - arrowSize/2,screenY + arrowSize/2};
			arrowPoints[2] = {screenX + arrowSize/2,screenY + arrowSize/2};
			break;
			case Direction::SOUTH:
			arrowPoints[0] = {screenX,screenY + arrowSize};
			arrowPoints[1] = {screenX - arrowSize/2,screenY - arrowSize/2};
			arrowPoints[2] = {screenX + arrowSize/2,screenY - arrowSize/2};
			break;
			case Direction::WEST:
			arrowPoints[0] = {screenX - arrowSize,screenY};
			arrowPoints[1] = {screenX + arrowSize/2,screenY - arrowSize/2};
			arrowPoints[2] = {screenX + arrowSize/2,screenY + arrowSize/2};
			break;
			case Direction::EAST:
			arrowPoints[0] = {screenX + arrowSize,screenY};
			arrowPoints[1] = {screenX - arrowSize/2,screenY - arrowSize/2};
			arrowPoints[2] = {screenX - arrowSize/2,screenY + arrowSize/2};
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
										   int mapWidth,RECT mapArea)
{
	int visibleWidth = VISIBLE_MAP_WIDTH;
	int visibleHeight = VISIBLE_MAP_HEIGHT;

	int tileWidth = (mapArea.right - mapArea.left) / visibleWidth;
	int tileHeight = (mapArea.bottom - mapArea.top) / visibleHeight;

	// 시작 위치 계산
	int startX = static_cast<int>(startPos.x - 0.5f);
	int startY = static_cast<int>(startPos.y - 0.5f);

	if(startX >= 0 && startX < visibleWidth && startY >= 0 && startY < visibleHeight)
	{
		int screenX = mapArea.left + startX * tileWidth + (tileWidth / 2);
		int screenY = mapArea.top + startY * tileHeight + (tileHeight / 2);

		HBRUSH greenBrush = CreateSolidBrush(RGB(0,255,0)); // 밝은 녹색
		HPEN greenPen = CreatePen(PS_SOLID,3,RGB(0,160,0));

		HBRUSH oldBrush = (HBRUSH)SelectObject(hdc,greenBrush);
		HPEN oldPen = (HPEN)SelectObject(hdc,greenPen);

		// 원 그리기
		Ellipse(hdc,
			   screenX - tileWidth/3,
			   screenY - tileHeight/3,
			   screenX + tileWidth/3,
			   screenY + tileHeight/3);

		// "S" 표시 추가
		SetTextColor(hdc,RGB(0,0,0));
		SetBkMode(hdc,TRANSPARENT);
		HFONT font = CreateFont(tileHeight/3,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,
							   DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
							   DEFAULT_QUALITY,DEFAULT_PITCH | FF_DONTCARE,TEXT("Arial"));
		HFONT oldFont = (HFONT)SelectObject(hdc,font);

		RECT textRect = {
			screenX - tileWidth/4,
			screenY - tileHeight/4,
			screenX + tileWidth/4,
			screenY + tileHeight/4
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

void MapEditorRender::RenderModeInfo(HDC hdc,EditorMode mode)
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

	SelectObject(hdc,oldFont);
	DeleteObject(hFont);
	SetTextColor(hdc,RGB(0,0,0)); // 기본 색상으로 복원
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

	TextOut(hdc,20,WINSIZE_Y - 70,TEXT("Left Click: Place / Right Click: Delete"),
			lstrlen(TEXT("Left Click: Place / Right Click: Delete")));
	TextOut(hdc,20,WINSIZE_Y - 50,TEXT("ESC: Return to Game, Arrow Keys: Change Obstacle Direction"),
			lstrlen(TEXT("ESC: Return to Game, Arrow Keys: Change Obstacle Direction")));
	TextOut(hdc,20,WINSIZE_Y - 30,TEXT("1-5: Change Mode, S: Save, L: Load, C: Clear"),
			lstrlen(TEXT("1-5: Change Mode, S: Save, L: Load, C: Clear")));

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
