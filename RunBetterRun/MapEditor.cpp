#include "MapEditor.h"
#include "TextureManager.h"
#include "MapManager.h"
#include "DataManager.h"
#include "KeyManager.h"
#include "Image.h"
#include "SceneManager.h"

MapEditor::MapEditor():
	currentMode(EditMode::TILE),
	selectedTileType(RoomType::FLOOR),
	selectedObstacleDir(Direction::EAST),
	selectedTile({0,0}),
	mapWidth(VISIBLE_MAP_WIDTH),
	mapHeight(VISIBLE_MAP_HEIGHT),
	zoomLevel(1.0f),
	viewportOffset({0.0f,0.0f}),
	isDragging(false),
	mouseInMapArea(false),
	mouseInSampleArea(false),
	sampleTileImage(nullptr)
{}

MapEditor::~MapEditor()
{
	Release();
}

HRESULT MapEditor::Init()
{
	tiles.resize(mapWidth * mapHeight);

	// 타일 초기화
	for(int y = 0; y < mapHeight; y++) {
		for(int x = 0; x < mapWidth; x++) {
			int index = y * mapWidth + x;

			if(x == 0 || y == 0 || x == mapWidth - 1 || y == mapHeight - 1) {
				tiles[index].roomType = RoomType::WALL;
				tiles[index].tilePos = 4; 
			} else {
				tiles[index].roomType = RoomType::FLOOR;
				tiles[index].tilePos = 10;
			}
		}
	}

	// 시작 위치 초기화
	startPosition = {mapWidth / 2.0f,mapHeight / 2.0f};
	int startIndex = (int)startPosition.y * mapWidth + (int)startPosition.x;
	if(startIndex < tiles.size()) {
		tiles[startIndex].roomType = RoomType::START;
	}

	// 샘플 타일 이미지 로드
	sampleTileImage = ImageManager::GetInstance()->AddImage(
		"EditorSampleTile",L"Image/tiles32x32.bmp",
		SAMPLE_TILE_X * TILE_SIZE,SAMPLE_TILE_Y * TILE_SIZE,
		SAMPLE_TILE_X,SAMPLE_TILE_Y,
		true,RGB(255,0,255));

	if(!sampleTileImage) {
		return E_FAIL;
	}

	// 정보창
	int infoHeight = 80;
	int uiPadding = 20;
	int rightPanelWidth = 300;

	// 샘플 타일 영역 
	sampleArea = {
		TILEMAPTOOL_X - rightPanelWidth - uiPadding,
		infoHeight + uiPadding * 2, 
		TILEMAPTOOL_X - rightPanelWidth - uiPadding + SAMPLE_TILE_X * TILE_SIZE,
		infoHeight + uiPadding * 2 + SAMPLE_TILE_Y * TILE_SIZE
	};

	// 맵 편집 영역 
	int mapAreaWidth = sampleArea.left - (uiPadding * 2);
	int mapAreaHeight = TILEMAPTOOL_Y - (infoHeight + uiPadding * 4);

	mapArea = {
		uiPadding,
		infoHeight + uiPadding, 
		uiPadding + mapAreaWidth,
		infoHeight + uiPadding + mapAreaHeight
	};

	while(ShowCursor(TRUE) < 0);

	return S_OK;
}

void MapEditor::Release()
{
	tiles.clear();
	editorSprites.clear();
	editorObstacles.clear();
	DataManager::GetInstance()->ClearAllData();
}

void MapEditor::Update()
{
	// 마우스 위치 업데이트
	POINT cursorPos;
	GetCursorPos(&cursorPos);
	ScreenToClient(g_hWnd,&cursorPos);
	mousePos = cursorPos;

	// 마우스 위치 확인
	mouseInMapArea = PtInRect(&mapArea,mousePos);
	mouseInSampleArea = PtInRect(&sampleArea,mousePos);

	if(KeyManager::GetInstance()->IsOnceKeyDown(VK_ESCAPE))
	{
		SceneManager::GetInstance()->ChangeScene("MainGameScene");
		return;

	}

	HandleInput();
}

void MapEditor::Render(HDC hdc)
{
	// 배경 채우기
	PatBlt(hdc,0,0,TILEMAPTOOL_X,TILEMAPTOOL_Y,WHITENESS);

	// 맵 영역 테두리 그리기
	HPEN mapAreaPen = CreatePen(PS_SOLID,2,RGB(100,100,100));
	HPEN oldPen = (HPEN)SelectObject(hdc,mapAreaPen);
	Rectangle(hdc,mapArea.left-2,mapArea.top-2,mapArea.right+2,mapArea.bottom+2);
	SelectObject(hdc,oldPen);
	DeleteObject(mapAreaPen);

	// 맵 타일 렌더링
	RenderMapTiles(hdc);

	// 스프라이트 렌더링
	RenderSprites(hdc);

	// 장애물 렌더링
	RenderObstacles(hdc);

	// 샘플 타일 렌더링
	RenderSampleTiles(hdc);

	// UI 정보 렌더링
	RenderUI(hdc);
}

