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

	sampleTileImage = ImageManager::GetInstance()->AddImage(
		"EditorSampleTile",L"Image/horrorMapTiles.bmp",
		1408,1408,SAMPLE_TILE_X,SAMPLE_TILE_Y,
		true,RGB(255,0,255));

	if(sampleTileImage)
	{
		MessageBox(g_hWnd,TEXT("Tile image loaded successfully"),TEXT("Debug"),MB_OK);
	}
	else
	{
		MessageBox(g_hWnd,TEXT("Failed to load tile image"),TEXT("Debug"),MB_OK);
		return E_FAIL;
	}

	// 샘플 타일 영역 설정
	int screenWidth = WINSIZE_X;
	int sampleTileSize = 16;
	sampleArea.left = WINSIZE_X - 400;
	sampleArea.top = 100;
	sampleArea.right = sampleArea.left + (SAMPLE_TILE_X * sampleTileSize);
	sampleArea.bottom = sampleArea.top + (SAMPLE_TILE_Y * sampleTileSize);

	// 맵 영역 설정
	mapArea.left = 50;
	mapArea.top = 50;
	mapArea.right = mapArea.left + (VISIBLE_MAP_WIDTH * TILE_SIZE);
	mapArea.bottom = mapArea.top + (VISIBLE_MAP_HEIGHT * TILE_SIZE);

	// 기존 맵 로드 시도
	if(DataManager::GetInstance()->LoadMapFile(L"Map/EditorMap.dat"))
	{
		LoadFromDataManager();
	} else
	{
		InitTiles();
	}

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

void MapEditor::InitButtons()
{
	int buttonWidth = 100;
	int buttonHeight = 30;
	int buttonSpacing = 10;
	int startX = sampleArea.left;
	int startY = sampleArea.bottom + 120;

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

//업데이트 및 렌더

void MapEditor::Update()
{
	GetCursorPos(&mousePos);
	ScreenToClient(g_hWnd,&mousePos);

	for(auto& button : buttons)
	{
		button->Update();
	}

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
	PatBlt(hdc,0,0,WINSIZE_X,WINSIZE_Y,WHITENESS);

	Rectangle(hdc,mapArea.left - 1,mapArea.top - 1,
			 mapArea.right + 1,mapArea.bottom + 1);

	RenderTiles(hdc);
	RenderSprites(hdc);

	Rectangle(hdc,sampleArea.left - 1,sampleArea.top - 1,
			 sampleArea.right + 1,sampleArea.bottom + 1);

	RenderSampleTiles(hdc);

	for(auto& button : buttons)
	{
		button->Render(hdc);
	}

	// 현재 선택된 타일 표시
	RECT selectedTileRect = {sampleArea.left,sampleArea.bottom + 10,
		sampleArea.left + TILE_SIZE * 2,
		sampleArea.bottom + 10 + TILE_SIZE * 2};
	Rectangle(hdc,selectedTileRect.left,selectedTileRect.top,
			selectedTileRect.right,selectedTileRect.bottom);

	if(sampleTileImage) {
		sampleTileImage->FrameRender(
			hdc,(selectedTileRect.left + selectedTileRect.right) / 2,
			(selectedTileRect.top + selectedTileRect.bottom) / 2,
			selectedTile.x,selectedTile.y);
	}

	// 현재 모드 표시
	TCHAR szText[128];
	LPCWSTR modeName = TEXT("Tile");
	COLORREF modeColor = RGB(255,255,255);

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
	for(int y = 0; y < VISIBLE_MAP_HEIGHT; y++)
	{
		for(int x = 0; x < VISIBLE_MAP_WIDTH; x++)
		{
			int screenX = mapArea.left + x * TILE_SIZE + (TILE_SIZE / 2);
			int screenY = mapArea.top + y * TILE_SIZE + (TILE_SIZE / 2);

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

			if(mapTiles[y][x].type == RoomType::START)
			{
				HBRUSH greenBrush = CreateSolidBrush(RGB(100,255,0));
				HBRUSH oldBrush = (HBRUSH)SelectObject(hdc,greenBrush);

				Ellipse(hdc,
					   screenX - (TILE_SIZE / 4),
					   screenY - (TILE_SIZE / 4),
					   screenX + (TILE_SIZE / 4),
					   screenY + (TILE_SIZE / 4));

				SelectObject(hdc,oldBrush);
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

		for(int y = 0; y < SAMPLE_TILE_Y; y++)
		{
			for(int x = 0; x < SAMPLE_TILE_X; x++)
			{
				int posX = sampleArea.left + x * sampleTileSize + (sampleTileSize / 2);
				int posY = sampleArea.top + y * sampleTileSize + (sampleTileSize / 2);

				if(x == selectedTile.x && y == selectedTile.y)
				{
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