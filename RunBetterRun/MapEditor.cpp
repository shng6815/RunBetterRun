#include "MapEditor.h"
#include "Image.h"
#include "Button.h"
#include "TextureManager.h"
#include "SpriteManager.h"
#include "MapManager.h"
#include "CommonFunction.h"

HRESULT MapEditor::Init()
{
	mode = EditorMode::TILE;
	selectedTile = {0,0};
	selectedRoomType = RoomType::FLOOR;

	mapOffset = {0,0};
	zoomLevel = 1.0f;

	// UI 레이아웃 
	InitLayout();

	sampleTileImage = ImageManager::GetInstance()->AddImage(
		"EditorSampleTile",L"Image/horrorMapTiles.bmp",
		1408,1408,SAMPLE_TILE_X,SAMPLE_TILE_Y,
		true,RGB(255,0,255));

	if(!sampleTileImage)
	{
		MessageBox(g_hWnd,TEXT("failed to load tile"),TEXT("error"),MB_OK);
		return E_FAIL;
	}

	InitTiles();
	InitButtons();

	while(ShowCursor(TRUE) < 0);
	return S_OK;
}

void MapEditor::Release()
{
	for(auto& button : buttons)
	{
		if(button)
		{
			button->Release();
			delete button;
			button = nullptr;
		}
	}
	buttons.clear();

	// 스프라이트 벡터 비우기
	editorSprites.clear();
}

void MapEditor::InitTiles()
{
	MapData* existingMap = MapManager::GetInstance()->GetMapData();

	for(int y = 0; y < MAP_EDITOR_HEIGHT; y++)
	{
		for(int x = 0; x < MAP_EDITOR_WIDTH; x++)
		{
			mapTiles[y][x].rc.left = (x * TILE_SIZE);
			mapTiles[y][x].rc.top = (y * TILE_SIZE);
			mapTiles[y][x].rc.right = mapTiles[y][x].rc.left + TILE_SIZE;
			mapTiles[y][x].rc.bottom = mapTiles[y][x].rc.top + TILE_SIZE;

			if(existingMap && x < existingMap->width && y < existingMap->height)
			{
				int index = y * existingMap->width + x;
				mapTiles[y][x].tileIndex = existingMap->tiles[index].tilePos;
				mapTiles[y][x].type = existingMap->tiles[index].roomType;
			} else
			{
				if(x == 0 || y == 0 || x == MAP_EDITOR_WIDTH - 1 ||
					y == MAP_EDITOR_HEIGHT - 1)
				{
					mapTiles[y][x].tileIndex = 4;
					mapTiles[y][x].type = RoomType::WALL;
				} else
				{
					mapTiles[y][x].tileIndex = 10;
					mapTiles[y][x].type = RoomType::FLOOR;
				}
			}
		}
	}

	int centerX = MAP_EDITOR_WIDTH / 2;
	int centerY = MAP_EDITOR_HEIGHT / 2;
	mapTiles[centerY][centerX].type = RoomType::START;
}

void MapEditor::InitButtons()
{
	// 버튼 생성 위치 및 크기 설정
	int buttonWidth = 100;
	int buttonHeight = 30;
	int buttonSpacing = 10;
	int startX = sampleArea.left + 1500;
	int startY = sampleArea.bottom + 100;

	// 모드 변경 버튼들
	Button* tileButton = new Button();
	tileButton->InitTextButton(startX,startY,
					  buttonWidth,buttonHeight,TEXT("Tile"));
	tileButton->SetFunction([this]() { this->ChangeMode(EditorMode::TILE); });
	buttons.push_back(tileButton);

	Button* startButton = new Button();
	startButton->InitTextButton(startX + buttonWidth + buttonSpacing,startY,
					   buttonWidth,buttonHeight,TEXT("Start"));
	startButton->SetFunction([this]() { this->ChangeMode(EditorMode::START); });
	buttons.push_back(startButton);

	Button* itemButton = new Button();
	itemButton->InitTextButton(startX,startY + buttonHeight + buttonSpacing,
					  buttonWidth,buttonHeight,TEXT("Item"));
	itemButton->SetFunction([this]() { this->ChangeMode(EditorMode::ITEM); });
	buttons.push_back(itemButton);

	Button* monsterButton = new Button();
	monsterButton->InitTextButton(startX + buttonWidth + buttonSpacing,startY + buttonHeight + buttonSpacing,
						 buttonWidth,buttonHeight,TEXT("Monster"));
	monsterButton->SetFunction([this]() { this->ChangeMode(EditorMode::MONSTER); });
	buttons.push_back(monsterButton);

	// 맵 저장/로드 버튼
	Button* saveButton = new Button();
	saveButton->InitTextButton(startX,startY + (buttonHeight + buttonSpacing) * 2,
					  buttonWidth,buttonHeight,TEXT("Save"));
	saveButton->SetFunction([this]() { this->SaveMap(); });
	buttons.push_back(saveButton);

	Button* loadButton = new Button();
	loadButton->InitTextButton(startX + buttonWidth + buttonSpacing,startY + (buttonHeight + buttonSpacing) * 2,
					  buttonWidth,buttonHeight,TEXT("Load"));
	loadButton->SetFunction([this]() { this->LoadMap(); });
	buttons.push_back(loadButton);
}


