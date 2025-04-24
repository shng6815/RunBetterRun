#include "MainGameScene.h"
#include "RayCast.h"
#include "Player.h"
#include "MonsterManager.h"
#include "SpriteManager.h"
#include "ItemManager.h"
#include "UIManager.h"
#include "Key.h"
#include "Tentacle.h"
#include "ObstacleManager.h"
#include "Pile.h"
#include "DataManager.h"
#include "SoundManager.h"
#include "Display.h"
#include "Elevator.h"
#include "Phone.h"
#include "Stun.h"
#include "Insight.h"

HRESULT MainGameScene::Init()
{
	rayCasting = new RayCast();
	if (FAILED(rayCasting->Init()))
	{
		MessageBox(g_hWnd, TEXT("error"), TEXT("error"), MB_OK);
		return E_FAIL;
	}

	MapManager::GetInstance()->Init(L"Map/EditorMap.dat");
	SpriteManager::GetInstance()->Init();
	Player::GetInstance()->Init([&](float shakePower, float time, bool isStepShake) { ShakeScreen(shakePower, time, isStepShake); });
	MonsterManager::GetInstance()->Init();
	ItemManager::GetInstance()->Init();
	ObstacleManager::GetInstance()->Init();

	const auto& items = DataManager::GetInstance()->GetItems();
	for(const auto& itemData : items)
	{
		Key* key = new Key(itemData.pos);
		key->SetAnimInfo(itemData.aniInfo);
		ItemManager::GetInstance()->PutItem(key);
	}
	const auto& monsters = DataManager::GetInstance()->GetMonsters();
	for(const auto& monsterData : monsters)
	{
		Tentacle* tentacle = new Tentacle(monsterData.pos);
		tentacle->SetAnimInfo(monsterData.aniInfo);
		MonsterManager::GetInstance()->PutMonster(tentacle);
	}
	const auto& obstacles = DataManager::GetInstance()->GetObstacles();
	for(const auto& obstacleData : obstacles)
	{
		Pile* pile = new Pile(obstacleData.pos,obstacleData.dir);
		ObstacleManager::GetInstance()->PutObstacle(pile);
	}

	UIManager::GetInstance()->Init();
	UIManager::GetInstance()->ChangeUIType(UIType::PLAYING);

	InitButtons();

	status = SceneStatus::IN_GAME;
	while(ShowCursor(FALSE) >= 0);

	RECT rc;
	GetClientRect(g_hWnd, &rc);

	HDC screenDC = GetDC(g_hWnd);
	backBufferDC = CreateCompatibleDC(screenDC);
	backBufferBitmap = CreateCompatibleBitmap(screenDC, rc.right, rc.bottom);
	oldBitmap = (HBITMAP)SelectObject(backBufferDC, backBufferBitmap);
	ReleaseDC(g_hWnd, screenDC);

	SoundManager::GetInstance()->LoadMusic("GameSceneBGM","Sounds/BGM_InGame.wav");

	SoundManager::GetInstance()->PlayMusic("GameSceneBGM",true,0.5f);

	/*ItemManager::GetInstance()->PutItem(new Key({ 21.5, 10.5 }));
	MonsterManager::GetInstance()->PutMonster(new Tentacle({ 21.5, 8.5 }));
	MonsterManager::GetInstance()->PutMonster(new Tentacle({21.5,7.5}));
	ObstacleManager::GetInstance()->PutObstacle(new Pile({21,21},Direction::WEST));
	ObstacleManager::GetInstance()->PutObstacle(new Pile({21,19},Direction::EAST));
	ObstacleManager::GetInstance()->PutObstacle(new Pile({21,20},Direction::NORTH));
	ObstacleManager::GetInstance()->PutObstacle(new Pile({21,18},Direction::SOUTH));*/

	return S_OK;
}

HRESULT MainGameScene::Init(LPCWCH)
{
	return E_NOTIMPL;
}

