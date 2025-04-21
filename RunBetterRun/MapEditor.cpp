#include "MapEditor.h"
#include "Image.h"
#include "Button.h"
#include "TextureManager.h"
#include "SpriteManager.h"
#include "MapManager.h"
#include "DataManager.h"  
#include "CommonFunction.h"

HRESULT MapEditor::Init()
{
	mode = EditorMode::TILE;
	selectedTile = {0,0};
	selectedRoomType = RoomType::FLOOR;
	startPosition = {VISIBLE_MAP_WIDTH / 2.0f + 0.5f,VISIBLE_MAP_HEIGHT / 2.0f + 0.5f};

	// 타일 이미지 로드
	sampleTileImage = ImageManager::GetInstance()->AddImage(
		"EditorSampleTile",L"Image/horrorMapTiles.bmp",
		1408,1408,SAMPLE_TILE_X,SAMPLE_TILE_Y,
		true,RGB(255,0,255));

	if(!sampleTileImage)
	{
		MessageBox(g_hWnd,TEXT("Failed to load tile image"),TEXT("Error"),MB_OK);
		return E_FAIL;
	}

	// UI 레이아웃 계산
	int rightPanelWidth = 400;  // 오른쪽 패널 너비
	int uiPadding = 20;         // UI 요소 간 여백
	int sampleTileSize = 32;    // 샘플 타일 크기 (32x32 픽셀로 증가)

	// 오른쪽 패널 영역 설정
	int rightPanelLeft = WINSIZE_X - rightPanelWidth - uiPadding;

	// 샘플 타일 영역 설정
	sampleArea.left = rightPanelLeft;
	sampleArea.top = uiPadding * 3;
	sampleArea.right = sampleArea.left + (SAMPLE_TILE_X * sampleTileSize);
	sampleArea.bottom = sampleArea.top + (SAMPLE_TILE_Y * sampleTileSize);

	// 맵 영역 설정 - 왼쪽 영역을 최대한 활용
	int mapAreaWidth = rightPanelLeft - (uiPadding * 2);
	int mapTileSize = min(TILE_SIZE,mapAreaWidth / VISIBLE_MAP_WIDTH);

	mapArea.left = uiPadding;
	mapArea.top = uiPadding * 3;
	mapArea.right = mapArea.left + (VISIBLE_MAP_WIDTH * mapTileSize);
	mapArea.bottom = mapArea.top + (VISIBLE_MAP_HEIGHT * mapTileSize);

	// 기존 맵 로드 시도
	if(DataManager::GetInstance()->LoadMapFile(L"Map/EditorMap.dat"))
	{
		LoadFromDataManager();
	} else
	{
		InitTiles();
	}

	while(ShowCursor(TRUE) < 0);
	return S_OK;
}

void MapEditor::Release()
{
	editorSprites.clear();
}

