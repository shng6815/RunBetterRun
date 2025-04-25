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
#include "PhoneUI.h"
#include "Stun.h"
#include "Insight.h"
#include "TutorialUI.h"

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

	BOOL phone = FALSE;
	const auto& items = DataManager::GetInstance()->GetItems();
	for(const auto& itemData : items)
	{
		switch(itemData.id)
		{
		case(0):
		ItemManager::GetInstance()->PutItem(new Key(itemData.pos));
		break;
		case(1):
		ItemManager::GetInstance()->PutItem(new Phone(itemData.pos));
		phone = TRUE;
		break;
		case(2):
		ItemManager::GetInstance()->PutItem(new Insight(itemData.pos));
		break;
		case(3):
		ItemManager::GetInstance()->PutItem(new Stun(itemData.pos));
		break;
		case(4): case(5): case(6): case(7): case(8):
		ItemManager::GetInstance()->PutItem(new Display(itemData.pos,itemData.id));
		break;
		}
		
	}
	const auto& monsters = DataManager::GetInstance()->GetMonsters();
	for(const auto& monsterData : monsters)
	{
		switch(monsterData.id)
		{
		case(100):
			MonsterManager::GetInstance()->PutMonster(new Tentacle(monsterData.pos));
			break;
		default:
			break;
		}
	}
	const auto& obstacles = DataManager::GetInstance()->GetObstacles();
	for(const auto& obstacleData : obstacles)
	{ 
		switch(obstacleData.id)
		{
		case(1001):
			ObstacleManager::GetInstance()->PutObstacle(new Pile(obstacleData.pos,obstacleData.dir));
			break;
		case(1000): case(1002):
			ObstacleManager::GetInstance()->PutObstacle(new Elevator(obstacleData.pos,obstacleData.dir,obstacleData.id));
			break;
		default:
			break;
		}
	}
	if(!phone)
	{
		PhoneUI* uiUnit = new PhoneUI();
		uiUnit->Init(UIType::PLAYING,FPOINT{100,WINSIZE_Y - 500},FPOINT{300,400},0);
		UIManager::GetInstance()->AddUIUnit("PhoneUI",uiUnit);
	}

	UIManager::GetInstance()->Init();
	UIManager::GetInstance()->ChangeUIType(UIType::PLAYING);

	InitButtons();

	status = SceneStatus::IN_GAME;

	RECT rc;
	GetClientRect(g_hWnd, &rc);

	HDC screenDC = GetDC(g_hWnd);
	backBufferDC = CreateCompatibleDC(screenDC);
	backBufferBitmap = CreateCompatibleBitmap(screenDC, rc.right, rc.bottom);
	oldBitmap = (HBITMAP)SelectObject(backBufferDC, backBufferBitmap);
	ReleaseDC(g_hWnd, screenDC);

	SoundManager::GetInstance()->LoadMusic("GameSceneBGM","Sounds/BGM_InGame2.wav");

	SoundManager::GetInstance()->PlayMusic("GameSceneBGM",true,0.3f);

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
	SoundManager::GetInstance()->StopAllSounds();

	SelectObject(backBufferDC, oldBitmap);
	DeleteObject(backBufferBitmap);
	DeleteDC(backBufferDC);
}

void MainGameScene::Update()
{
	// Get mouse position
	POINT cursor;
	GetCursorPos(&cursor);
	ScreenToClient(g_hWnd,&cursor);
	mousePos = cursor;

	switch(status)
	{
	case MainGameScene::SceneStatus::IN_GAME:
		if(ShowCursor(FALSE) >= 0)
			ShowCursor(FALSE);

		if(KeyManager::GetInstance()->IsOnceKeyDown(VK_ESCAPE))
		{
			ShowCursor(TRUE);
			MonsterManager::GetInstance()->StopSound();
			status = SceneStatus::PAUSE;
			break;
		}

		if(KeyManager::GetInstance()->IsOnceKeyDown('M'))
		{
			UIManager::GetInstance()->ToggleActiveUIUnit("PhoneUI");
		}

		Player::GetInstance()->Update();
		if(rayCasting)
			rayCasting->Update();
		SpriteManager::GetInstance()->SortSpritesByDistance();
		MonsterManager::GetInstance()->Update();
		ItemManager::GetInstance()->Update();
		ObstacleManager::GetInstance()->Update();
		break;

	case MainGameScene::SceneStatus::PAUSE:
		if(ShowCursor(TRUE) < 0)
			ShowCursor(TRUE);

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

		if(KeyManager::GetInstance()->IsOnceKeyDown(VK_ESCAPE))
		{
			ShowCursor(FALSE);
			status = SceneStatus::IN_GAME;
		}

		if(KeyManager::GetInstance()->IsOnceKeyDown('T'))
		{
			SceneManager::GetInstance()->ChangeScene("MapEditorScene");
		}
		break;

	case MainGameScene::SceneStatus::QUIT:
		break;

	case SceneStatus::MONSTER_CATCH:
		// 몬스터에게 잡혔을 때의 회전 애니메이션 처리
		UpdateMonsterCatchAnimation();
		break;

	case SceneStatus::PHONE_GUIDE:
		// 폰 가이드 표시 및 입력 대기
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
		textColor = RGB(141,0,0);
	}

	SetTextColor(hdc,textColor);
	SetBkMode(hdc,TRANSPARENT);

	HFONT hFont = CreateFont(40,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,
							 DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
							 DEFAULT_QUALITY,DEFAULT_PITCH | FF_DONTCARE,TEXT("Chainsaw Carnage"));
	HFONT oldFont = (HFONT)SelectObject(hdc,hFont);


	DrawText(hdc,button.text,-1,&button.rect,DT_CENTER | DT_VCENTER | DT_SINGLELINE);


	/*if(button.state == PauseButtonState::HOVER || button.state == PauseButtonState::CLICKED) {
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
	}*/

	SelectObject(hdc,oldFont);
	DeleteObject(hFont);
}