void MainGameScene::Release()
{
	if (rayCasting)
	{
		rayCasting->Release();
		delete rayCasting;
		rayCasting = nullptr;
	}
	ItemManager::GetInstance()->Release();
	MonsterManager::GetInstance()->Release();
	Player::GetInstance()->Release();
	SpriteManager::GetInstance()->Release();
	MapManager::GetInstance()->Release();

	UIManager::GetInstance()->Release();

	SelectObject(backBufferDC, oldBitmap);
	DeleteObject(backBufferBitmap);
	DeleteDC(backBufferDC);
}

void MainGameScene::Update()
{
	POINT cursor;
	GetCursorPos(&cursor);
	ScreenToClient(g_hWnd,&cursor);
	mousePos = cursor;

	switch (status)
	{
	case MainGameScene::SceneStatus::IN_GAME:
		if (KeyManager::GetInstance()->IsOnceKeyDown(VK_ESCAPE)) 
		{
			while(ShowCursor(TRUE) < 0);

			status = SceneStatus::PAUSE;
		}

		if (KeyManager::GetInstance()->IsOnceKeyDown('M'))
		{
			UIManager::GetInstance()->ToggleActiveUIUnit("PhoneUI");
		}
		
		Player::GetInstance()->Update();
		if (rayCasting)
			rayCasting->Update();
		SpriteManager::GetInstance()->SortSpritesByDistance();
		MonsterManager::GetInstance()->Update();
		ItemManager::GetInstance()->Update();
		UIManager::GetInstance()->Update();
		ObstacleManager::GetInstance()->Update();
		break;

	case MainGameScene::SceneStatus::PAUSE:
		CheckButtonHover();

		if(KeyManager::GetInstance()->IsOnceKeyDown(VK_LBUTTON))
		{
			for(auto& button: buttons)
			{
				if(button.state==PauseButtonState::HOVER)
				{
					HandleButtonClick(button);
					break;
				}
			}
		}

		if (KeyManager::GetInstance()->IsOnceKeyDown(VK_ESCAPE)) 
		{
			while(ShowCursor(FALSE) >= 0);
			status = SceneStatus::IN_GAME;
		}

		if(KeyManager::GetInstance()->IsOnceKeyDown('T'))
		{
			SceneManager::GetInstance()->ChangeScene("MapEditorScene");
		}

		break;

	case MainGameScene::SceneStatus::QUIT:
		break;

	default:
		break;
	}

	UIManager::GetInstance()->Update();
}

void MainGameScene::Render(HDC hdc)
{
	rayCasting->Render(backBufferDC);

	ApplyShake(hdc);

	UIManager::GetInstance()->Render(hdc);

	if(status == SceneStatus::PAUSE)
	{
		RenderPauseOverlay(hdc);
		
		for(auto & button : buttons)
		{
			RenderPauseMenu(hdc, button);
		}
	}
}

void MainGameScene::ShakeScreen(float shakePower, float time, bool isStepShake)
{
	screenShake.Start(shakePower, time, isStepShake);
}

void MainGameScene::ApplyShake(HDC hdc)
{
	screenShake.Update(TimerManager::GetInstance()->GetDeltaTime());

	POINT offset = screenShake.GetOffset();

	BitBlt(hdc, offset.x, offset.y, WINSIZE_X, WINSIZE_Y, backBufferDC, 0, 0, SRCCOPY);
}

void MainGameScene::RenderPauseOverlay(HDC hdc)
{
	RECT rc;
	GetClientRect(g_hWnd,&rc);

	BLENDFUNCTION bf;
	bf.BlendOp = AC_SRC_OVER; 
	bf.BlendFlags = 0;
	bf.SourceConstantAlpha = 150;
	bf.AlphaFormat = 0;

	HDC tempDC = CreateCompatibleDC(hdc);
	HBITMAP tempBitmap = CreateCompatibleBitmap(hdc,rc.right,rc.bottom);
	HBITMAP oldBitmap = (HBITMAP)SelectObject(tempDC,tempBitmap);
	
	HBRUSH blackBrush = CreateSolidBrush(RGB(0,0,0));
	FillRect(tempDC,&rc,blackBrush);
	DeleteObject(blackBrush);

	AlphaBlend(hdc, 0, 0, rc.right, rc.bottom,tempDC,0,0,rc.right,rc.bottom,bf);

	SelectObject(tempDC,oldBitmap);
	DeleteObject(tempBitmap);
	DeleteDC(tempDC);
}