// UI 레이아웃 초기화 함수
void MapEditor::InitLayout()
{
	// 화면 크기 가져오기
	RECT rc;
	GetClientRect(g_hWnd,&rc);
	int screenWidth = rc.right - rc.left;
	int screenHeight = rc.bottom - rc.top;

	// 맵 뷰 영역 - 화면 왼쪽 2/3
	layout.mapViewArea = {
		50,50,
		50 + (screenWidth * 2 / 3) - 60,
		screenHeight - 100
	};

	// 샘플 타일 영역 - 화면 오른쪽 상단
	layout.sampleTileArea = {
		layout.mapViewArea.right + 20,
		50,
		screenWidth - 50,
		50 + (screenHeight / 2) - 20
	};

	// 컨트롤 패널 - 화면 오른쪽 중앙
	layout.controlPanel = {
		layout.mapViewArea.right + 20,
		layout.sampleTileArea.bottom + 20,
		screenWidth - 50,
		screenHeight - 180
	};

	// 상태 바 - 화면 하단
	layout.statusBar = {
		50,
		screenHeight - 120,
		screenWidth - 50,
		screenHeight - 20
	};
}

void MapEditor::Update()
{
	GetCursorPos(&mousePos);
	ScreenToClient(g_hWnd,&mousePos);

	for(auto& button : buttons)
	{
		button->Update();
	}

	HandleScroll();
	HandleZoom();
	HandleTileSelect();
	HandleMapEdit();
	HandleShortcut();
}

void MapEditor::HandleTileSelect()
{
	if(PtInRect(&sampleArea,mousePos))
	{
		if(KeyManager::GetInstance()->IsOnceKeyDown(VK_LBUTTON))
		{
			// 샘플 영역 내 상대 좌표
			int relX = mousePos.x - sampleArea.left;
			int relY = mousePos.y - sampleArea.top;

			// 샘플 타일 크기 계산
			int sampleTileSize = (sampleArea.right - sampleArea.left) / SAMPLE_TILE_X;

			// 타일 인덱스 계산
			selectedTile.x = relX / sampleTileSize;
			selectedTile.y = relY / sampleTileSize;

			// 경계 체크
			selectedTile.x = max(0,min(selectedTile.x,SAMPLE_TILE_X - 1));
			selectedTile.y = max(0,min(selectedTile.y,SAMPLE_TILE_Y - 1));

			// 타일 인덱스와 룸 타입 설정
			int tileIndex = selectedTile.y * SAMPLE_TILE_X + selectedTile.x;
			if(tileIndex < 5)
			{
				selectedRoomType = RoomType::WALL;
			} else
			{
				selectedRoomType = RoomType::FLOOR;
			}
		}
	}
}

// 타일 선택 및 배치 함수 수정
void MapEditor::HandleMapEdit()
{
	if(PtInRect(&layout.mapViewArea,mousePos))
	{
		if(KeyManager::GetInstance()->IsStayKeyDown(VK_LBUTTON))
		{
			// 화면 좌표를 맵 좌표로 변환
			POINT mapPos = ScreenToMapCoord(mousePos);

			if(mapPos.x >= 0 && mapPos.y >= 0)
			{
				switch(mode)
				{
				case EditorMode::TILE:
					PlaceTile(mapPos.x,mapPos.y);
					break;
				case EditorMode::START:
					PlaceStartPoint(mapPos.x,mapPos.y);
					break;
				case EditorMode::ITEM:
					PlaceItem(mapPos.x,mapPos.y);
					break;
				case EditorMode::MONSTER:
					PlaceMonster(mapPos.x,mapPos.y);
					break;
				}
			}
		}

		// 오른쪽 마우스 버튼으로 삭제
		if(KeyManager::GetInstance()->IsStayKeyDown(VK_RBUTTON))
		{
			// 화면 좌표를 맵 좌표로 변환
			POINT mapPos = ScreenToMapCoord(mousePos);

			if(mapPos.x >= 0 && mapPos.y >= 0)
			{
				if(mode == EditorMode::ITEM || mode == EditorMode::MONSTER)
				{
					// 아이템이나 몬스터 제거
					RemoveSprite(mapPos.x,mapPos.y);
				} else if(mode == EditorMode::TILE)
				{
					// 타일을 기본 바닥 타일로 리셋
					mapTiles[mapPos.y][mapPos.x].tileIndex = 10;
					mapTiles[mapPos.y][mapPos.x].type = RoomType::FLOOR;
				}
			}
		}
	}
}

