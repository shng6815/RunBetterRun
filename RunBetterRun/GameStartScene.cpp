#include "GameStartScene.h"
#include "SceneManager.h"

HRESULT GameStartScene::Init()
{
	titleText = TEXT("HorrorGame");
	InitButtons();
	ShowCursor(true);

	return S_OK;
}

void GameStartScene::Release()
{
	if(backgroundBitmap)
		DeleteObject(backgroundBitmap);
}

void GameStartScene::Update()
{
	POINT cursor;
	GetCursorPos(&cursor);
	ScreenToClient(g_hWnd, &cursor);
	mousePos = cursor;

	CheckButtonHover();

	if(KeyManager::GetInstance()->IsOnceKeyDown(VK_LBUTTON))
	{
		for(auto& button: buttons)
		{
			if(button.state==ButtonState::HOVER)
			{
				HandleButtonClick(button);
				break;
			}
		}
	}
}

void GameStartScene::Render(HDC hdc)
{
	RECT rc;
	GetClientRect(g_hWnd,&rc);
	FillRect(hdc,&rc,(HBRUSH)GetStockObject(BLACK_BRUSH));

	// 제목 그리기
	DrawTitle(hdc);

	// 버튼 그리기
	for(auto& button : buttons) {
		DrawButton(hdc,button);
	}
}

void GameStartScene::InitButtons()
{
	/*화면 중앙 위치 계산
		3개 버튼 생성 (게임 시작,맵 에디터,게임 종료)
		각 버튼의 위치,크기,종류,텍스트 설정*/
		// 화면 중앙에 버튼 배치

	RECT rc;
	GetClientRect(g_hWnd,&rc);
	int centerX = rc.right / 2;
	int startY = rc.bottom / 2;
	int buttonWidth = 200;
	int buttonHeight = 50;
	int buttonSpacing = 20;

	// 버튼 생성
	buttons.resize(3);

	// 게임 시작 버튼
	buttons[0].Init(
		centerX - buttonWidth / 2,
		startY,
		buttonWidth,
		buttonHeight,
		ButtonType::START,
		TEXT("GameStart")
	);

	// 맵 에디터 버튼
	buttons[1].Init(
		centerX - buttonWidth / 2,
		startY + buttonHeight + buttonSpacing,
		buttonWidth,
		buttonHeight,
		ButtonType::MAP_EDITOR,
		TEXT("Map Editer")
	);

	// 게임 종료 버튼
	buttons[2].Init(
		centerX - buttonWidth / 2,
		startY + (buttonHeight + buttonSpacing) * 2,
		buttonWidth,
		buttonHeight,
		ButtonType::EXIT,
		TEXT("Game Exit")
	);

}

void GameStartScene::CheckButtonHover()
{

	for(auto& button : buttons)
	{
		if(PtInRect(&button.rect,mousePos))
		{
			if(button.state!=ButtonState::CLICKED)
			{
				button.state=ButtonState::HOVER;
			}
			else
			{
				if(button.state!=ButtonState::CLICKED)
				{
				button.state=ButtonState::NORMAL;
				}
			}
		}
	}
}

void GameStartScene::HandleButtonClick(Button & button)
{

	button.state == ButtonState::CLICKED;

	switch(button.type)
	{
	case ButtonType::START:
		SceneManager::GetInstance()->ChangeScene("MainGameScene");
		break;
	
	case ButtonType::MAP_EDITOR:
		SceneManager::GetInstance()->ChangeScene("MapEditer");
		break;
	case ButtonType::EXIT:
		PostQuitMessage(0);
		break;
	}
}

void GameStartScene::DrawTitle(HDC hdc)
{
	SetTextColor(hdc,RGB(255,255,255));  
	SetBkMode(hdc,TRANSPARENT);          

	RECT rc;
	GetClientRect(g_hWnd,&rc);

	// 제목용 큰 폰트 생성
	HFONT hFont = CreateFont(60,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,
							 DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
							 DEFAULT_QUALITY,DEFAULT_PITCH | FF_DONTCARE,TEXT("Arial"));
	HFONT oldFont = (HFONT)SelectObject(hdc,hFont);

	// 제목 위치 계산 (화면 상단에서 1/3 위치)
	RECT titleRect = {
		0,
		rc.bottom / 3 - 60,
		rc.right,
		rc.bottom / 3
	};

	DrawText(hdc,titleText,-1,&titleRect,DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	SelectObject(hdc,oldFont);
	DeleteObject(hFont);
}

void GameStartScene::DrawButton(HDC hdc,Button & button)
{

	COLORREF bgColor;
	switch(button.state) {
	case ButtonState::NORMAL:
		bgColor = RGB(50,50,50); 
		break;

	case ButtonState::HOVER:
		bgColor = RGB(80,80,80);  
		break;

	case ButtonState::CLICKED:
		bgColor = RGB(120,120,120);  
		break;
	}

	// 버튼 그리기
	HBRUSH hBrush = CreateSolidBrush(bgColor);
	HBRUSH oldBrush = (HBRUSH)SelectObject(hdc,hBrush);

	RoundRect(hdc,button.rect.left,button.rect.top,button.rect.right,button.rect.bottom,20,20);

	SetTextColor(hdc,RGB(255,255,255));  
	SetBkMode(hdc,TRANSPARENT);          

	// 버튼 텍스트용 폰트
	HFONT hFont = CreateFont(24,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,
							 DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
							 DEFAULT_QUALITY,DEFAULT_PITCH | FF_DONTCARE,TEXT("Arial"));
	HFONT oldFont = (HFONT)SelectObject(hdc,hFont);


	DrawText(hdc,button.text,-1,&button.rect,DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	// 리소스 정리
	SelectObject(hdc,oldFont);
	DeleteObject(hFont);
	SelectObject(hdc,oldBrush);
	DeleteObject(hBrush);
}