void MapEditor::RenderMapTiles(HDC hdc)
{
    if (!sampleTileImage) return;
    
    // 타일 크기 계산 (확대/축소 적용)
    int tileWidth = (mapArea.right - mapArea.left) / (mapWidth / zoomLevel);
    int tileHeight = (mapArea.bottom - mapArea.top) / (mapHeight / zoomLevel);
    int tileSize = min(tileWidth, tileHeight);
    
    // 화면에 표시될 타일 범위
    int startX = (int)viewportOffset.x;
    int startY = (int)viewportOffset.y;
    int endX = min(mapWidth, (int)(viewportOffset.x + mapWidth / zoomLevel) + 1);
    int endY = min(mapHeight, (int)(viewportOffset.y + mapHeight / zoomLevel) + 1);
    
    // 타일 렌더링
    for (int y = startY; y < endY; y++) {
        for (int x = startX; x < endX; x++) {
            // 스크린 좌표 계산
            POINT screenPos = TileToScreen({x, y});
            
            // 타일 정보 가져오기
            int index = y * mapWidth + x;
            if (index >= tiles.size()) continue;
            
            int tilePos = tiles[index].tilePos;
            int frameX = tilePos % SAMPLE_TILE_X;
            int frameY = tilePos / SAMPLE_TILE_X;
            
            // 타일 렌더링
            sampleTileImage->FrameRender(
                hdc,
                screenPos.x + tileSize / 2,
                screenPos.y + tileSize / 2,
                frameX, frameY,
                false, true
            );
            
            // 시작 위치 표시
            if (tiles[index].roomType == RoomType::START) {
                HBRUSH startBrush = CreateSolidBrush(RGB(0, 200, 0));
                HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, startBrush);
                Ellipse(
                    hdc,
                    screenPos.x + tileSize / 4,
                    screenPos.y + tileSize / 4,
                    screenPos.x + 3 * tileSize / 4,
                    screenPos.y + 3 * tileSize / 4
                );
                SelectObject(hdc, oldBrush);
                DeleteObject(startBrush);
            }
            
            // 마우스 위치의 타일 표시 - 현재 모드에 맞는 색상 사용
            if (mouseInMapArea) {
                POINT tilePos = ScreenToTile(mousePos);
                if (tilePos.x == x && tilePos.y == y) {
                    // 모드별 색상 설정
                    COLORREF highlightColor;
                    switch (currentMode) {
                    case EditMode::TILE:
                        highlightColor = RGB(255, 255, 0); // 노란색
                        break;
                    case EditMode::START:
                        highlightColor = RGB(0, 255, 0);   // 녹색
                        break;
                    case EditMode::OBSTACLE:
                        highlightColor = RGB(255, 128, 0); // 주황색
                        break;
                    case EditMode::MONSTER:
                        highlightColor = RGB(255, 0, 0);   // 빨간색
                        break;
                    case EditMode::ITEM:
                        highlightColor = RGB(0, 0, 255);   // 파란색
                        break;
                    default:
                        highlightColor = RGB(255, 0, 0);   // 기본값
                    }
                    
                    HPEN highlightPen = CreatePen(PS_SOLID, 2, highlightColor);
                    HPEN oldPen = (HPEN)SelectObject(hdc, highlightPen);
                    SelectObject(hdc, GetStockObject(NULL_BRUSH));
                    
                    Rectangle(
                        hdc,
                        screenPos.x,
                        screenPos.y,
                        screenPos.x + tileSize,
                        screenPos.y + tileSize
                    );
                    
                    SelectObject(hdc, oldPen);
                    DeleteObject(highlightPen);
                }
            }
        }
    }
}

void MapEditor::RenderSampleTiles(HDC hdc)
{
	if(!sampleTileImage) return;

	// 샘플 영역 배경 및 테두리
	HBRUSH sampleBgBrush = CreateSolidBrush(RGB(240,240,240));
	HBRUSH oldBrush = (HBRUSH)SelectObject(hdc,sampleBgBrush);
	HPEN samplePen = CreatePen(PS_SOLID,2,RGB(150,50,50));
	HPEN oldPen = (HPEN)SelectObject(hdc,samplePen);

	// 샘플 영역 배경
	Rectangle(hdc,sampleArea.left-5,sampleArea.top-25,sampleArea.right+5,sampleArea.bottom+5);

	// 샘플 영역 제목
	SetBkMode(hdc,TRANSPARENT);
	SetTextColor(hdc,RGB(0,0,0));
	HFONT titleFont = CreateFont(18,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,
							 DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
							 DEFAULT_QUALITY,DEFAULT_PITCH | FF_DONTCARE,TEXT("Arial"));
	HFONT oldFont = (HFONT)SelectObject(hdc,titleFont);

	TextOut(hdc,sampleArea.left,sampleArea.top-20,L"Sample Tiles",12);

	SelectObject(hdc,oldFont);
	DeleteObject(titleFont);

	// 샘플 타일 그리기
	for(int y = 0; y < SAMPLE_TILE_Y; y++) {
		for(int x = 0; x < SAMPLE_TILE_X; x++) {
			sampleTileImage->FrameRender(
				hdc,
				sampleArea.left + x * TILE_SIZE + TILE_SIZE/2,
				sampleArea.top + y * TILE_SIZE + TILE_SIZE/2,
				x,y,false,true
			);

			// 선택된 샘플 타일 표시
			if(x == selectedTile.x && y == selectedTile.y) {
				HPEN selectionPen = CreatePen(PS_SOLID,3,RGB(255,50,50));
				HPEN oldSelPen = (HPEN)SelectObject(hdc,selectionPen);
				SelectObject(hdc,GetStockObject(NULL_BRUSH));

				Rectangle(hdc,
					sampleArea.left + x * TILE_SIZE,
					sampleArea.top + y * TILE_SIZE,
					sampleArea.left + (x+1) * TILE_SIZE,
					sampleArea.top + (y+1) * TILE_SIZE
				);

				SelectObject(hdc,oldSelPen);
				DeleteObject(selectionPen);
			}
		}
	}

	SelectObject(hdc,oldPen);
	DeleteObject(samplePen);
	SelectObject(hdc,oldBrush);
	DeleteObject(sampleBgBrush);
}