void MainGameScene::InitButtons()
{
	RECT rc;
	GetClientRect(g_hWnd,&rc);

	int menuWidth = 400;
	int menuHeight = 300;
	int menuX = (rc.right - menuWidth) / 2;
	int menuY = (rc.bottom - menuHeight) / 2;

	int buttonWidth = 300; 
	int buttonHeight = 40;
	int buttonSpacing = 50;

	int startY = menuY + 90;

	buttons.resize(3);

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
	    status = MainGameScene::SceneStatus::IN_GAME;
		button.state = PauseButtonState::NORMAL;
		break;

	case PauseButtonType::MAP_EDITOR:
		SceneManager::GetInstance()->ChangeScene("MapEditorScene");
		break;
	case PauseButtonType::EXIT:
	    SceneManager::GetInstance()->ChangeScene("GameStartScene");
		break;
	}
}

void MainGameScene::StartMonsterCatchAnimation(FPOINT monsterPos) {
	// 현재 게임 상태 저장
	status = SceneStatus::MONSTER_CATCH;
	isCaught = true;
	catchRotationTime = 0.0f;

	// 현재 방향 저장
	originalDirection = Player::GetInstance()->GetCameraVerDir();

	// 몬스터 방향으로의 방향 계산
	FPOINT playerPos = Player::GetInstance()->GetCameraPos();
	targetDirection = {
		monsterPos.x - playerPos.x,
		monsterPos.y - playerPos.y
	};

	// 방향 벡터 정규화
	float length = sqrt(targetDirection.x * targetDirection.x + targetDirection.y * targetDirection.y);
	if(length > 0.0001f) {
		targetDirection.x /= length;
		targetDirection.y /= length;
	}
}

void MainGameScene::UpdateMonsterCatchAnimation() {
	float deltaTime = TimerManager::GetInstance()->GetDeltaTime();
	catchRotationTime += deltaTime;

	// 보간 계수 (0.0 ~ 1.0)
	float t = min(catchRotationTime / catchRotationDuration,1.0f);

	// ease-in-out 보간 적용
	t = t < 0.5f ? 2.0f * t * t : 1.0f - pow(-2.0f * t + 2.0f,2.0f) / 2.0f;

	// 방향 선형 보간
	FPOINT newDir = {
		originalDirection.x + t * (targetDirection.x - originalDirection.x),
		originalDirection.y + t * (targetDirection.y - originalDirection.y)
	};

	// 방향 벡터 정규화
	float length = sqrt(newDir.x * newDir.x + newDir.y * newDir.y);
	if(length > 0.0001f) {
		newDir.x /= length;
		newDir.y /= length;
	}

	// 플레이어의 방향 직접 변경 (Player 클래스에 SetCameraDirection 메소드 필요)
	Player::GetInstance()->SetCameraDirection(newDir);

	// 애니메이션 완료 시
	if(catchRotationTime >= catchRotationDuration) {
		isCaught = false;
		// JumpscareScene으로 전환
		SceneManager::GetInstance()->ChangeScene("JumpscareScene");
	}
}

void MainGameScene::ShowPhoneGuide()
{
	UIManager::GetInstance()->ChangeUIType(UIType::PAUSE);
	status = SceneStatus::PHONE_GUIDE;

	TutorialUI* tutorialUI = new TutorialUI();
	tutorialUI->Init(UIType::PAUSE,{WINSIZE_X/2,WINSIZE_Y/2},{676,900},0);
	UIManager::GetInstance()->AddUIUnit("PhoneGuide", tutorialUI);
}

void MainGameScene::SetInGameStatus()
{
	status = SceneStatus::IN_GAME;
	UIManager::GetInstance()->ChangeUIType(UIType::PLAYING);
}