void MapEditor::HandleShortcut()
{
	// ESC로 게임으로 돌아가기
	if(KeyManager::GetInstance()->IsOnceKeyDown(VK_ESCAPE))
	{
		SceneManager::GetInstance()->ChangeScene("MainGameScene");
	}

	// 모드 변경 단축키
	if(KeyManager::GetInstance()->IsOnceKeyDown('1'))
	{
		ChangeMode(EditorMode::TILE);
	} 
	else if(KeyManager::GetInstance()->IsOnceKeyDown('2'))
	{
		ChangeMode(EditorMode::START);
	} 
	else if(KeyManager::GetInstance()->IsOnceKeyDown('3'))
	{
		ChangeMode(EditorMode::ITEM);
	} 
	else if(KeyManager::GetInstance()->IsOnceKeyDown('4'))
	{
		ChangeMode(EditorMode::MONSTER);
	}
}

void MapEditor::Render(HDC hdc)
{
	PatBlt(hdc,0,0,WINSIZE_X,WINSIZE_Y,WHITENESS);

	// 맵 영역과 테두리
	Rectangle(hdc,mapArea.left - 1,mapArea.top - 1,
			 mapArea.right + 1,mapArea.bottom + 1);

	// 타일 그리기
	RenderTiles(hdc);

	// 스프라이트 그리기
	RenderSprites(hdc);

	//// 샘플 타일 영역 테두리
	//Rectangle(hdc,sampleArea.left - 1,sampleArea.top - 1,
	//		 sampleArea.right + 1,sampleArea.bottom + 1);

	// 샘플 타일 그리기
	RenderSampleTiles(hdc);

	// 버튼 그리기
	for(auto& button : buttons)
	{
		button->Render(hdc);
	}

	//// 현재 선택된 타일 표시
	//RECT selectedTileRect = {sampleArea.left,sampleArea.bottom + 10,
	//	sampleArea.left + TILE_SIZE * 2,
	//	sampleArea.bottom + 10 + TILE_SIZE * 2};
	//Rectangle(hdc,selectedTileRect.left,selectedTileRect.top,
	//		selectedTileRect.right,selectedTileRect.bottom);

	//if(sampleTileImage) {
	//	sampleTileImage->FrameRender(
	//		hdc,(selectedTileRect.left + selectedTileRect.right) / 2,
	//		(selectedTileRect.top + selectedTileRect.bottom) / 2,
	//		selectedTile.x,selectedTile.y);
	//}

	// 현재 모드 표시 강화
	TCHAR szText[128];
	LPCWSTR modeName = TEXT("Tile");
	COLORREF modeColor = RGB(255,255,255);  // 기본 검은색

	if(mode == EditorMode::TILE)
	{
		modeName = TEXT("TILE MODE (select tile)");
		modeColor = RGB(255,255,0);
	} 
	else if(mode == EditorMode::START)
	{
		modeName = TEXT("START MODE (place player start position)");
		modeColor = RGB(120,255,120);
	} 
	else if(mode == EditorMode::ITEM)
	{
		modeName = TEXT("ITEM MODE (place item)");
		modeColor = RGB(200,200,255);
	} 
	else if(mode == EditorMode::MONSTER)
	{
		modeName = TEXT("MONSTER MODE (place monster)");
		modeColor = RGB(255,100,100);
	}

	SetTextColor(hdc,modeColor);
	wsprintf(szText,TEXT("Current Mode: %s"),modeName);
	TextOut(hdc,20,20,szText,lstrlen(szText));
	SetTextColor(hdc,RGB(255,255,255)); 

	// 조작 설명 표시
	TextOut(hdc,20,WINSIZE_Y - 60,TEXT("Left Click: Place / Right Click: Delete"),
			lstrlen(TEXT("Left Click: Place / Right Click: Delete")));
	TextOut(hdc,20,WINSIZE_Y - 40,TEXT("ESC: Return to Game"),
			lstrlen(TEXT("ESC: Return to Game")));
	TextOut(hdc,20,WINSIZE_Y - 20,TEXT("1-4: Change Mode, S: Save, L: Load"),
			lstrlen(TEXT("1-4: Change Mode, S: Save, L: Load")));
}