void MapEditor::RenderSprites(HDC hdc)
{
	// 타일 크기 계산 (확대/축소 적용)
	int tileWidth = (mapArea.right - mapArea.left) / (mapWidth / zoomLevel);
	int tileHeight = (mapArea.bottom - mapArea.top) / (mapHeight / zoomLevel);
	int tileSize = min(tileWidth,tileHeight);

	for(const auto& sprite : editorSprites) {
		// 스프라이트 위치 (실제 좌표)
		float spriteX = sprite.pos.x;
		float spriteY = sprite.pos.y;

		// 화면 영역 내에 있는지 확인
		if(spriteX < viewportOffset.x || spriteX >= viewportOffset.x + mapWidth / zoomLevel ||
			spriteY < viewportOffset.y || spriteY >= viewportOffset.y + mapHeight / zoomLevel)
			continue;

		// 스크린 좌표 계산 (타일 위치가 아닌 실제 위치 기준)
		int screenX = mapArea.left + (int)((spriteX - viewportOffset.x) * tileSize);
		int screenY = mapArea.top + (int)((spriteY - viewportOffset.y) * tileSize);

		// 스프라이트 종류에 따라 다른 색상 + 현재 모드와 일치하면 더 밝게
		COLORREF color;
		if(sprite.type == SpriteType::KEY) {
			// 아이템
			color = (currentMode == EditMode::ITEM) ?
				RGB(100,100,255) : RGB(0,0,200);  // 파란색, 활성화시 더 밝게
		} else {
			// 몬스터
			color = (currentMode == EditMode::MONSTER) ?
				RGB(255,100,100) : RGB(200,0,0);  // 빨간색, 활성화시 더 밝게
		}

		// 스프라이트 렌더링 (크기는 타일의 1/4)
		int spriteRadius = tileSize / 6;  // 작은 크기로 변경
		HBRUSH spriteBrush = CreateSolidBrush(color);
		HBRUSH oldBrush = (HBRUSH)SelectObject(hdc,spriteBrush);

		Ellipse(hdc,
			screenX - spriteRadius,
			screenY - spriteRadius,
			screenX + spriteRadius,
			screenY + spriteRadius
		);

		SelectObject(hdc,oldBrush);
		DeleteObject(spriteBrush);
	}
}
void MapEditor::RenderObstacles(HDC hdc)
{
	// 타일 크기 계산 (확대/축소 적용)
	int tileWidth = (mapArea.right - mapArea.left) / (mapWidth / zoomLevel);
	int tileHeight = (mapArea.bottom - mapArea.top) / (mapHeight / zoomLevel);
	int tileSize = min(tileWidth,tileHeight);

	for(const auto& obstacle : editorObstacles) {
		// 장애물 위치
		int x = obstacle.pos.x;
		int y = obstacle.pos.y;

		// 화면 영역 내에 있는지 확인
		if(x < viewportOffset.x || x >= viewportOffset.x + mapWidth / zoomLevel ||
			y < viewportOffset.y || y >= viewportOffset.y + mapHeight / zoomLevel)
			continue;

		// 스크린 좌표 계산
		POINT screenPos = TileToScreen({x,y});

		// 장애물 렌더링
		HBRUSH obstacleBrush = CreateSolidBrush(RGB(255,128,0));
		HPEN obstaclePen = CreatePen(PS_SOLID,2,RGB(200,0,0));

		HBRUSH oldBrush = (HBRUSH)SelectObject(hdc,obstacleBrush);
		HPEN oldPen = (HPEN)SelectObject(hdc,obstaclePen);

		// 방향 표시 화살표
		POINT arrowPoints[3];

		switch(obstacle.dir) {
		case Direction::NORTH:
		arrowPoints[0] = {screenPos.x + tileSize/2,screenPos.y + tileSize/4};
		arrowPoints[1] = {screenPos.x + tileSize/4,screenPos.y + tileSize*3/4};
		arrowPoints[2] = {screenPos.x + tileSize*3/4,screenPos.y + tileSize*3/4};
		break;
		case Direction::SOUTH:
		arrowPoints[0] = {screenPos.x + tileSize/2,screenPos.y + tileSize*3/4};
		arrowPoints[1] = {screenPos.x + tileSize/4,screenPos.y + tileSize/4};
		arrowPoints[2] = {screenPos.x + tileSize*3/4,screenPos.y + tileSize/4};
		break;
		case Direction::WEST:
		arrowPoints[0] = {screenPos.x + tileSize/4,screenPos.y + tileSize/2};
		arrowPoints[1] = {screenPos.x + tileSize*3/4,screenPos.y + tileSize/4};
		arrowPoints[2] = {screenPos.x + tileSize*3/4,screenPos.y + tileSize*3/4};
		break;
		case Direction::EAST:
		arrowPoints[0] = {screenPos.x + tileSize*3/4,screenPos.y + tileSize/2};
		arrowPoints[1] = {screenPos.x + tileSize/4,screenPos.y + tileSize/4};
		arrowPoints[2] = {screenPos.x + tileSize/4,screenPos.y + tileSize*3/4};
		break;
		}

		Polygon(hdc,arrowPoints,3);

		SelectObject(hdc,oldPen);
		SelectObject(hdc,oldBrush);
		DeleteObject(obstaclePen);
		DeleteObject(obstacleBrush);
	}
}

void MapEditor::RenderUI(HDC hdc)
{
	// 정보 패널 배경 (상단에 배치)
	HBRUSH infoBgBrush = CreateSolidBrush(RGB(40,40,40));
	RECT infoRect = {0,0,TILEMAPTOOL_X,80}; // 상단에 위치
	FillRect(hdc,&infoRect,infoBgBrush);
	DeleteObject(infoBgBrush);

	// 현재 모드 표시 - 모드별로 다른 색상 적용
	SetBkMode(hdc,TRANSPARENT);

	HFONT infoFont = CreateFont(16,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,
						  DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
						  DEFAULT_QUALITY,DEFAULT_PITCH | FF_DONTCARE,TEXT("Arial"));
	HFONT oldFont = (HFONT)SelectObject(hdc,infoFont);

	// 모드별로 다른 색상 적용
	COLORREF modeColor;
	TCHAR modeText[50];

	switch(currentMode) {
	case EditMode::TILE:
	wcscpy_s(modeText,L"MODE: TILE");
	modeColor = RGB(255,255,0); // 노란색
	break;
	case EditMode::START:
	wcscpy_s(modeText,L"MODE: START POSITION");
	modeColor = RGB(0,255,0);   // 녹색
	break;
	case EditMode::OBSTACLE:
	wcscpy_s(modeText,L"MODE: OBSTACLE");
	modeColor = RGB(255,128,0); // 주황색
	break;
	case EditMode::MONSTER:
	wcscpy_s(modeText,L"MODE: MONSTER");
	modeColor = RGB(255,0,0);   // 빨간색
	break;
	case EditMode::ITEM:
	wcscpy_s(modeText,L"MODE: ITEM");
	modeColor = RGB(0,0,255);   // 파란색
	break;
	}

	
	SetTextColor(hdc,modeColor);
	TextOut(hdc,20,15,modeText,wcslen(modeText));

	HBRUSH modeBrush = CreateSolidBrush(modeColor);
	HBRUSH oldBrush = (HBRUSH)SelectObject(hdc,modeBrush);
	Rectangle(hdc,170,15,190,35);
	SelectObject(hdc,oldBrush);
	DeleteObject(modeBrush);

	SetTextColor(hdc,RGB(255,255,255));
	TCHAR tileTypeText[50];
	switch(selectedTileType) {
	case RoomType::FLOOR: wcscpy_s(tileTypeText,L"TILE TYPE: FLOOR"); break;
	case RoomType::WALL: wcscpy_s(tileTypeText,L"TILE TYPE: WALL"); break;
	default: wcscpy_s(tileTypeText,L"TILE TYPE: OTHER"); break;
	}
	TextOut(hdc,200,15,tileTypeText,wcslen(tileTypeText));

	if(currentMode == EditMode::OBSTACLE) {
		TCHAR dirText[50];
		switch(selectedObstacleDir) {
		case Direction::NORTH: wcscpy_s(dirText,L"DIRECTION: NORTH"); break;
		case Direction::SOUTH: wcscpy_s(dirText,L"DIRECTION: SOUTH"); break;
		case Direction::EAST: wcscpy_s(dirText,L"DIRECTION: EAST"); break;
		case Direction::WEST: wcscpy_s(dirText,L"DIRECTION: WEST"); break;
		}
		TextOut(hdc,400,15,dirText,wcslen(dirText));
	}

	// 현재 좌표
	TCHAR posText[50];
	if(mouseInMapArea) {
		POINT tilePos = ScreenToTile(mousePos);
		swprintf_s(posText,L"X: %d, Y: %d",tilePos.x,tilePos.y);
		TextOut(hdc,600,15,posText,wcslen(posText));
	}

	TCHAR zoomText[50];
	swprintf_s(zoomText,L"ZOOM: %.1f%%",zoomLevel * 100.0f);
	TextOut(hdc,800,15,zoomText,wcslen(zoomText));

	TextOut(hdc,20,45,L"1-5: Change Mode  F: Floor  W: Wall  Arrow Keys: Direction",62);
	TextOut(hdc,600,45,L"S: Save  L: Load  C: Clear  +/-: Zoom",40);

	SelectObject(hdc,oldFont);
	DeleteObject(infoFont);
}