void MapEditor::InitTiles()
{
	for(int y = 0; y < MAP_EDITOR_HEIGHT; y++)
	{
		for(int x = 0; x < MAP_EDITOR_WIDTH; x++)
		{
			mapTiles[y][x].rc.left = (x * TILE_SIZE);
			mapTiles[y][x].rc.top = (y * TILE_SIZE);
			mapTiles[y][x].rc.right = mapTiles[y][x].rc.left + TILE_SIZE;
			mapTiles[y][x].rc.bottom = mapTiles[y][x].rc.top + TILE_SIZE;

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

	int centerX = MAP_EDITOR_WIDTH / 2;
	int centerY = MAP_EDITOR_HEIGHT / 2;
	mapTiles[centerY][centerX].type = RoomType::START;
	startPosition = {centerX + 0.5f,centerY + 0.5f};
}

//업데이트 및 렌더

void MapEditor::Update()
{
	GetCursorPos(&mousePos);
	ScreenToClient(g_hWnd,&mousePos);

	TileSelect();
	MapEdit();
	Shortcut();
}

void MapEditor::TileSelect()
{
	if(PtInRect(&sampleArea,mousePos))
	{
		if(KeyManager::GetInstance()->IsOnceKeyDown(VK_LBUTTON))
		{
			int relX = mousePos.x - sampleArea.left;
			int relY = mousePos.y - sampleArea.top;

			int sampleTileSize = (sampleArea.right - sampleArea.left) / SAMPLE_TILE_X;

			selectedTile.x = relX / sampleTileSize;
			selectedTile.y = relY / sampleTileSize;

			selectedTile.x = max(0,min(selectedTile.x,SAMPLE_TILE_X - 1));
			selectedTile.y = max(0,min(selectedTile.y,SAMPLE_TILE_Y - 1));

			int tileIndex = selectedTile.y * SAMPLE_TILE_X + selectedTile.x;
			if(tileIndex < 5)
			{
				selectedRoomType = RoomType::WALL;
			}
			else
			{
				selectedRoomType = RoomType::FLOOR;
			}
		}
	}
}

void MapEditor::MapEdit()
{
	if(PtInRect(&mapArea,mousePos))
	{
		if(KeyManager::GetInstance()->IsStayKeyDown(VK_LBUTTON))
		{
			int relX = mousePos.x - mapArea.left;
			int relY = mousePos.y - mapArea.top;

			int tileX = relX / TILE_SIZE;
			int tileY = relY / TILE_SIZE;

			if(tileX >= 0 && tileX < VISIBLE_MAP_WIDTH &&
			   tileY >= 0 && tileY < VISIBLE_MAP_HEIGHT)
			{
				switch(mode)
				{
				case EditorMode::TILE:
					PlaceTile(tileX,tileY);
					break;
				case EditorMode::START:
					PlaceStartPoint(tileX,tileY);
					break;
				case EditorMode::ITEM:
					PlaceItem(tileX,tileY);
					break;
				case EditorMode::MONSTER:
					PlaceMonster(tileX,tileY);
					break;
				}
			}
		}

		if(KeyManager::GetInstance()->IsStayKeyDown(VK_RBUTTON))
		{
			int relX = mousePos.x - mapArea.left;
			int relY = mousePos.y - mapArea.top;

			int tileX = relX / TILE_SIZE;
			int tileY = relY / TILE_SIZE;

			if(tileX >= 0 && tileX < VISIBLE_MAP_WIDTH &&
			   tileY >= 0 && tileY < VISIBLE_MAP_HEIGHT)
			{
				if(mode == EditorMode::ITEM || mode == EditorMode::MONSTER)
				{
					RemoveSprite(tileX,tileY);
				} 
				else if(mode == EditorMode::TILE)
				{
					mapTiles[tileY][tileX].tileIndex = 10;
					mapTiles[tileY][tileX].type = RoomType::FLOOR;
				}
			}
		}
	}
}

void MapEditor::Shortcut()
{
	if(KeyManager::GetInstance()->IsOnceKeyDown(VK_ESCAPE))
	{
		SceneManager::GetInstance()->ChangeScene("MainGameScene");
	}

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

	if(KeyManager::GetInstance()->IsOnceKeyDown('S'))
	{
		SaveMap();
	} 
	else if(KeyManager::GetInstance()->IsOnceKeyDown('L'))
	{
		LoadMap();
	}
}

void MapEditor::Render(HDC hdc)
{
	// 배경 흰색으로 지우기
	PatBlt(hdc,0,0,WINSIZE_X,WINSIZE_Y,WHITENESS);

	// 맵 영역과 테두리 그리기
	Rectangle(hdc,mapArea.left - 1,mapArea.top - 1,
			 mapArea.right + 1,mapArea.bottom + 1);
	RenderTiles(hdc);
	RenderSprites(hdc);

	// 샘플 타일 영역과 테두리 그리기
	Rectangle(hdc,sampleArea.left - 1,sampleArea.top - 1,
			 sampleArea.right + 1,sampleArea.bottom + 1);
	RenderSampleTiles(hdc);

	// 현재 선택된 타일 표시 (크기 2배 증가)
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
	if(sampleTileImage) {
		sampleTileImage->FrameRender(
			hdc,
			(selectedTileRect.left + selectedTileRect.right) / 2,
			(selectedTileRect.top + selectedTileRect.bottom) / 2,
			selectedTile.x,selectedTile.y);
	}

	// 현재 모드 표시 (폰트 크기 및 색상 개선)
	TCHAR szText[128];
	LPCWSTR modeName = TEXT("Tile");
	COLORREF modeColor = RGB(255,255,255);

	if(mode == EditorMode::TILE) {
		modeName = TEXT("TILE MODE (select tile)");
		modeColor = RGB(255,255,0);
	} 
	else if(mode == EditorMode::START) {
		modeName = TEXT("START MODE (place player start position)");
		modeColor = RGB(120,255,120);
	} 
	else if(mode == EditorMode::ITEM) {
		modeName = TEXT("ITEM MODE (place item)");
		modeColor = RGB(200,200,255);
	} 
	else if(mode == EditorMode::MONSTER) {
		modeName = TEXT("MONSTER MODE (place monster)");
		modeColor = RGB(255,100,100);
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

	// 조작 설명 표시 (하단에 명확하게)
	HFONT guideFont = CreateFont(18,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,
						  DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
						  DEFAULT_QUALITY,DEFAULT_PITCH | FF_DONTCARE,TEXT("Arial"));
	SelectObject(hdc,guideFont);

	// 검은색 배경에 흰색 텍스트로 표시하여 가독성 높이기
	RECT guideRect = {0,WINSIZE_Y - 80,WINSIZE_X,WINSIZE_Y};
	FillRect(hdc,&guideRect,(HBRUSH)GetStockObject(BLACK_BRUSH));
	SetTextColor(hdc,RGB(255,255,255));

	TextOut(hdc,20,WINSIZE_Y - 70,TEXT("Left Click: Place / Right Click: Delete"),
			lstrlen(TEXT("Left Click: Place / Right Click: Delete")));
	TextOut(hdc,20,WINSIZE_Y - 50,TEXT("ESC: Return to Game"),
			lstrlen(TEXT("ESC: Return to Game")));
	TextOut(hdc,20,WINSIZE_Y - 30,TEXT("1-4: Change Mode, S: Save, L: Load"),
			lstrlen(TEXT("1-4: Change Mode, S: Save, L: Load")));

	SelectObject(hdc,oldFont);
	DeleteObject(guideFont);
}

void MapEditor::RenderTiles(HDC hdc)
{
	// 맵 섹션 제목 추가
	HFONT titleFont = CreateFont(20,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,
						  DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
						  DEFAULT_QUALITY,DEFAULT_PITCH | FF_DONTCARE,TEXT("Arial"));
	HFONT oldFont = (HFONT)SelectObject(hdc,titleFont);
	SetTextColor(hdc,RGB(0,0,0));

	TextOut(hdc,mapArea.left,mapArea.top - 25,TEXT("Map Editor"),
			lstrlen(TEXT("Map Editor")));

	SelectObject(hdc,oldFont);
	DeleteObject(titleFont);

	// 타일 크기 계산
	int tileSize = (mapArea.right - mapArea.left) / VISIBLE_MAP_WIDTH;

	for(int y = 0; y < VISIBLE_MAP_HEIGHT; y++)
	{
		for(int x = 0; x < VISIBLE_MAP_WIDTH; x++)
		{
			int screenX = mapArea.left + x * tileSize + (tileSize / 2);
			int screenY = mapArea.top + y * tileSize + (tileSize / 2);

			int tileIndex = mapTiles[y][x].tileIndex;
			int frameX = tileIndex % SAMPLE_TILE_X;
			int frameY = tileIndex / SAMPLE_TILE_X;

			if(sampleTileImage)
			{
				sampleTileImage->FrameRender(
					hdc,
					screenX,
					screenY,
					frameX,frameY,
					false,
					true
				);
			}

			// 시작 위치 강조 표시
			if(mapTiles[y][x].type == RoomType::START)
			{
				HBRUSH greenBrush = CreateSolidBrush(RGB(100,255,0));
				HPEN greenPen = CreatePen(PS_SOLID,2,RGB(0,160,0));

				HBRUSH oldBrush = (HBRUSH)SelectObject(hdc,greenBrush);
				HPEN oldPen = (HPEN)SelectObject(hdc,greenPen);

				Ellipse(hdc,
					   screenX - (tileSize / 3),
					   screenY - (tileSize / 3),
					   screenX + (tileSize / 3),
					   screenY + (tileSize / 3));

				SelectObject(hdc,oldPen);
				SelectObject(hdc,oldBrush);
				DeleteObject(greenPen);
				DeleteObject(greenBrush);
			}
		}
	}
}

void MapEditor::RenderSampleTiles(HDC hdc)
{
	if(sampleTileImage)
	{
		int sampleTileSize = (sampleArea.right - sampleArea.left) / SAMPLE_TILE_X;

		// 샘플 타일 섹션 제목 추가
		HFONT titleFont = CreateFont(20,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,
							  DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
							  DEFAULT_QUALITY,DEFAULT_PITCH | FF_DONTCARE,TEXT("Arial"));
		HFONT oldFont = (HFONT)SelectObject(hdc,titleFont);
		SetTextColor(hdc,RGB(0,0,0));

		TextOut(hdc,sampleArea.left,sampleArea.top - 25,TEXT("Tile Palette"),
				lstrlen(TEXT("Tile Palette")));

		SelectObject(hdc,oldFont);
		DeleteObject(titleFont);

		// 타일 렌더링
		for(int y = 0; y < SAMPLE_TILE_Y; y++)
		{
			for(int x = 0; x < SAMPLE_TILE_X; x++)
			{
				int posX = sampleArea.left + x * sampleTileSize + (sampleTileSize / 2);
				int posY = sampleArea.top + y * sampleTileSize + (sampleTileSize / 2);

				// 선택된 타일 강조 표시
				if(x == selectedTile.x && y == selectedTile.y)
				{
					// 더 눈에 띄는 선택 표시
					HPEN selectPen = CreatePen(PS_SOLID,3,RGB(255,0,0));
					HBRUSH selectBrush = CreateSolidBrush(RGB(255,230,230)); // 연한 빨강 배경

					HPEN oldPen = (HPEN)SelectObject(hdc,selectPen);
					HBRUSH oldBrush = (HBRUSH)SelectObject(hdc,selectBrush);

					Rectangle(hdc,
							posX - (sampleTileSize / 2) - 2,
							posY - (sampleTileSize / 2) - 2,
							posX + (sampleTileSize / 2) + 2,
							posY + (sampleTileSize / 2) + 2);

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
}

void MapEditor::RenderSprites(HDC hdc)
{
	for(const auto& sprite : editorSprites)
	{
		int tileX = static_cast<int>(sprite.pos.x - 0.5f);
		int tileY = static_cast<int>(sprite.pos.y - 0.5f);

		if(tileX >= 0 && tileX < VISIBLE_MAP_WIDTH &&
		   tileY >= 0 && tileY < VISIBLE_MAP_HEIGHT)
		{
			int screenX = mapArea.left + tileX * TILE_SIZE + (TILE_SIZE / 2);
			int screenY = mapArea.top + tileY * TILE_SIZE + (TILE_SIZE / 2);

			COLORREF color = (sprite.type == SpriteType::KEY) ? RGB(0,0,255) :
				(sprite.type == SpriteType::MONSTER) ? RGB(255,0,0) :
				RGB(128,128,128);

			HBRUSH spriteBrush = CreateSolidBrush(color);
			HBRUSH oldBrush = (HBRUSH)SelectObject(hdc,spriteBrush);

			Ellipse(hdc,
				   screenX - (TILE_SIZE / 3),
				   screenY - (TILE_SIZE / 3),
				   screenX + (TILE_SIZE / 3),
				   screenY + (TILE_SIZE / 3));

			SelectObject(hdc,oldBrush);
			DeleteObject(spriteBrush);
		}
	}

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

void MapEditor::ChangeMode(EditorMode newMode)
{
	mode = newMode;
}

void MapEditor::PlaceTile(int x,int y)
{
	int tileIndex = selectedTile.y * SAMPLE_TILE_X + selectedTile.x;
	mapTiles[y][x].tileIndex = tileIndex;
	mapTiles[y][x].type = selectedRoomType;
}

void MapEditor::PlaceStartPoint(int x,int y)
{
	for(int j = 0; j < MAP_EDITOR_HEIGHT; j++)
	{
		for(int i = 0; i < MAP_EDITOR_WIDTH; i++)
		{
			if(mapTiles[j][i].type == RoomType::START)
			{
				mapTiles[j][i].type = RoomType::FLOOR;
				mapTiles[j][i].tileIndex = 10;
			}
		}
	}

	mapTiles[y][x].type = RoomType::START;
	startPosition = {x + 0.5f,y + 0.5f};
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

void MapEditor::AddSprite(FPOINT position,Texture* texture,SpriteType type)
{
	Sprite newSprite;
	newSprite.pos = position;
	newSprite.texture = texture;
	newSprite.type = type;
	newSprite.distance = 0.0f;

	if(type == SpriteType::KEY)
	{
		newSprite.aniInfo = {0.1f,0.1f,{456,488},{10,1},{0,0}};
	} else if(type == SpriteType::MONSTER)
	{
		newSprite.aniInfo = {0.1f,0.1f,{423,437},{1,1},{0,0}};
	} else
	{
		newSprite.aniInfo = {0.1f,0.1f,{0,0},{1,1},{0,0}};
	}

	editorSprites.push_back(newSprite);
}

void MapEditor::RemoveSprite(int x,int y)
{
	int index = FindSprite(x,y);
	if(index >= 0)
	{
		editorSprites.erase(editorSprites.begin() + index);
	}
}

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
	PrepareDataForSave();

	if(DataManager::GetInstance()->SaveMapFile(L"Map/EditorMap.dat"))
	{
		MessageBox(g_hWnd,TEXT("Map Saved"),TEXT("Success"),MB_OK);
	} else
	{
		MessageBox(g_hWnd,TEXT("Failed to save map"),TEXT("Error"),MB_OK);
	}
}

void MapEditor::LoadMap()
{
	if(DataManager::GetInstance()->LoadMapFile(L"Map/EditorMap.dat"))
	{
		LoadFromDataManager();
		MessageBox(g_hWnd,TEXT("Map Loaded"),TEXT("Success"),MB_OK);
	} else
	{
		MessageBox(g_hWnd,TEXT("Failed to load map"),TEXT("Error"),MB_OK);
	}
}

void MapEditor::ClearMap()
{
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

	int centerX = VISIBLE_MAP_WIDTH / 2;
	int centerY = VISIBLE_MAP_HEIGHT / 2;
	mapTiles[centerY][centerX].type = RoomType::START;
	startPosition = {centerX + 0.5f,centerY + 0.5f};

	editorSprites.clear();

	MessageBox(g_hWnd,TEXT("Map Cleared"),TEXT("Success"),MB_OK);
}

void MapEditor::PrepareDataForSave()
{
	// 맵 타일 데이터 변환
	vector<Room> tiles;
	for(int y = 0; y < VISIBLE_MAP_HEIGHT; y++)
	{
		for(int x = 0; x < VISIBLE_MAP_WIDTH; x++)
		{
			Room tile;
			tile.roomType = mapTiles[y][x].type;
			tile.tilePos = mapTiles[y][x].tileIndex;
			tiles.push_back(tile);
		}
	}

	// DataManager 초기화 및 설정
	DataManager::GetInstance()->ClearAllData();
	DataManager::GetInstance()->SetMapData(tiles,VISIBLE_MAP_WIDTH,VISIBLE_MAP_HEIGHT);
	DataManager::GetInstance()->SetTextureInfo(L"Image/horrorMapTiles.bmp",128,11,11);
	DataManager::GetInstance()->SetStartPosition(startPosition);

	// 아이템 데이터 전달
	for(const auto& sprite : editorSprites)
	{
		if(sprite.type == SpriteType::KEY)
		{
			ItemData item;
			item.pos = sprite.pos;
			item.aniInfo = sprite.aniInfo;
			item.itemType = 0;  // Key
			DataManager::GetInstance()->AddItemData(item);
		}
	}

	// 몬스터 데이터 전달
	for(const auto& sprite : editorSprites)
	{
		if(sprite.type == SpriteType::MONSTER)
		{
			MonsterData monster;
			monster.pos = sprite.pos;
			monster.aniInfo = sprite.aniInfo;
			monster.monsterType = 0;  // Tentacle
			DataManager::GetInstance()->AddMonsterData(monster);
		}
	}
}

void MapEditor::LoadFromDataManager()
{
	// 맵 데이터 로드
	MapData mapData;
	if(DataManager::GetInstance()->GetMapData(mapData))
	{
		// 타일 데이터 복원
		for(int y = 0; y < mapData.height && y < MAP_EDITOR_HEIGHT; y++)
		{
			for(int x = 0; x < mapData.width && x < MAP_EDITOR_WIDTH; x++)
			{
				int index = y * mapData.width + x;
				mapTiles[y][x].tileIndex = mapData.tiles[index].tilePos;
				mapTiles[y][x].type = mapData.tiles[index].roomType;
			}
		}
	}

	// 시작 위치 복원
	startPosition = DataManager::GetInstance()->GetStartPosition();

	// 아이템 및 몬스터 복원
	editorSprites.clear();

	// 아이템 복원
	const auto& items = DataManager::GetInstance()->GetItems();
	for(const auto& item : items)
	{
		Sprite sprite;
		sprite.pos = item.pos;
		sprite.type = SpriteType::KEY;
		sprite.aniInfo = item.aniInfo;
		sprite.texture = TextureManager::GetInstance()->GetTexture(L"Image/jewel.bmp");
		sprite.distance = 0.0f;
		editorSprites.push_back(sprite);
	}

	// 몬스터 복원
	const auto& monsters = DataManager::GetInstance()->GetMonsters();
	for(const auto& monster : monsters)
	{
		Sprite sprite;
		sprite.pos = monster.pos;
		sprite.type = SpriteType::MONSTER;
		sprite.aniInfo = monster.aniInfo;
		sprite.texture = TextureManager::GetInstance()->GetTexture(L"Image/boss.bmp");
		sprite.distance = 0.0f;
		editorSprites.push_back(sprite);
	}
}