void MainGameScene::RenderPauseMenu(HDC hdc,PauseButton& button)
{
	COLORREF textColor = RGB(220,220,220);

	if(button.state == PauseButtonState::HOVER)
	{
		textColor = RGB(255,0,0);
	}

	SetTextColor(hdc,textColor);
	SetBkMode(hdc,TRANSPARENT);

	// ��ư �ؽ�Ʈ�� ��Ʈ
	HFONT hFont = CreateFont(40,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,
							 DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
							 DEFAULT_QUALITY,DEFAULT_PITCH | FF_DONTCARE,TEXT("Chainsaw Carnage"));
	HFONT oldFont = (HFONT)SelectObject(hdc,hFont);


	DrawText(hdc,button.text,-1,&button.rect,DT_CENTER | DT_VCENTER | DT_SINGLELINE);


	if(button.state == PauseButtonState::HOVER || button.state == PauseButtonState::CLICKED) {
		SIZE textSize;
		GetTextExtentPoint32(hdc,button.text,wcslen(button.text),&textSize);

		int lineX1 = button.rect.left + (button.rect.right - button.rect.left - textSize.cx) / 2;
		int lineX2 = lineX1 + textSize.cx;
		int lineY = button.rect.bottom - 5;  

		HPEN underlinePen = CreatePen(PS_SOLID,2,textColor);
		HPEN oldPen = (HPEN)SelectObject(hdc,underlinePen);

		MoveToEx(hdc,lineX1,lineY,NULL);
		LineTo(hdc,lineX2,lineY);

		SelectObject(hdc,oldPen);
		DeleteObject(underlinePen);
	}

	SelectObject(hdc,oldFont);
	DeleteObject(hFont);
}

void MainGameScene::InitButtons()
{
	RECT rc;
	GetClientRect(g_hWnd,&rc);

	// �޴� ��� ����
	int menuWidth = 400;
	int menuHeight = 300;
	int menuX = (rc.right - menuWidth) / 2;
	int menuY = (rc.bottom - menuHeight) / 2;

	int buttonWidth = 300; 
	int buttonHeight = 40;
	int buttonSpacing = 50;

	// ��ư�� ���� Y ��ġ
	int startY = menuY + 90;

	buttons.resize(3);

	// ��ư �߾� ����
	int buttonX = menuX + (menuWidth - buttonWidth) / 2;

	buttons[0].Init(
		buttonX,
		startY,
		buttonWidth,
		buttonHeight,
		PauseButtonType::STARTSCREEN,
		L"REPLAY"
	);


	buttons[1].Init(
		buttonX,
		startY + buttonHeight + buttonSpacing,
		buttonWidth,
		buttonHeight,
		PauseButtonType::MAP_EDITOR,
		L"Map Editer"
	);


	buttons[2].Init(
		buttonX,
		startY + (buttonHeight + buttonSpacing) * 2,
		buttonWidth,
		buttonHeight,
		PauseButtonType::EXIT,
		L"EXIT GAME"
	);
}

void MainGameScene::CheckButtonHover()
{
	for(auto& button : buttons)
	{
		if(PtInRect(&button.rect,mousePos))
		{
			if(button.state!=PauseButtonState::CLICKED)
			{
				button.state=PauseButtonState::HOVER;
			}
		} else
		{
			if(button.state!=PauseButtonState::CLICKED)
			{
				button.state=PauseButtonState::NORMAL;
			}
		}
	}
}

void MainGameScene::HandleButtonClick(PauseButton & button)
{
	button.state = PauseButtonState::CLICKED;

	switch(button.type)
	{
	case PauseButtonType::STARTSCREEN:
		SceneManager::GetInstance()->ChangeScene("GameStartScene");
		break;

	case PauseButtonType::MAP_EDITOR:
		SceneManager::GetInstance()->ChangeScene("MapEditorScene");
		break;
	case PauseButtonType::EXIT:
		DestroyWindow(g_hWnd);
		break;
	}
}