void MapEditor::HandleInput()
{
	KeyManager* km = KeyManager::GetInstance();

	// 에디터 모드 변경
	if(km->IsOnceKeyDown('1')) ChangeEditMode(EditMode::TILE);
	else if(km->IsOnceKeyDown('2')) ChangeEditMode(EditMode::START);
	else if(km->IsOnceKeyDown('3')) ChangeEditMode(EditMode::ITEM);
	else if(km->IsOnceKeyDown('4')) ChangeEditMode(EditMode::MONSTER);
	else if(km->IsOnceKeyDown('5')) ChangeEditMode(EditMode::OBSTACLE);

	// 타일 타입 설정
	if(km->IsOnceKeyDown('F')) selectedTileType = RoomType::FLOOR;
	else if(km->IsOnceKeyDown('W')) selectedTileType = RoomType::WALL;

	// 맵 저장/로드/초기화
	if(km->IsOnceKeyDown('S')) SaveMap(L"Map/EditorMap.dat");
	else if(km->IsOnceKeyDown('L')) LoadMap(L"Map/EditorMap.dat");
	else if(km->IsOnceKeyDown('C')) ClearMap();

	// 확대/축소
	if(km->IsOnceKeyDown(VK_OEM_PLUS)) Zoom(0.1f);
	else if(km->IsOnceKeyDown(VK_OEM_MINUS)) Zoom(-0.1f);

	// 장애물 방향 설정
	if(currentMode == EditMode::OBSTACLE) {
		if(km->IsOnceKeyDown(VK_UP)) selectedObstacleDir = Direction::NORTH;
		else if(km->IsOnceKeyDown(VK_DOWN)) selectedObstacleDir = Direction::SOUTH;
		else if(km->IsOnceKeyDown(VK_LEFT)) selectedObstacleDir = Direction::WEST;
		else if(km->IsOnceKeyDown(VK_RIGHT)) selectedObstacleDir = Direction::EAST;
	}

	// 타일/오브젝트 배치 및 삭제
	if(mouseInSampleArea && km->IsOnceKeyDown(VK_LBUTTON)) {
		// 샘플 타일 선택
		int relX = mousePos.x - sampleArea.left;
		int relY = mousePos.y - sampleArea.top;

		// 샘플 타일 범위 체크
		if(relX >= 0 && relY >= 0) {
			selectedTile.x = min(relX / TILE_SIZE,SAMPLE_TILE_X - 1);
			selectedTile.y = min(relY / TILE_SIZE,SAMPLE_TILE_Y - 1);
		}
	} else if(mouseInMapArea) {
		// 맵 편집
		POINT tilePos = ScreenToTile(mousePos);

		// 유효한 타일 위치인지 확인
		if(tilePos.x >= 0 && tilePos.y >= 0) {
			// 타일의 중앙 화면 좌표 계산
			POINT screenPos = TileToScreen(tilePos);

			// 맵 위치가 유효한 경우만 처리
			if(screenPos.x >= 0 && screenPos.y >= 0) {
				int tileWidth = (mapArea.right - mapArea.left) / (mapWidth / zoomLevel);
				int tileHeight = (mapArea.bottom - mapArea.top) / (mapHeight / zoomLevel);
				int tileSize = min(tileWidth,tileHeight);

				// 타일 영역 계산
				RECT tileRect = {
					screenPos.x,
					screenPos.y,
					screenPos.x + tileSize,
					screenPos.y + tileSize
				};

				// 마우스가 타일 위에 있는지 확인 (추가 정확도 체크)
				if(PtInRect(&tileRect,mousePos))
				{
					// 왼쪽 버튼 - 오브젝트 배치
					if(km->IsStayKeyDown(VK_LBUTTON))
					{
						switch(currentMode) {
						case EditMode::TILE: PlaceTile(tilePos.x,tilePos.y); break;
						case EditMode::START: PlaceStart(tilePos.x,tilePos.y); break;
						case EditMode::OBSTACLE: PlaceObstacle(tilePos.x,tilePos.y); break;
						case EditMode::MONSTER: PlaceMonster(tilePos.x,tilePos.y); break;
						case EditMode::ITEM: PlaceItem(tilePos.x,tilePos.y); break;
						}
					}
					// 오른쪽 버튼 - 오브젝트 삭제
					else if(km->IsStayKeyDown(VK_RBUTTON)) {
						RemoveObject(tilePos.x,tilePos.y);
					}
				}
			}

			// 중간 버튼 - 맵 스크롤
			if(km->IsStayKeyDown(VK_MBUTTON)) {
				if(!isDragging) {
					isDragging = true;
					lastMousePos = mousePos;
				} else {
					int deltaX = mousePos.x - lastMousePos.x;
					int deltaY = mousePos.y - lastMousePos.y;

					// 0으로 나누기 예방
					if(deltaX != 0 || deltaY != 0) {
						Scroll(-deltaX / 20.0f,-deltaY / 20.0f);
						lastMousePos = mousePos;
					}
				}
			} else {
				isDragging = false;
			}
		}
	}
}