void MapEditor::RenderTiles(HDC hdc)
{
	// 맵 영역 배경 그리기
	HBRUSH bgBrush = CreateSolidBrush(RGB(240,240,240));
	FillRect(hdc,&layout.mapViewArea,bgBrush);
	DeleteObject(bgBrush);

	// 화면에 보이는 타일 수 계산
	int visibleColumns = GetVisibleMapColumns();
	int visibleRows = GetVisibleMapRows();

	// 확대/축소를 고려한 타일 크기
	int tileRenderSize = TILE_SIZE * zoomLevel;

	// 타일 그리기
	for(int y = 0; y < visibleRows; y++)
	{
		for(int x = 0; x < visibleColumns; x++)
		{

			int mapX = x + mapOffset.x;
			int mapY = y + mapOffset.y;

			// 맵 범위 체크
			if(mapX >= MAP_EDITOR_WIDTH || mapY >= MAP_EDITOR_HEIGHT ||
			   mapX < 0 || mapY < 0)
				continue;

			// 화면 좌표 계산
			int screenX = layout.mapViewArea.left + x * tileRenderSize + (tileRenderSize / 2);
			int screenY = layout.mapViewArea.top + y * tileRenderSize + (tileRenderSize / 2);

			// 타일 인덱스 및 프레임 계산
			int tileIndex = mapTiles[mapY][mapX].tileIndex;
			int frameX = tileIndex % SAMPLE_TILE_X;
			int frameY = tileIndex / SAMPLE_TILE_X;

			// 타일 렌더링
			if(sampleTileImage)
			{
				// 정확한 중앙 위치에 타일 렌더링
				sampleTileImage->FrameRender(
					hdc,
					screenX,  // 정확한 중앙 X 좌표
					screenY,  // 정확한 중앙 Y 좌표
					frameX,frameY,
					false,
					true      // 중앙 기준 렌더링
				);
			}

			// 그리드 그리기
			HPEN gridPen = CreatePen(PS_SOLID,1,RGB(180,180,180));
			HPEN oldPen = (HPEN)SelectObject(hdc,gridPen);

			Rectangle(hdc,
				screenX - (tileRenderSize / 2),
				screenY - (tileRenderSize / 2),
				screenX + (tileRenderSize / 2),
				screenY + (tileRenderSize / 2));

			SelectObject(hdc,oldPen);
			DeleteObject(gridPen);

			// 시작 지점 표시
			if(mapTiles[mapY][mapX].type == RoomType::START)
			{
				HBRUSH greenBrush = CreateSolidBrush(RGB(100,255,0));
				HBRUSH oldBrush = (HBRUSH)SelectObject(hdc,greenBrush);

				// 중앙에 작은 원 표시
				Ellipse(hdc,
					screenX - (tileRenderSize / 4),
					screenY - (tileRenderSize / 4),
					screenX + (tileRenderSize / 4),
					screenY + (tileRenderSize / 4));

				SelectObject(hdc,oldBrush);
				DeleteObject(greenBrush);
			}

			// 벽 타일인 경우 색상 표시
			if(mapTiles[mapY][mapX].type == RoomType::WALL)
			{
				// 반투명 오버레이 표시 (벽 구분을 위해)
				HBRUSH wallBrush = CreateSolidBrush(RGB(255,200,200));
				HBRUSH oldBrush = (HBRUSH)SelectObject(hdc,wallBrush);

				RECT wallRect = {
					screenX - (tileRenderSize / 2),
					screenY - (tileRenderSize / 2),
					screenX + (tileRenderSize / 2),
					screenY + (tileRenderSize / 2)
				};

				// 반투명 효과를 주기 위해 패턴 브러쉬 활용
				FillRect(hdc,&wallRect,wallBrush);

				SelectObject(hdc,oldBrush);
				DeleteObject(wallBrush);
			}
		}
	}
}

