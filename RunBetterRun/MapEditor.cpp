#include "MapEditor.h"
#include "Image.h"
#include "Button.h"
#include "MapManager.h"
#include "TextureManager.h"
#include "SpriteManager.h"
#include "SceneManager.h"
#include "CommonFunction.h"

HRESULT MapEditor::Init()
{
	mode = EditorMode::TILE;
	selectedTile = {0,0};
	selectedRoomType = RoomType::FLOOR;

	sampleTileImage = ImageManager::GetInstance()->AddImage(
		"EditorSampleTile",L"Image/horrorMapTiles.bmp",
		1408,1408,SAMPLE_TILE_X,SAMPLE_TILE_Y,
		true,RGB(255,0,255));

	if(!sampleTileImage)
	{
		MessageBox(g_hWnd, TEXT("failed to load tile"), TEXT("error"), MB_OK);
		return E_FAIL;
	}

	sampleArea.left = 1600 - (SAMPLE_TILE_X * (TILE_SIZE + TILE_PADDING)) - 20;
	sampleArea.top = 100;
	sampleArea.right = sampleArea.left + (SAMPLE_TILE_X * (TILE_SIZE + TILE_PADDING));
	sampleArea.bottom = sampleArea.top + (SAMPLE_TILE_Y * (TILE_SIZE + TILE_PADDING));

	mapArea.left = 20;
	mapArea.top = 50;
	mapArea.right = mapArea.left + (VISIBLE_MAP_WIDTH * TILE_SIZE);
	mapArea.bottom = mapArea.top + (VISIBLE_MAP_HEIGHT * TILE_SIZE);

	InitTiles();
	InitButtons();
	InitItemMonsterSamples();

	while(ShowCursor(TRUE) < 0);

	return S_OK;
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
			} 
			else
			{
				if(x == 0 || y == 0 || x == MAP_EDITOR_WIDTH - 1 ||
					y == MAP_EDITOR_HEIGHT - 1)
				{
					mapTiles[y][x].tileIndex = 4;
					mapTiles[y][x].type = RoomType::WALL;
				}
				else
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

void MapEditor::InitItemMonsterSamples()
{
	// 아이템 샘플 초기화
	Texture* keyTexture = TextureManager::GetInstance()->GetTexture(TEXT("Image/jewel.bmp"));
	if(keyTexture)
	{
		sampleItem item;
		item.rc = {itemSampleArea.left + 10,itemSampleArea.top + 10,
			itemSampleArea.left + 50,itemSampleArea.top + 50};
		item.texture = keyTexture;
		item.type = SpriteType::KEY;
		itemSamples.push_back(item);
	}

	// 몬스터 샘플 초기화
	Texture* monsterTexture = TextureManager::GetInstance()->GetTexture(TEXT("Image/boss.bmp"));
	if(monsterTexture)
	{
		sampleItem monster;
		monster.rc = {monsterSampleArea.left + 10,monsterSampleArea.top + 10,
			monsterSampleArea.left + 50,monsterSampleArea.top + 50};
		monster.texture = monsterTexture;
		monster.type = SpriteType::MONSTER;
		monsterSamples.push_back(monster);
	}
}

void MapEditor::Release()
{
	for(auto& button : buttons) 
	{
		button->Release();
		delete button;
	}
	buttons.clear();
	editorSprites.clear();
}

void MapEditor::Update()
{
	GetCursorPos(&mousePos);
	ScreenToClient(g_hWnd,&mousePos);

	for(auto& button : buttons)
	{
		button->Update();
	}

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
			int relX = mousePos.x - sampleArea.left;
			int relY = mousePos.y - sampleArea.top;

			// 타일 중심을 기준으로 계산
			selectedTile.x = (relX + TILE_SIZE / 2) / TILE_SIZE;
			selectedTile.y = (relY + TILE_SIZE / 2) / TILE_SIZE;

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

void MapEditor::HandleMapEdit()
{
	if(PtInRect(&mapArea,mousePos))
	{
		if(KeyManager::GetInstance()->IsStayKeyDown(VK_LBUTTON))
		{
			int relX = mousePos.x - mapArea.left;
			int relY = mousePos.y - mapArea.top;

			// 타일 중심을 기준으로 계산
			int tileX = (relX + TILE_SIZE / 2) / TILE_SIZE;
			int tileY = (relY + TILE_SIZE / 2) / TILE_SIZE;

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

		if(KeyManager::GetInstance()->IsOnceKeyDown(VK_RBUTTON))
		{
			int relX = mousePos.x - mapArea.left;
			int relY = mousePos.y - mapArea.top;

			// 타일 중심을 기준으로 계산
			int tileX = (relX + TILE_SIZE / 2) / TILE_SIZE;
			int tileY = (relY + TILE_SIZE / 2) / TILE_SIZE;

			if(tileX >= 0 && tileX < VISIBLE_MAP_WIDTH &&
				tileY >= 0 && tileY < VISIBLE_MAP_HEIGHT)
			{
				if(mode == EditorMode::ITEM || mode == EditorMode::MONSTER)
				{
					RemoveSprite(tileX,tileY);
				} else if(mode == EditorMode::TILE)
				{
					mapTiles[tileY][tileX].tileIndex = 10;
					mapTiles[tileY][tileX].type = RoomType::FLOOR;
				}
			}
		}
	}
}

void MapEditor::HandleShortcut()
{
	if(KeyManager::GetInstance()->IsOnceKeyDown(VK_ESCAPE)) 
	{
		SceneManager::GetInstance()->ChangeScene("MainGameScene");
	}
}

void MapEditor::HandleItemSelect()
{
	if(PtInRect(&itemSampleArea,mousePos))
	{
		if(KeyManager::GetInstance()->IsOnceKeyDown(VK_LBUTTON))
		{
			for(int i = 0; i < itemSamples.size(); i++)
			{
				if(PtInRect(&itemSamples[i].rc,mousePos))
				{
					selectedItemIndex = i;
					break;
				}
			}
		}
	}
}

void MapEditor::HandleMonsterSelect()
{
	if(PtInRect(&monsterSampleArea,mousePos))
	{
		if(KeyManager::GetInstance()->IsOnceKeyDown(VK_LBUTTON))
		{
			for(int i = 0; i < monsterSamples.size(); i++)
			{
				if(PtInRect(&monsterSamples[i].rc,mousePos))
				{
					selectedItemIndex = i;
					break;
				}
			}
		}
	}
}

void MapEditor::Render(HDC hdc)
{
	PatBlt(hdc,0,0,1920,1080,WHITENESS);

	Rectangle(hdc,mapArea.left - 1,mapArea.top - 1,
			 mapArea.right + 1,mapArea.bottom + 1);

	RenderTiles(hdc);
	RenderSprites(hdc);

	Rectangle(hdc,sampleArea.left - 1,sampleArea.top - 1,
			 sampleArea.right + 1,sampleArea.bottom + 1);

	RenderSampleTiles(hdc);

	RECT selectedTileRect = {sampleArea.left, sampleArea.bottom + 10, 
							sampleArea.left + TILE_SIZE * 2, 
							sampleArea.bottom + 10 + TILE_SIZE * 2};
	Rectangle(hdc, selectedTileRect.left, selectedTileRect.top,
			selectedTileRect.right, selectedTileRect.bottom);

	if(sampleTileImage) {
		sampleTileImage->FrameRender(
			hdc, (selectedTileRect.left + selectedTileRect.right) / 2,
			(selectedTileRect.top + selectedTileRect.bottom) / 2,
			selectedTile.x,selectedTile.y);
	}

	TCHAR szText[128];
	LPCWSTR modeName = TEXT("Tile");

	if(mode == EditorMode::TILE)
	{
		modeName = TEXT("Tile");
	} else if(mode == EditorMode::START)
	{
		modeName = TEXT("Start");
	} else if(mode == EditorMode::ITEM)
	{
		modeName = TEXT("Item");
	} else if(mode == EditorMode::MONSTER)
	{
		modeName = TEXT("Monster");
	}

	wsprintf(szText,TEXT("CurrentMode: %s"),modeName);

	TextOut(hdc, sampleArea.left, sampleArea.bottom + 80,
			szText, lstrlen(szText));
	TextOut(hdc, sampleArea.left, sampleArea.bottom + 130, 
			TEXT("LeftClick: add, RightClick: delete/clear"),
			lstrlen(TEXT("LeftClick: add, RightClick: delete/clear")));
	TextOut(hdc, sampleArea.left, sampleArea.bottom + 170,
			TEXT("ESC: return to game"),
		    lstrlen(TEXT("ESC: return to game")));

	for(auto& button : buttons)
	{
		button->Render(hdc);
	}
}

void MapEditor::RenderTiles(HDC hdc)
{
	for(int y = 0; y < VISIBLE_MAP_HEIGHT; y++) 
	{
		for(int x = 0; x < VISIBLE_MAP_WIDTH; x++) 
		{
			int screenX = mapArea.left + x * TILE_SIZE;
			int screenY = mapArea.top + y * TILE_SIZE;

			int tileIndex = mapTiles[y][x].tileIndex;
			int frameX = tileIndex % SAMPLE_TILE_X;
			int frameY = tileIndex / SAMPLE_TILE_X;

			if(sampleTileImage)
			{
				sampleTileImage->FrameRender(hdc, 
					screenX + TILE_SIZE / 2,
					screenY + TILE_SIZE / 2, 
					frameX, frameY
				);
			}

			if(mapTiles[y][x].type == RoomType::START)
			{
				HBRUSH greenBrush = CreateSolidBrush(RGB(0,255,0));
				HBRUSH oldBrush = (HBRUSH)SelectObject(hdc,greenBrush);

				Ellipse(hdc,
					   screenX + TILE_SIZE / 4,
					   screenY + TILE_SIZE / 4,
					   screenX + TILE_SIZE - TILE_SIZE / 4,
					   screenY + TILE_SIZE - TILE_SIZE / 4);

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
		for(int y = 0; y < SAMPLE_TILE_Y; y++)
		{
			for(int x = 0; x < SAMPLE_TILE_X; x++)
			{
				int posX = sampleArea.left + x * (SAMPLE_TILE_SIZE + TILE_PADDING) + SAMPLE_TILE_SIZE / 2;
				int posY = sampleArea.top + y * (SAMPLE_TILE_SIZE + TILE_PADDING) + SAMPLE_TILE_SIZE / 2;

				sampleTileImage->FrameRender(hdc,posX,posY,x,y);
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
			int screenX = mapArea.left + tileX * TILE_SIZE + TILE_SIZE / 2;
			int screenY = mapArea.top + tileY * TILE_SIZE + TILE_SIZE / 2;

			COLORREF color = (sprite.type == SpriteType::KEY) ? RGB(0,0,255) :
				(sprite.type == SpriteType::MONSTER) ? RGB(255,0,0) :
				RGB(128,128,128);

			HBRUSH spriteBrush = CreateSolidBrush(color);
			HBRUSH oldBrush = (HBRUSH)SelectObject(hdc,spriteBrush);

			Ellipse(hdc,
				   screenX - TILE_SIZE / 4,
				   screenY - TILE_SIZE / 4,
				   screenX + TILE_SIZE / 4,
				   screenY + TILE_SIZE / 4);

			SelectObject(hdc,oldBrush);
			DeleteObject(spriteBrush);
		}
	}
}

void MapEditor::RenderItemSamples(HDC hdc)
{
	// 아이템 샘플 영역 테두리
	Rectangle(hdc,itemSampleArea.left - 1,itemSampleArea.top - 1,
			 itemSampleArea.right + 1,itemSampleArea.bottom + 1);

	TextOut(hdc,itemSampleArea.left,itemSampleArea.top - 20,TEXT("Items"),lstrlen(TEXT("Items")));

	// 아이템 샘플 그리기
	for(int i = 0; i < itemSamples.size(); i++)
	{
		sampleItem& item = itemSamples[i];

		// 선택된 아이템 표시
		if(mode == EditorMode::ITEM && selectedItemIndex == i)
		{
			HBRUSH selectBrush = CreateSolidBrush(RGB(200,200,255));
			HBRUSH oldBrush = (HBRUSH)SelectObject(hdc,selectBrush);
			Rectangle(hdc,item.rc.left - 2,item.rc.top - 2,item.rc.right + 2,item.rc.bottom + 2);
			SelectObject(hdc,oldBrush);
			DeleteObject(selectBrush);
		}

		// 아이템 아이콘 그리기
		HBRUSH itemBrush = CreateSolidBrush(RGB(0,0,255));
		HBRUSH oldBrush = (HBRUSH)SelectObject(hdc,itemBrush);
		Ellipse(hdc,item.rc.left,item.rc.top,item.rc.right,item.rc.bottom);
		SelectObject(hdc,oldBrush);
		DeleteObject(itemBrush);
	}
}

void MapEditor::RenderMonsterSamples(HDC hdc)
{
	// 몬스터 샘플 영역 그리기
	Rectangle(hdc,monsterSampleArea.left - 1,monsterSampleArea.top - 1,
			 monsterSampleArea.right + 1,monsterSampleArea.bottom + 1);

	TextOut(hdc,monsterSampleArea.left,monsterSampleArea.top - 20,TEXT("Monsters"),lstrlen(TEXT("Monsters")));

	// 몬스터 샘플 그리기
	for(int i = 0; i < monsterSamples.size(); i++)
	{
		sampleItem& monster = monsterSamples[i];

		// 선택된 몬스터 표시
		if(mode == EditorMode::MONSTER && selectedItemIndex == i)
		{
			HBRUSH selectBrush = CreateSolidBrush(RGB(255,200,200));
			HBRUSH oldBrush = (HBRUSH)SelectObject(hdc,selectBrush);
			Rectangle(hdc,monster.rc.left - 2,monster.rc.top - 2,monster.rc.right + 2,monster.rc.bottom + 2);
			SelectObject(hdc,oldBrush);
			DeleteObject(selectBrush);
		}

		// 몬스터 아이콘 그리기
		HBRUSH monsterBrush = CreateSolidBrush(RGB(255,0,0));
		HBRUSH oldBrush = (HBRUSH)SelectObject(hdc,monsterBrush);
		Ellipse(hdc,monster.rc.left,monster.rc.top,monster.rc.right,monster.rc.bottom);
		SelectObject(hdc,oldBrush);
		DeleteObject(monsterBrush);
	}
}

void MapEditor::RenderModeUI(HDC hdc)
{
	switch(mode)
	{
	case EditorMode::TILE:
		{
			// 샘플 타일 영역 테두리
			Rectangle(hdc,sampleArea.left - 1,sampleArea.top - 1,
					 sampleArea.right + 1,sampleArea.bottom + 1);
			// 샘플 타일 그리기
			RenderSampleTiles(hdc);

			// 현재 선택된 타일 표시
			RECT selectedTileRect = {sampleArea.left,sampleArea.bottom + 10,
									sampleArea.left + TILE_SIZE * 2,
									sampleArea.bottom + 10 + TILE_SIZE * 2};
			Rectangle(hdc,selectedTileRect.left,selectedTileRect.top,
						selectedTileRect.right,selectedTileRect.bottom);

		if(sampleTileImage)
		{
			sampleTileImage->FrameRender(
				hdc,(selectedTileRect.left + selectedTileRect.right) / 2,
				(selectedTileRect.top + selectedTileRect.bottom) / 2,
				selectedTile.x,selectedTile.y);
		}

		break;
		}
	case EditorMode::ITEM:
		RenderItemSamples(hdc);
		break;
	case EditorMode::MONSTER:
		RenderMonsterSamples(hdc);
		break;
	default:
		break;
	}

	// 모든 모드에서 표시되는 UI
	TCHAR szText[128];
	LPCWSTR modeName = TEXT("Tile");

	// 현재 모드에 따라 텍스트 결정
	if(mode == EditorMode::TILE)
	{
		modeName = TEXT("Tile");
	} 
	else if(mode == EditorMode::START)
	{
		modeName = TEXT("Start");
	} 
	else if(mode == EditorMode::ITEM)
	{
		modeName = TEXT("Item");
	} 
	else if(mode == EditorMode::MONSTER)
	{
		modeName = TEXT("Monster");
	}

	// 문자열 포맷팅
	wsprintf(szText,TEXT("CurrentMode: %s"),modeName);
	TextOut(hdc,sampleArea.left,sampleArea.bottom + 80,szText,lstrlen(szText));
}

void MapEditor::InitButtons()
{
	int buttonWidth = 80;
	int buttonHeight = 30;
	int buttonSpacing = 10;
	int startX = sampleArea.left;
	int startY = sampleArea.bottom + 220;

	Button* tileButton = new Button();
	tileButton->InitTextButton(startX, startY,
							buttonWidth, buttonHeight,TEXT("Tile"));
	tileButton->SetFunction([this]() { this->ChangeMode(EditorMode::TILE); });
	buttons.push_back(tileButton);

	Button* startButton = new Button();
	startButton->InitTextButton(startX + buttonWidth + buttonSpacing, startY, 
							buttonWidth, buttonHeight,TEXT("Start"));
	startButton->SetFunction([this]() { this->ChangeMode(EditorMode::START); });
	buttons.push_back(startButton);

	Button* itemButton = new Button();
	itemButton->InitTextButton(startX + (buttonWidth + buttonSpacing) * 2, startY,
							buttonWidth, buttonHeight,TEXT("Item"));
	itemButton->SetFunction([this]() { this->ChangeMode(EditorMode::ITEM); });
	buttons.push_back(itemButton);

	Button* monsterButton = new Button();
	monsterButton->InitTextButton(startX + (buttonWidth + buttonSpacing) * 3, startY,
								buttonWidth, buttonHeight,TEXT("Monster"));
	monsterButton->SetFunction([this]() { this->ChangeMode(EditorMode::MONSTER); });
	buttons.push_back(monsterButton);

	Button* saveButton = new Button();
	saveButton->InitTextButton(startX,startY + buttonHeight + buttonSpacing,
							   buttonWidth, buttonHeight,TEXT("Save"));
	saveButton->SetFunction([this]() { this->SaveMap(); });
	buttons.push_back(saveButton);

	Button* loadButton = new Button();
	loadButton->InitTextButton(startX + buttonWidth + buttonSpacing,startY + buttonHeight + buttonSpacing,
							   buttonWidth, buttonHeight,TEXT("Load"));
	loadButton->SetFunction([this]() { this->LoadMap(); });
	buttons.push_back(loadButton);

	Button* returnButton = new Button();
	returnButton->InitTextButton(startX + (buttonWidth + buttonSpacing) * 2,
								startY + buttonHeight + buttonSpacing,
								buttonWidth, buttonHeight, TEXT("Return"));
	returnButton->SetFunction([]() { SceneManager::GetInstance()->ChangeScene("MainGameScene"); });
	buttons.push_back(returnButton);

	Button* clearButton = new Button();
	clearButton->InitTextButton(startX + (buttonWidth + buttonSpacing) * 3,
								startY + buttonHeight + buttonSpacing,
								buttonWidth, buttonHeight, TEXT("Clear"));
	clearButton->SetFunction([this]() { this->ClearMap(); });
	buttons.push_back(clearButton);
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
	for(int j = 0; j < VISIBLE_MAP_HEIGHT; j++)
	{
		for(int i = 0; i < VISIBLE_MAP_WIDTH; i++)
		{
			if(mapTiles[j][i].type == RoomType::START) 
			{
				mapTiles[j][i].type = RoomType::FLOOR;
			}
		}
	}
	mapTiles[y][x].type = RoomType::START;
}

void MapEditor::PlaceItem(int x,int y)
{
	// 해당 위치에 이미 스프라이트가 있는지 확인
	if(FindSpriteAt(x,y) >= 0)
	{
		return; // 이미 있으면 배치하지 않음
	}

	if(selectedItemIndex >= 0 && selectedItemIndex < itemSamples.size())
	{
		sampleItem& sample = itemSamples[selectedItemIndex];
		FPOINT centerPos = {x + 0.5f,y + 0.5f};
		AddSprite(centerPos, sample.texture, sample.type);
	}
}

void MapEditor::PlaceMonster(int x, int y)
{
	// 해당 위치에 이미 스프라이트가 있는지 확인
	if(FindSpriteAt(x, y) >= 0)
	{
		return; // 이미 있으면 배치하지 않음
	}

	if(selectedItemIndex >= 0 && selectedItemIndex < monsterSamples.size())
	{
		sampleItem& sample = monsterSamples[selectedItemIndex];
		FPOINT centerPos = {x + 0.5f, y + 0.5f};
		AddSprite(centerPos, sample.texture, sample.type);
	}
}

void MapEditor::AddSprite(FPOINT position, Texture* texture, SpriteType type)
{
	Sprite newSprite;
	newSprite.pos = position;
	newSprite.texture = texture;
	newSprite.type = type;
	newSprite.distance = 0.0f;

	editorSprites.push_back(newSprite);
}

void MapEditor::RemoveSprite(int x,int y)
{
	int index = FindSpriteAt(x,y);
	if(index >= 0) {
		editorSprites.erase(editorSprites.begin() + index);
	}
}

int MapEditor::FindSpriteAt(int x,int y)
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
		mapFilePath, GENERIC_WRITE, 0, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if(mapFile == INVALID_HANDLE_VALUE)
	{
		MessageBox(g_hWnd,TEXT("failed to make file"),TEXT("error"),MB_OK);
		return;
	}

	DWORD bytesWritten = 0;

	MapData mapData;
	mapData.width = VISIBLE_MAP_WIDTH;
	mapData.height = VISIBLE_MAP_HEIGHT;
	mapData.tiles.resize(VISIBLE_MAP_WIDTH * VISIBLE_MAP_HEIGHT);
	mapData.texture = TextureManager::GetInstance()->GetTexture(TEXT("Image/horrorMapTiles.bmp"));
	mapData.textureTileSize = TILE_SIZE;
	mapData.textureTileRowSize = SAMPLE_TILE_X;
	mapData.textureTileColumnSize = SAMPLE_TILE_Y;

	// 타일 데이터 복사
	for(int y = 0; y < VISIBLE_MAP_HEIGHT; y++)
	{
		for(int x = 0; x < VISIBLE_MAP_WIDTH; x++)
		{
			int index = y * VISIBLE_MAP_WIDTH + x;
			mapData.tiles[index].roomType = mapTiles[y][x].type;
			mapData.tiles[index].tilePos = mapTiles[y][x].tileIndex;
		}
	}

	// 맵 크기 정보 저장
	WriteFile(mapFile, &mapData.width, sizeof(int), &bytesWritten, NULL);
	WriteFile(mapFile, &mapData.height, sizeof(int), &bytesWritten, NULL);
	int tileCount = mapData.tiles.size();
	WriteFile(mapFile, &tileCount, sizeof(int), &bytesWritten, NULL);
	WriteFile(mapFile, mapData.tiles.data(), sizeof(Room) * tileCount, &bytesWritten, NULL);

	// 아이템 개수와 위치 저장
	int itemCount = 0;
	for(const auto& sprite : editorSprites)
	{
		if(sprite.type == SpriteType::KEY)
		{
			itemCount++;
		}
	}
	WriteFile(mapFile, &itemCount, sizeof(int), &bytesWritten, NULL);
 
	for(const auto& sprite : editorSprites)
	{
		if(sprite.type == SpriteType::KEY)
		{
			WriteFile(mapFile, &sprite.pos, sizeof(FPOINT), &bytesWritten, NULL);
		}
	}

	// 몬스터 개수와 위치 저장
	int monsterCount = 0;

	for(const auto& sprite : editorSprites)
	{
		if(sprite.type == SpriteType::MONSTER)
		{
			monsterCount++;
		}
	}
	WriteFile(mapFile, &monsterCount, sizeof(int), &bytesWritten, NULL);

	for(const auto& sprite : editorSprites)
	{
		if(sprite.type == SpriteType::MONSTER)
		{
			WriteFile(mapFile, &sprite.pos, sizeof(FPOINT), &bytesWritten, NULL);
		}
	}

	CloseHandle(mapFile);

	// MapManager에도 현재 맵 데이터 복사
	MapData* managerMapData = MapManager::GetInstance()->GetMapData();
	managerMapData->width = mapData.width;
	managerMapData->height = mapData.height;
	managerMapData->tiles.resize(mapData.width * mapData.height);
	managerMapData->texture = mapData.texture;
	managerMapData->textureTileSize = mapData.textureTileSize;
	managerMapData->textureTileRowSize = mapData.textureTileRowSize;
	managerMapData->textureTileColumnSize = mapData.textureTileColumnSize;

	// 타일 데이터 복사
	for(int i = 0; i < mapData.width * mapData.height; i++)
	{
		managerMapData->tiles[i] = mapData.tiles[i];
	}

	MessageBox(g_hWnd, TEXT("map saved"), TEXT("map saved"), MB_OK);
}

void MapEditor::LoadMap()
{
	const LPCWCH mapFilePath = L"Map/EditorMap.dat";

	HANDLE mapFile = CreateFile(
		mapFilePath,GENERIC_READ,0,NULL,
		OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

	if(mapFile == INVALID_HANDLE_VALUE) 
	{
		MessageBox(g_hWnd,TEXT("Can't read file"),TEXT("error"),MB_OK);
		return;
	}

	DWORD bytesRead = 0;
	int mapWidth, mapHeight;

	ReadFile(mapFile, &mapWidth, sizeof(int), &bytesRead,NULL);
	ReadFile(mapFile, &mapHeight, sizeof(int), &bytesRead,NULL);

	int tileCount = 0;

	ReadFile(mapFile, &tileCount, sizeof(int), &bytesRead,NULL);

	MapData mapData;
	mapData.width = mapWidth;
	mapData.height = mapHeight;
	mapData.tiles.resize(tileCount);
	ReadFile(mapFile, mapData.tiles.data(), sizeof(Room) * tileCount, &bytesRead,NULL);

	for(int y = 0; y < min(VISIBLE_MAP_HEIGHT,mapHeight); y++) 
	{
		for(int x = 0; x < min(VISIBLE_MAP_WIDTH,mapWidth); x++) 
		{
			int index = y * mapWidth + x;
			mapTiles[y][x].tileIndex = mapData.tiles[index].tilePos;
			mapTiles[y][x].type = mapData.tiles[index].roomType;
		}
	}

	editorSprites.clear();

	int itemCount = 0;

	ReadFile(mapFile, &itemCount, sizeof(int), &bytesRead, NULL);

	Texture* itemTexture = TextureManager::GetInstance()->GetTexture(TEXT("Image/jewel.bmp"));

	for(int i = 0; i < itemCount; i++)
	{
		FPOINT pos;
		ReadFile(mapFile, &pos,sizeof(FPOINT), &bytesRead, NULL);

		if(itemTexture)
		{
			AddSprite(pos, itemTexture, SpriteType::KEY);
		}
	}

	int monsterCount = 0;

	ReadFile(mapFile,&monsterCount,sizeof(int),&bytesRead,NULL);

	Texture* monsterTexture = TextureManager::GetInstance()->GetTexture(TEXT("Image/boss.bmp"));

	for(int i = 0; i < monsterCount; i++)
	{
		FPOINT pos;
		ReadFile(mapFile,&pos,sizeof(FPOINT),&bytesRead,NULL);

		if(monsterTexture)
		{
			AddSprite(pos,monsterTexture,SpriteType::MONSTER);
		}
	}

	CloseHandle(mapFile);

	MapData* managerMapData = MapManager::GetInstance()->GetMapData();
	managerMapData->width = mapData.width;
	managerMapData->height = mapData.height;
	managerMapData->tiles.resize(mapData.width * mapData.height);
	managerMapData->texture = TextureManager::GetInstance()->GetTexture(TEXT("Image/horrorMapTiles.bmp"));
	managerMapData->textureTileSize = TILE_SIZE;
	managerMapData->textureTileRowSize = SAMPLE_TILE_X;
	managerMapData->textureTileColumnSize = SAMPLE_TILE_Y;

	for(int i = 0; i < mapData.width * mapData.height; i++)
	{
		managerMapData->tiles[i] = mapData.tiles[i];
	}

	MessageBox(g_hWnd,TEXT("map loaded"),TEXT("map loaded"),MB_OK);
}

void MapEditor::ClearMap()
{
	for(int y = 0; y < VISIBLE_MAP_HEIGHT; y++) 
	{
		for(int x = 0; x < VISIBLE_MAP_WIDTH; x++)
		{
			if(x == 0 || y == 0 ||
				x == VISIBLE_MAP_WIDTH - 1 ||
				y == VISIBLE_MAP_HEIGHT - 1) 
			{
				mapTiles[y][x].tileIndex = 4;
				mapTiles[y][x].type = RoomType::WALL;
			} 
			else 
			{
				mapTiles[y][x].tileIndex = 10;
				mapTiles[y][x].type = RoomType::FLOOR;
			}
		}
	}

	int centerX = VISIBLE_MAP_WIDTH / 2;
	int centerY = VISIBLE_MAP_HEIGHT / 2;
	mapTiles[centerY][centerX].type = RoomType::START;

	editorSprites.clear();

	MessageBox(g_hWnd,TEXT("clear complete"),TEXT("cleared"),MB_OK);
}