void MapEditor::PlaceTile(int x,int y)
{
	if(x < 0 || x >= mapWidth || y < 0 || y >= mapHeight)
		return;

	int tileIndex = selectedTile.y * SAMPLE_TILE_X + selectedTile.x;
	int index = y * mapWidth + x;

	// 배열 범위 검사
	if(index >= 0 && index < tiles.size()) {
		tiles[index].tilePos = tileIndex;
		tiles[index].roomType = selectedTileType;
	}
}

void MapEditor::PlaceStart(int x,int y)
{
	if(x < 0 || x >= mapWidth || y < 0 || y >= mapHeight)
		return;

	for(size_t i = 0; i < tiles.size(); i++) {
		if(tiles[i].roomType == RoomType::START) {
			tiles[i].roomType = RoomType::FLOOR;
			tiles[i].tilePos = 10; // 기본 바닥 타일
		}
	}

	// 새 시작 위치 설정
	int index = y * mapWidth + x;
	if(index >= 0 && index < tiles.size()) {
		tiles[index].roomType = RoomType::START;
		startPosition = {x + 0.5f,y + 0.5f};
	}
}

void MapEditor::PlaceObstacle(int x,int y)
{
	if(x < 0 || x >= mapWidth || y < 0 || y >= mapHeight)
		return;

	for(size_t i = 0; i < editorObstacles.size(); i++) {
		if(editorObstacles[i].pos.x == x && editorObstacles[i].pos.y == y) {
			return; 
		}
	}

	Texture* obstacleTexture = TextureManager::GetInstance()->GetTexture(TEXT("Image/jewel.bmp"));
	if(obstacleTexture) {
		Obstacle newObstacle;
		newObstacle.pos = {x,y};
		newObstacle.texture = obstacleTexture;
		newObstacle.dir = selectedObstacleDir;
		newObstacle.block = TRUE;

		editorObstacles.push_back(newObstacle);
	}
}

void MapEditor::PlaceMonster(int x,int y)
{
	// 맵 범위 검사
	if(x < 0 || x >= mapWidth || y < 0 || y >= mapHeight)
		return;

	// 마우스의 정확한 위치를 사용하여 타일 내에서의 상대적 위치 계산
	POINT tileScreenPos = TileToScreen({x,y});
	if(tileScreenPos.x < 0 || tileScreenPos.y < 0) return; // 예외 처리

	int tileWidth = (mapArea.right - mapArea.left) / (mapWidth / zoomLevel);
	int tileHeight = (mapArea.bottom - mapArea.top) / (mapHeight / zoomLevel);
	int tileSize = min(tileWidth,tileHeight);

	// 타일 내에서의 상대 위치 (0.0 ~ 1.0)
	float relativeX = (mousePos.x - tileScreenPos.x) / (float)tileSize;
	float relativeY = (mousePos.y - tileScreenPos.y) / (float)tileSize;

	// 범위 제한 (0.0 ~ 1.0)
	relativeX = max(0.0f,min(1.0f,relativeX));
	relativeY = max(0.0f,min(1.0f,relativeY));

	// 스프라이트의 최종 위치 계산 (타일 좌표 + 타일 내 상대 위치)
	FPOINT spritePos = {
		x + relativeX,
		y + relativeY
	};

	// 근처에 다른 스프라이트가 있는지 확인 (일정 거리 내에 있으면 배치 불가)
	const float MIN_DISTANCE = 0.2f; // 최소 거리 (타일 크기의 20%)

	for(const auto& sprite : editorSprites) {
		float dx = sprite.pos.x - spritePos.x;
		float dy = sprite.pos.y - spritePos.y;
		float distance = sqrt(dx*dx + dy*dy);

		if(distance < MIN_DISTANCE) {
			return; // 너무 가까운 위치에 이미 스프라이트가 있음
		}
	}

	Texture* monsterTexture = TextureManager::GetInstance()->GetTexture(TEXT("Image/boss.bmp"));
	if(!monsterTexture) {
		MessageBox(g_hWnd,TEXT("Monster texture not found!"),TEXT("Error"),MB_OK);
		return;
	}

	Sprite newSprite;
	newSprite.pos = spritePos;
	newSprite.type = SpriteType::MONSTER;
	newSprite.texture = monsterTexture;
	newSprite.distance = 0.0f;
	newSprite.aniInfo = {0.1f,0.1f,{423,437},{1,1},{0,0}};

	editorSprites.push_back(newSprite);
}