void MapEditor::RenderSampleTiles(HDC hdc)
{
	if(sampleTileImage)
	{
		// 샘플 타일 간 간격
		int sampleTileSize = (sampleArea.right - sampleArea.left) / SAMPLE_TILE_X;

		for(int y = 0; y < SAMPLE_TILE_Y; y++)
		{
			for(int x = 0; x < SAMPLE_TILE_X; x++)
			{
				int posX = sampleArea.left + x * sampleTileSize + (sampleTileSize / 2);
				int posY = sampleArea.top + y * sampleTileSize + (sampleTileSize / 2);

				// 현재 선택된 타일 표시
				if(x == selectedTile.x && y == selectedTile.y)
				{
					// 선택된 타일 주변에 테두리 표시
					HPEN selectPen = CreatePen(PS_SOLID,2,RGB(255,0,0));
					HPEN oldPen = (HPEN)SelectObject(hdc,selectPen);

					Rectangle(hdc,
							posX - (sampleTileSize / 2) - 2,
							posY - (sampleTileSize / 2) - 2,
							posX + (sampleTileSize / 2) + 2,
							posY + (sampleTileSize / 2) + 2);

					SelectObject(hdc,oldPen);
					DeleteObject(selectPen);
				}

				// 타일 렌더링
				sampleTileImage->FrameRender(hdc,posX,posY,x,y,false,true);
			}
		}
	}
}

void MapEditor::ChangeMode(EditorMode newMode)
{
	mode = newMode;
}

void MapEditor::PlaceTile(int x,int y)
{
	// 가장자리벽 변경금지
	if(x == 0 || y == 0 || x == MAP_EDITOR_WIDTH - 1 || y == MAP_EDITOR_HEIGHT - 1)
	{
		return;
	}
	int tileIndex = selectedTile.y * SAMPLE_TILE_X + selectedTile.x;
	mapTiles[y][x].tileIndex = tileIndex;
	mapTiles[y][x].type = selectedRoomType;
}

void MapEditor::PlaceStartPoint(int x,int y)
{
	// 기존의 시작 위치를 찾아 FLOOR로 변경
	for(int j = 0; j < MAP_EDITOR_HEIGHT; j++)
	{
		for(int i = 0; i < MAP_EDITOR_WIDTH; i++)
		{
			if(mapTiles[j][i].type == RoomType::START)
			{
				mapTiles[j][i].type = RoomType::FLOOR;
				// 타일 인덱스도 바닥 타일로 변경
				mapTiles[j][i].tileIndex = 10;
			}
		}
	}

	// 새로운 시작 위치 설정
	mapTiles[y][x].type = RoomType::START;
	// 시작 위치의 타일은 유지 (바닥 타일로 보이지만 시작 위치임)
}

void MapEditor::PlaceItem(int x,int y)
{
	if(FindSprite(x,y) >= 0)
		return;

	Texture* keyTexture = TextureManager::GetInstance()->GetTexture(TEXT("Image/jewel.bmp"));
	if(keyTexture)
	{
		FPOINT centerPos = {x + 0.5f,y + 0.5f};
		AddSprite(centerPos,keyTexture,SpriteType::KEY);
	}
}

void MapEditor::PlaceMonster(int x,int y)
{
	if(FindSprite(x,y) >= 0)
		return;

	Texture* monsterTexture = TextureManager::GetInstance()->GetTexture(TEXT("Image/boss.bmp"));
	if(monsterTexture)
	{
		FPOINT centerPos = {x + 0.5f,y + 0.5f};
		AddSprite(centerPos,monsterTexture,SpriteType::MONSTER);
	}
}

// 스프라이트 검색 함수
int MapEditor::FindSprite(int x,int y)
{
	for(size_t i = 0; i < editorSprites.size(); i++)
	{
		float spriteX = editorSprites[i].pos.x - 0.5f;
		float spriteY = editorSprites[i].pos.y - 0.5f;

		if(static_cast<int>(spriteX) == x && static_cast<int>(spriteY) == y)
		{
			return i;
		}
	}
	return -1;
}