void MapEditor::PlaceItem(int x,int y)
{
	// 맵 범위 검사
	if(x < 0 || x >= mapWidth || y < 0 || y >= mapHeight)
		return;

	// 마우스의 정확한 위치를 사용하여 타일 내에서의 상대적 위치 계산
	POINT tileScreenPos = TileToScreen({x,y});
	if(tileScreenPos.x < 0 || tileScreenPos.y < 0) return; // 예외 처리

	int tileWidth = (mapArea.right - mapArea.left) / (mapWidth / zoomLevel);
	int tileHeight = (mapArea.bottom - mapArea.top) / (mapHeight / zoomLevel);
	int tileSize = min(tileWidth,tileHeight);

	// 타일 내에서의 상대 위치 (0.0 ~ 1.0)
	float relativeX = (mousePos.x - tileScreenPos.x) / (float)tileSize;
	float relativeY = (mousePos.y - tileScreenPos.y) / (float)tileSize;

	// 범위 제한 (0.0 ~ 1.0)
	relativeX = max(0.0f,min(1.0f,relativeX));
	relativeY = max(0.0f,min(1.0f,relativeY));

	// 스프라이트의 최종 위치 계산 (타일 좌표 + 타일 내 상대 위치)
	FPOINT spritePos = {
		x + relativeX,
		y + relativeY
	};

	// 근처에 다른 스프라이트가 있는지 확인 (일정 거리 내에 있으면 배치 불가)
	const float MIN_DISTANCE = 0.2f; // 최소 거리 (타일 크기의 20%)

	for(const auto& sprite : editorSprites) {
		float dx = sprite.pos.x - spritePos.x;
		float dy = sprite.pos.y - spritePos.y;
		float distance = sqrt(dx*dx + dy*dy);

		if(distance < MIN_DISTANCE) {
			return; // 너무 가까운 위치에 이미 스프라이트가 있음
		}
	}

	Texture* keyTexture = TextureManager::GetInstance()->GetTexture(TEXT("Image/jewel.bmp"));
	if(!keyTexture) {
		MessageBox(g_hWnd,TEXT("Item texture not found!"),TEXT("Error"),MB_OK);
		return;
	}

	Sprite newSprite;
	newSprite.pos = spritePos;
	newSprite.type = SpriteType::KEY;
	newSprite.texture = keyTexture;
	newSprite.distance = 0.0f;
	newSprite.aniInfo = {0.1f,0.1f,{456,488},{10,1},{0,0}};

	editorSprites.push_back(newSprite);
}

void MapEditor::RemoveObject(int x,int y)
{
	// 맵 범위 검사
	if(x < 0 || x >= mapWidth || y < 0 || y >= mapHeight)
		return;

	// 현재 모드에 따라 다른 삭제 동작
	switch(currentMode) {
	case EditMode::TILE:
	// 타일을 기본 바닥으로 변경 (가장자리는 벽 유지)
	{
		int index = y * mapWidth + x;
		if(index >= 0 && index < tiles.size()) {
			// 가장자리는 벽으로 유지
			if(x == 0 || y == 0 || x == mapWidth - 1 || y == mapHeight - 1) {
				tiles[index].roomType = RoomType::WALL;
				tiles[index].tilePos = 4; // 벽 타일 인덱스
			} else {
				tiles[index].roomType = RoomType::FLOOR;
				tiles[index].tilePos = 10; // 기본 바닥 타일
			}
		}
	}
	break;

	case EditMode::START:
	// 시작 위치는 삭제 불가
	MessageBox(g_hWnd,L"Cannot delete start position. Place a new one instead.",L"Info",MB_OK);
	break;

	case EditMode::OBSTACLE:
	// 장애물 삭제
	for(auto it = editorObstacles.begin(); it != editorObstacles.end(); ) {
		if(it->pos.x == x && it->pos.y == y) {
			it = editorObstacles.erase(it);
		} else {
			++it;
		}
	}
	break;

	case EditMode::MONSTER:
	case EditMode::ITEM:
	// 마우스 위치와 가장 가까운 스프라이트 찾기 (해당 타일 내에서)
	{
		// 마우스의 정확한 위치를 사용하여 타일 내에서의 상대적 위치 계산
		POINT tileScreenPos = TileToScreen({x,y});
		if(tileScreenPos.x < 0 || tileScreenPos.y < 0) return; // 예외 처리

		int tileWidth = (mapArea.right - mapArea.left) / (mapWidth / zoomLevel);
		int tileHeight = (mapArea.bottom - mapArea.top) / (mapHeight / zoomLevel);
		int tileSize = min(tileWidth,tileHeight);

		// 타일 내에서의 상대 위치 (0.0 ~ 1.0)
		float relativeX = (mousePos.x - tileScreenPos.x) / (float)tileSize;
		float relativeY = (mousePos.y - tileScreenPos.y) / (float)tileSize;

		// 범위 제한 (0.0 ~ 1.0)
		relativeX = max(0.0f,min(1.0f,relativeX));
		relativeY = max(0.0f,min(1.0f,relativeY));

		// 마우스 위치
		FPOINT mouseWorldPos = {
			x + relativeX,
			y + relativeY
		};

		// 가장 가까운 스프라이트 찾기
		float minDistance = FLT_MAX;
		auto closestSprite = editorSprites.end();

		for(auto it = editorSprites.begin(); it != editorSprites.end(); ++it) {
			// 현재 타일 내에 있는 스프라이트만 고려
			if((int)it->pos.x == x || (int)it->pos.y == y) {
				float dx = it->pos.x - mouseWorldPos.x;
				float dy = it->pos.y - mouseWorldPos.y;
				float distance = sqrt(dx*dx + dy*dy);

				if(distance < minDistance) {
					minDistance = distance;
					closestSprite = it;
				}
			}
		}

		// 일정 거리 내에 있는 경우에만 삭제
		const float DELETE_RADIUS = 0.3f; // 삭제 범위 (타일 크기의 30%)
		if(minDistance <= DELETE_RADIUS && closestSprite != editorSprites.end()) {
			editorSprites.erase(closestSprite);
		}
	}
	break;
	}
}
POINT MapEditor::ScreenToTile(POINT screenPos)
{
	POINT result = {-1,-1}; // 기본값으로 유효하지 않은 좌표 설정

	// 맵 영역 내의 좌표인지 확인
	if(PtInRect(&mapArea,screenPos))
	{
		// 타일 크기 계산
		int tileWidth = (mapArea.right - mapArea.left) / (mapWidth / zoomLevel);
		int tileHeight = (mapArea.bottom - mapArea.top) / (mapHeight / zoomLevel);
		int tileSize = min(tileWidth,tileHeight);

		if(tileSize <= 0) return result;

		// 맵 내 상대 좌표 계산
		float relX = (screenPos.x - mapArea.left) / (float)tileSize;
		float relY = (screenPos.y - mapArea.top) / (float)tileSize;

		// 뷰포트 오프셋 적용
		result.x = (int)(viewportOffset.x + relX);
		result.y = (int)(viewportOffset.y + relY);

		// 맵 범위 내로 제한
		result.x = max(0,min(result.x,mapWidth - 1));
		result.y = max(0,min(result.y,mapHeight - 1));
	}

	return result;
}