void MapEditor::SaveMap()
{
	const LPCWCH mapFilePath = L"Map/EditorMap.dat";
	CreateDirectory(L"Map",NULL);

	HANDLE mapFile = CreateFile(
		mapFilePath,GENERIC_WRITE,0,NULL,
		CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);

	if(mapFile == INVALID_HANDLE_VALUE)
	{
		MessageBox(g_hWnd,TEXT("failed to create file"),TEXT("error"),MB_OK);
		return;
	}

	DWORD bytesWritten = 0;

	// 맵 크기 저장
	int mapWidth = VISIBLE_MAP_WIDTH;
	int mapHeight = VISIBLE_MAP_HEIGHT;
	WriteFile(mapFile,&mapWidth,sizeof(int),&bytesWritten,NULL);
	WriteFile(mapFile,&mapHeight,sizeof(int),&bytesWritten,NULL);

	// 타일 수 저장
	int tileCount = mapWidth * mapHeight;
	WriteFile(mapFile,&tileCount,sizeof(int),&bytesWritten,NULL);

	// 타일 데이터 저장
	for(int y = 0; y < mapHeight; y++)
	{
		for(int x = 0; x < mapWidth; x++)
		{
			Room tile;
			tile.roomType = mapTiles[y][x].type;
			tile.tilePos = mapTiles[y][x].tileIndex;
			WriteFile(mapFile,&tile,sizeof(Room),&bytesWritten,NULL);
		}
	}

	// 아이템 개수 저장
	int itemCount = 0;
	for(const auto& sprite : editorSprites)
	{
		if(sprite.type == SpriteType::KEY)
			itemCount++;
	}
	WriteFile(mapFile,&itemCount,sizeof(int),&bytesWritten,NULL);

	// 아이템 위치 저장
	for(const auto& sprite : editorSprites)
	{
		if(sprite.type == SpriteType::KEY)
		{
			WriteFile(mapFile,&sprite.pos,sizeof(FPOINT),&bytesWritten,NULL);
		}
	}

	// 몬스터 개수 저장
	int monsterCount = 0;
	for(const auto& sprite : editorSprites)
	{
		if(sprite.type == SpriteType::MONSTER)
			monsterCount++;
	}
	WriteFile(mapFile,&monsterCount,sizeof(int),&bytesWritten,NULL);

	// 몬스터 위치 저장
	for(const auto& sprite : editorSprites)
	{
		if(sprite.type == SpriteType::MONSTER)
		{
			WriteFile(mapFile,&sprite.pos,sizeof(FPOINT),&bytesWritten,NULL);
		}
	}

	CloseHandle(mapFile);
	MessageBox(g_hWnd,TEXT("Map Saved"),TEXT("Success"),MB_OK);
}