POINT MapEditor::TileToScreen(POINT tilePos)
{
	// 기본값으로 유효하지 않은 좌표 설정
	POINT result = {-1,-1};

	// 타일 위치가 맵 범위 내에 있는지 확인
	if(tilePos.x >= 0 && tilePos.x < mapWidth && tilePos.y >= 0 && tilePos.y < mapHeight)
	{
		// 타일 크기 계산
		int tileWidth = (mapArea.right - mapArea.left) / (mapWidth / zoomLevel);
		int tileHeight = (mapArea.bottom - mapArea.top) / (mapHeight / zoomLevel);
		int tileSize = min(tileWidth,tileHeight);

		if(tileSize <= 0) return result; 

		// 뷰포트 내 상대 위치
		float relX = tilePos.x - viewportOffset.x;
		float relY = tilePos.y - viewportOffset.y;

		// 화면 좌표 계산
		result.x = mapArea.left + (int)(relX * tileSize);
		result.y = mapArea.top + (int)(relY * tileSize);
	}

	return result;
}

void MapEditor::ChangeEditMode(EditMode mode)
{
	currentMode = mode;
}

void MapEditor::ResizeMap(int newWidth,int newHeight)
{
	if(newWidth <= 0 || newHeight <= 0)
		return;

	// 새 타일 배열
	vector<Room> newTiles(newWidth * newHeight);

	// 기본값으로 초기화
	for(int y = 0; y < newHeight; y++) {
		for(int x = 0; x < newWidth; x++) {
			int index = y * newWidth + x;
			if(x == 0 || y == 0 || x == newWidth - 1 || y == newHeight - 1) {
				newTiles[index].roomType = RoomType::WALL;
				newTiles[index].tilePos = 4; // 벽 타일
			} else {
				newTiles[index].roomType = RoomType::FLOOR;
				newTiles[index].tilePos = 10; // 바닥 타일
			}
		}
	}

	// 기존 맵 데이터 복사
	int copyWidth = min(mapWidth,newWidth);
	int copyHeight = min(mapHeight,newHeight);

	for(int y = 0; y < copyHeight; y++) {
		for(int x = 0; x < copyWidth; x++) {
			int oldIndex = y * mapWidth + x;
			int newIndex = y * newWidth + x;
			if(oldIndex < tiles.size() && newIndex < newTiles.size()) {
				newTiles[newIndex] = tiles[oldIndex];
			}
		}
	}

	// 맵 정보 업데이트
	mapWidth = newWidth;
	mapHeight = newHeight;
	tiles = newTiles;

	// 시작 위치가 맵 안에 있는지 확인
	if(startPosition.x >= newWidth || startPosition.y >= newHeight) {
		startPosition = {newWidth / 2.0f,newHeight / 2.0f};

		// 시작 타일로 설정
		int index = (int)startPosition.y * newWidth + (int)startPosition.x;
		if(index < tiles.size()) {
			tiles[index].roomType = RoomType::START;
		}
	}

	// 맵 범위를 벗어난 오브젝트 제거
	for(auto it = editorSprites.begin(); it != editorSprites.end(); ) {
		if(it->pos.x >= newWidth || it->pos.y >= newHeight) {
			it = editorSprites.erase(it);
		} else {
			++it;
		}
	}

	for(auto it = editorObstacles.begin(); it != editorObstacles.end(); ) {
		if(it->pos.x >= newWidth || it->pos.y >= newHeight) {
			it = editorObstacles.erase(it);
		} else {
			++it;
		}
	}

	// 뷰포트 리셋
	viewportOffset = {0.0f,0.0f};
}

void MapEditor::Zoom(float delta)
{
	if(delta == 0.0f) return;

	float newZoom = zoomLevel + delta;
	newZoom = max(0.5f,min(newZoom,2.0f)); // 0.5x ~ 2.0x 범위로 제한

	// 현재 마우스 위치를 기준으로 확대/축소
	if(mouseInMapArea) {
		// 타일 좌표 계산
		POINT tilePos = ScreenToTile(mousePos);

		zoomLevel = newZoom;

		// 뷰포트 조정
		POINT newScreenPos = TileToScreen(tilePos);
		float offsetX = (mousePos.x - newScreenPos.x) / (float)(mapArea.right - mapArea.left) * mapWidth / zoomLevel;
		float offsetY = (mousePos.y - newScreenPos.y) / (float)(mapArea.bottom - mapArea.top) * mapHeight / zoomLevel;

		viewportOffset.x += offsetX;
		viewportOffset.y += offsetY;
	} 
	else 
	{
		// 마우스가 맵 영역 밖이면 중앙 기준으로 확대/축소
		zoomLevel = newZoom;
	}

	// 뷰포트 범위 제한
	float maxOffsetX = max(0.0f,mapWidth - mapWidth / zoomLevel);
	float maxOffsetY = max(0.0f,mapHeight - mapHeight / zoomLevel);

	viewportOffset.x = max(0.0f,min(viewportOffset.x,maxOffsetX));
	viewportOffset.y = max(0.0f,min(viewportOffset.y,maxOffsetY));
}

void MapEditor::Scroll(float deltaX,float deltaY)
{
	viewportOffset.x += deltaX;
	viewportOffset.y += deltaY;

	// 뷰포트 범위 제한
	float maxOffsetX = max(0.0f,mapWidth - mapWidth / zoomLevel);
	float maxOffsetY = max(0.0f,mapHeight - mapHeight / zoomLevel);

	viewportOffset.x = max(0.0f,min(viewportOffset.x,maxOffsetX));
	viewportOffset.y = max(0.0f,min(viewportOffset.y,maxOffsetY));
}

void MapEditor::MouseWheel(int delta)
{
	if(KeyManager::GetInstance()->IsStayKeyDown(VK_CONTROL))
	{
		Zoom(delta > 0 ? 0.1f : -0.1f);
	} 
	else
	{
		VerticalScroll(delta);
	}
}

void MapEditor::VerticalScroll(int delta)
{
	float scrollAmount = 3.0f;

	if(delta > 0)
	{
		viewportOffset.y = max(0.0f,viewportOffset.y - scrollAmount / zoomLevel);
	} 
	else
	{
		float maxY = max(0.0f,mapHeight - mapHeight / zoomLevel);
		viewportOffset.y = min(maxY,viewportOffset.y + scrollAmount / zoomLevel);
	}
}

void MapEditor::HorizontalScroll(int delta)
{
	float scrollAmount = 3.0f;

	if(delta > 0)
	{
		viewportOffset.x = max(0.0f,viewportOffset.x - scrollAmount / zoomLevel);
	} 
	else
	{
		float maxX = max(0.0f,mapWidth - mapWidth / zoomLevel);
		viewportOffset.x = min(maxX,viewportOffset.x + scrollAmount / zoomLevel);
	}
}

void MapEditor::ChangeObstacleDirection(Direction dir)
{
	selectedObstacleDir = dir;
}

void MapEditor::SaveMap(const wchar_t* filePath)
{
	// 가장자리를 벽으로 강제 설정
	for(int y = 0; y < mapHeight; y++) {
		for(int x = 0; x < mapWidth; x++) {
			if(x == 0 || x == mapWidth - 1 || y == 0 || y == mapHeight - 1)
			{
				int index = y * mapWidth + x;
				tiles[index].roomType = RoomType::WALL;
				tiles[index].tilePos = 12;  // 벽 타일 인덱스
			}
		}
	}

	// 데이터 매니저에 맵 데이터 전달
	DataManager::GetInstance()->ClearAllData();
	DataManager::GetInstance()->SetMapData(tiles,mapWidth,mapHeight);
	DataManager::GetInstance()->SetTextureInfo(L"Image/tiles.bmp",128,SAMPLE_TILE_X,SAMPLE_TILE_Y);
	DataManager::GetInstance()->SetStartPosition(startPosition);

	// 아이템, 몬스터, 장애물 데이터 추가
	for(const auto& sprite : editorSprites) {
		if(sprite.type == SpriteType::KEY) 
		{
			ItemData item;
			item.pos = sprite.pos;
			item.aniInfo = {0.1f,0.1f,{456,488},{10,1},{0,0}}; // 기본 애니메이션 정보
			item.itemType = 0; // Key
			DataManager::GetInstance()->AddItemData(item);
		} 
		else if(sprite.type == SpriteType::MONSTER) 
		{
			MonsterData monster;
			monster.pos = sprite.pos;
			monster.aniInfo = {0.1f,0.1f,{423,437},{1,1},{0,0}}; // 기본 애니메이션 정보
			monster.monsterType = 0; // Tentacle
			DataManager::GetInstance()->AddMonsterData(monster);
		}
	}

	for(const auto& obstacle : editorObstacles) {
		ObstacleData obsData;
		obsData.pos = obstacle.pos;
		obsData.dir = obstacle.dir;
		DataManager::GetInstance()->AddObstacleData(obsData);
	}

	// 파일 저장
	if(DataManager::GetInstance()->SaveMapFile(filePath))
	{
		MessageBox(g_hWnd,TEXT("Map saved successfully!"),TEXT("Success"),MB_OK);
	}
	else 
	{
		MessageBox(g_hWnd,TEXT("Failed to save map!"),TEXT("Error"),MB_OK);
	}
}

void MapEditor::LoadMap(const wchar_t* filePath)
{
	if(!DataManager::GetInstance()->LoadMapFile(filePath)) 
	{
		MessageBox(g_hWnd,TEXT("Failed to load map!"),TEXT("Error"),MB_OK);
		return;
	}

	// 맵 데이터 로드
	MapData mapData;
	if(DataManager::GetInstance()->GetMapData(mapData))
	{
		// 타일 데이터 복원
		mapWidth = mapData.width;
		mapHeight = mapData.height;
		tiles = mapData.tiles;
	}

	// 시작 위치 복원
	startPosition = DataManager::GetInstance()->GetStartPosition();

	// 데이터 초기화
	editorSprites.clear();
	editorObstacles.clear();

	// 아이템 복원
	const auto& items = DataManager::GetInstance()->GetItems();
	for(const auto& item : items) {
		Sprite sprite;
		sprite.pos = item.pos;
		sprite.type = SpriteType::KEY;
		sprite.distance = 0.0f;
		editorSprites.push_back(sprite);
	}

	// 몬스터 복원
	const auto& monsters = DataManager::GetInstance()->GetMonsters();
	for(const auto& monster : monsters) {
		Sprite sprite;
		sprite.pos = monster.pos;
		sprite.type = SpriteType::MONSTER;
		sprite.distance = 0.0f;
		editorSprites.push_back(sprite);
	}

	// 장애물 복원
	const auto& obstacles = DataManager::GetInstance()->GetObstacles();
	for(const auto& obstacle : obstacles) {
		Obstacle editorObstacle;
		editorObstacle.pos = obstacle.pos;
		editorObstacle.dir = obstacle.dir;
		editorObstacle.block = TRUE;
		editorObstacles.push_back(editorObstacle);
	}

	// 뷰포트 초기화
	viewportOffset = {0.0f,0.0f};
	zoomLevel = 1.0f;

	MessageBox(g_hWnd,TEXT("Map loaded successfully!"),TEXT("Success"),MB_OK);
}

void MapEditor::ClearMap()
{
	// 기본 타일로 초기화
	for(int y = 0; y < mapHeight; y++) {
		for(int x = 0; x < mapWidth; x++) {
			int index = y * mapWidth + x;
			if(x == 0 || y == 0 || x == mapWidth - 1 || y == mapHeight - 1)
			{
				tiles[index].roomType = RoomType::WALL;
				tiles[index].tilePos = 4; // 벽 타일 인덱스
			} 
			else 
			{
				tiles[index].roomType = RoomType::FLOOR;
				tiles[index].tilePos = 10; // 바닥 타일 인덱스
			}
		}
	}

	// 시작 위치 재설정
	startPosition = {mapWidth / 2.0f,mapHeight / 2.0f};
	int startIndex = (int)startPosition.y * mapWidth + (int)startPosition.x;
	if(startIndex < tiles.size()) 
	{
		tiles[startIndex].roomType = RoomType::START;
	}

	// 스프라이트와 장애물 초기화
	editorSprites.clear();
	editorObstacles.clear();

	// 뷰포트 초기화
	viewportOffset = {0.0f,0.0f};
	zoomLevel = 1.0f;

	MessageBox(g_hWnd,TEXT("Map cleared!"),TEXT("Success"),MB_OK);
}