void MapEditor::LoadMap()
{
	const LPCWCH mapFilePath = L"Map/EditorMap.dat";

	HANDLE mapFile = CreateFile(
		mapFilePath,GENERIC_READ,0,NULL,
		OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

	if(mapFile == INVALID_HANDLE_VALUE)
	{
		MessageBox(g_hWnd,TEXT("Failed to open file"),TEXT("Error"),MB_OK);
		return;
	}

	DWORD bytesRead = 0;

	// 맵 크기 읽기
	int mapWidth,mapHeight;
	ReadFile(mapFile,&mapWidth,sizeof(int),&bytesRead,NULL);
	ReadFile(mapFile,&mapHeight,sizeof(int),&bytesRead,NULL);

	// 타일 수 읽기
	int tileCount;
	ReadFile(mapFile,&tileCount,sizeof(int),&bytesRead,NULL);

	// 타일 데이터 읽기
	for(int y = 0; y < min(mapHeight,VISIBLE_MAP_HEIGHT); y++)
	{
		for(int x = 0; x < min(mapWidth,VISIBLE_MAP_WIDTH); x++)
		{
			Room tile;
			ReadFile(mapFile,&tile,sizeof(Room),&bytesRead,NULL);
			mapTiles[y][x].type = tile.roomType;
			mapTiles[y][x].tileIndex = tile.tilePos;
		}
	}

	// 아이템 데이터 읽기
	editorSprites.clear();

	int itemCount;
	ReadFile(mapFile,&itemCount,sizeof(int),&bytesRead,NULL);

	Texture* keyTexture = TextureManager::GetInstance()->GetTexture(TEXT("Image/jewel.bmp"));
	for(int i = 0; i < itemCount; i++)
	{
		FPOINT pos;
		ReadFile(mapFile,&pos,sizeof(FPOINT),&bytesRead,NULL);

		if(keyTexture)
		{
			Sprite sprite;
			sprite.pos = pos;
			sprite.texture = keyTexture;
			sprite.type = SpriteType::KEY;
			sprite.distance = 0.0f;
			// 기본 애니메이션 정보
			sprite.aniInfo = {0.1f,0.1f,{456,488},{10,1},{0,0}};

			editorSprites.push_back(sprite);
		}
	}

	// 몬스터 데이터 읽기
	int monsterCount;
	ReadFile(mapFile,&monsterCount,sizeof(int),&bytesRead,NULL);

	Texture* monsterTexture = TextureManager::GetInstance()->GetTexture(TEXT("Image/boss.bmp"));
	for(int i = 0; i < monsterCount; i++)
	{
		FPOINT pos;
		ReadFile(mapFile,&pos,sizeof(FPOINT),&bytesRead,NULL);

		if(monsterTexture)
		{
			Sprite sprite;
			sprite.pos = pos;
			sprite.texture = monsterTexture;
			sprite.type = SpriteType::MONSTER;
			sprite.distance = 0.0f;
			// 기본 애니메이션 정보
			sprite.aniInfo = {0.1f,0.1f,{423,437},{1,1},{0,0}};

			editorSprites.push_back(sprite);
		}
	}

	CloseHandle(mapFile);
	MessageBox(g_hWnd,TEXT("Map Loaded"),TEXT("Success"),MB_OK);
}

void MapEditor::ClearMap()
{
	// 맵 초기화
	for(int y = 0; y < VISIBLE_MAP_HEIGHT; y++)
	{
		for(int x = 0; x < VISIBLE_MAP_WIDTH; x++)
		{
			if(x == 0 || y == 0 || x == VISIBLE_MAP_WIDTH - 1 || y == VISIBLE_MAP_HEIGHT - 1)
			{
				mapTiles[y][x].tileIndex = 4;
				mapTiles[y][x].type = RoomType::WALL;
			} else
			{
				mapTiles[y][x].tileIndex = 10;
				mapTiles[y][x].type = RoomType::FLOOR;
			}
		}
	}

	// 시작 지점 설정
	int centerX = VISIBLE_MAP_WIDTH / 2;
	int centerY = VISIBLE_MAP_HEIGHT / 2;
	mapTiles[centerY][centerX].type = RoomType::START;

	// 스프라이트 제거
	editorSprites.clear();

	MessageBox(g_hWnd,TEXT("Map Cleared"),TEXT("Success"),MB_OK);
}

void MapEditor::AddSprite(FPOINT position,Texture* texture,SpriteType type)
{
	// 새 스프라이트 생성
	Sprite newSprite;
	newSprite.pos = position;
	newSprite.texture = texture;
	newSprite.type = type;
	newSprite.distance = 0.0f;

	// 스프라이트 타입에 따라 기본 애니메이션 정보 설정
	if(type == SpriteType::KEY)
	{
		// 키 아이템의 애니메이션 정보
		newSprite.aniInfo = {0.1f,0.1f,{456,488},{10,1},{0,0}};
	} else if(type == SpriteType::MONSTER)
	{
		// 몬스터의 애니메이션 정보
		newSprite.aniInfo = {0.1f,0.1f,{423,437},{1,1},{0,0}};
	} else
	{
		// 기본 애니메이션 정보
		newSprite.aniInfo = {0.1f,0.1f,{0,0},{1,1},{0,0}};
	}

	editorSprites.push_back(newSprite);
}

// 스프라이트 제거 
void MapEditor::RemoveSprite(int x,int y)
{
	int index = FindSprite(x,y);
	if(index >= 0)
	{
		editorSprites.erase(editorSprites.begin() + index);
	}
}

// 스프라이트 렌더
void MapEditor::RenderSprites(HDC hdc)
{
	for(const auto& sprite : editorSprites)
	{
		// 스프라이트의 타일 좌표 계산
		int tileX = static_cast<int>(sprite.pos.x - 0.5f);
		int tileY = static_cast<int>(sprite.pos.y - 0.5f);

		if(tileX >= 0 && tileX < VISIBLE_MAP_WIDTH &&
		   tileY >= 0 && tileY < VISIBLE_MAP_HEIGHT)
		{
			// 화면 좌표 계산 (타일 중앙)
			int screenX = mapArea.left + tileX * TILE_SIZE + (TILE_SIZE / 2);
			int screenY = mapArea.top + tileY * TILE_SIZE + (TILE_SIZE / 2);

			COLORREF color = (sprite.type == SpriteType::KEY) ? RGB(0,0,255) :
				(sprite.type == SpriteType::MONSTER) ? RGB(255,0,0) :
				RGB(128,128,128);

			HBRUSH spriteBrush = CreateSolidBrush(color);
			HBRUSH oldBrush = (HBRUSH)SelectObject(hdc,spriteBrush);

			// 중앙에 원 그리기
			Ellipse(hdc,
				   screenX - (TILE_SIZE / 3),
				   screenY - (TILE_SIZE / 3),
				   screenX + (TILE_SIZE / 3),
				   screenY + (TILE_SIZE / 3));

			SelectObject(hdc,oldBrush);
			DeleteObject(spriteBrush);
		}
	}

	// 배치된 스프라이트 수 표시
	int itemCount = 0,monsterCount = 0;
	for(const auto& sprite : editorSprites)
	{
		if(sprite.type == SpriteType::KEY) itemCount++;
		else if(sprite.type == SpriteType::MONSTER) monsterCount++;
	}

	TCHAR szCount[128];
	wsprintf(szCount,TEXT("Items: %d, Monsters: %d"),itemCount,monsterCount);
	TextOut(hdc,mapArea.left,mapArea.bottom + 10,szCount,lstrlen(szCount));
}

// 맵 스크롤 처리 함수
void MapEditor::HandleScroll()
{
	static float scrollAccumulator = 0.0f;
	float deltaTime = TimerManager::GetInstance()->GetDeltaTime();
	float scrollSpeed = 10.0f * deltaTime;  // 스크롤 속도

	// 방향키로 맵이동
	if(KeyManager::GetInstance()->IsStayKeyDown('A')){
		mapOffset.x = max(0,mapOffset.x - 1);
	}
	if(KeyManager::GetInstance()->IsStayKeyDown('D')) {
		mapOffset.x = min(MAP_EDITOR_WIDTH - GetVisibleMapColumns(),mapOffset.x + 1);
	}
	if(KeyManager::GetInstance()->IsStayKeyDown('W')) {
		mapOffset.y = max(0,mapOffset.y - 1);
	}
	if(KeyManager::GetInstance()->IsStayKeyDown('S')) {
		mapOffset.y = min(MAP_EDITOR_HEIGHT - GetVisibleMapRows(),mapOffset.y + 1);
	}
	mapOffset.x = max(0,min(MAP_EDITOR_WIDTH - GetVisibleMapColumns(),mapOffset.x));
	mapOffset.y = max(0,min(MAP_EDITOR_HEIGHT - GetVisibleMapRows(),mapOffset.y));
}

void MapEditor::HandleZoom()
{
	// +/- 키로 확대/축소
	if(KeyManager::GetInstance()->IsOnceKeyDown(VK_OEM_PLUS) ||
		KeyManager::GetInstance()->IsOnceKeyDown(VK_ADD)) {
		zoomLevel = min(2.0f,zoomLevel + 0.1f);
	}

	if(KeyManager::GetInstance()->IsOnceKeyDown(VK_OEM_MINUS) ||
		KeyManager::GetInstance()->IsOnceKeyDown(VK_SUBTRACT)) {
		zoomLevel = max(0.5f,zoomLevel - 0.1f);
	}
}

// 화면 좌표를 맵 좌표로 변환하는 함수
POINT MapEditor::ScreenToMapCoord(POINT screenPos)
{
	// 맵 영역 외부면 (-1, -1) 반환
	if(!PtInRect(&layout.mapViewArea,screenPos)) {
		return {-1,-1};
	}

	// 확대/축소를 고려한 타일 크기
	int tileRenderSize = TILE_SIZE * zoomLevel;

	// 맵 영역 내 상대 좌표 계산
	int relX = screenPos.x - layout.mapViewArea.left;
	int relY = screenPos.y - layout.mapViewArea.top;

	// 타일 좌표 계산
	int tileX = relX / tileRenderSize;
	int tileY = relY / tileRenderSize;

	// 맵 오프셋 적용
	int mapX = tileX + mapOffset.x;
	int mapY = tileY + mapOffset.y;

	// 맵 범위 확인
	if(mapX < 0 || mapX >= MAP_EDITOR_WIDTH ||
		mapY < 0 || mapY >= MAP_EDITOR_HEIGHT) {
		return {-1,-1};
	}

	return {mapX,mapY};
}

int MapEditor::GetVisibleMapColumns() {
	return (layout.mapViewArea.right - layout.mapViewArea.left) / TILE_SIZE;
}

int MapEditor::GetVisibleMapRows() {
	return (layout.mapViewArea.bottom - layout.mapViewArea.top) / TILE_SIZE;
}