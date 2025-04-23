#include "GameStartScene.h"
#include "SceneManager.h"
#include "Player.h"
#include "MapManager.h"
#include "Image.h"

HRESULT GameStartScene::Init()
{
	image = ImageManager::GetInstance()->
		AddImage("게임시작",L"Image/eleveator_scene.bmp",1920,1080,1,2);

	if(image == nullptr)
	{
		return E_FAIL;
	}

	titleText = L"Escape from the \napartment";
	InitButtons();
	MapManager::GetInstance()->Init(L"Map/SavedMap.dat");
	//MapManager::GetInstance()->Init(L"Map/EditorMap.dat");
	//MapManager::GetInstance()->Init();
	while(ShowCursor(TRUE) < 0);
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

	if(KeyManager::GetInstance()->IsOnceKeyDown(VK_SPACE)) {
		SceneManager::GetInstance()->ChangeScene("MapEditorScene");
	}

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
	if(image) {
		image->Render(hdc);

	} else {
		RECT rc;
		GetClientRect(g_hWnd,&rc);
		FillRect(hdc,&rc,(HBRUSH)GetStockObject(BLACK_BRUSH));
	}

	DrawTitle(hdc);

	for(auto& button : buttons) {
		DrawButton(hdc,button);
	}
}

void GameStartScene::InitButtons()
{

	RECT rc;
	GetClientRect(g_hWnd,&rc);
	int centerX = rc.right / 1.35;
	int startY = rc.bottom / 2;
	int buttonWidth = 250;
	int buttonHeight = 40;
	int buttonSpacing = 50;

	// 버튼 생성
	buttons.resize(3);

	// 게임 시작 버튼
	buttons[0].Init(
		centerX - buttonWidth / 2,
		startY,
		buttonWidth,
		buttonHeight,
		ButtonType::START,
		TEXT("Game Start")
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
		L"EXIT"
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

void GameStartScene::HandleButtonClick(Button & button)
{

	button.state = ButtonState::CLICKED;

	switch(button.type)
	{
	case ButtonType::START:
	    //SceneManager::GetInstance()->ChangeScene("LoadingScene");
		SceneManager::GetInstance()->ChangeScene("MainGameScene");
		break;
	
	case ButtonType::MAP_EDITOR:
		SceneManager::GetInstance()->ChangeScene("MapEditorScene");
		break;
	case ButtonType::EXIT:
		DestroyWindow(g_hWnd);	
		break;
	}
}

void GameStartScene::DrawTitle(HDC hdc)
{
	// 텍스트 색상을 빨간색으로 변경
	SetTextColor(hdc,RGB(180,0,0));  // 진한 빨간색  
	SetBkMode(hdc,TRANSPARENT);

	RECT rc;
	GetClientRect(g_hWnd,&rc);

	HFONT hFont = CreateFont(60,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,
							 DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
							 DEFAULT_QUALITY,DEFAULT_PITCH | FF_DONTCARE,TEXT("Chainsaw Carnage"));

	HFONT oldFont = (HFONT)SelectObject(hdc,hFont);

	int titleWidth = 800;
	int titleHeight = 160;

	RECT titleRect = {
		rc.right - titleWidth,
		rc.bottom / 4,  
		rc.right - 20,
		rc.bottom / 3 + titleHeight
	};

	DrawText(hdc,titleText,-1,&titleRect,DT_CENTER | DT_VCENTER);

	SelectObject(hdc,oldFont);
	DeleteObject(hFont);
}

void GameStartScene::DrawButton(HDC hdc,Button & button)
{

	COLORREF textColor = RGB(220, 220, 220);

	if(button.state == ButtonState::HOVER)
	{
		textColor = RGB(255, 0,0 );
	}
	// 버튼 그리기


	SetTextColor(hdc, textColor);  
	SetBkMode(hdc,TRANSPARENT);          

	// 버튼 텍스트용 폰트
	HFONT hFont = CreateFont(40,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,
							 DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
							 DEFAULT_QUALITY,DEFAULT_PITCH | FF_DONTCARE,TEXT("Chainsaw Carnage"));
	HFONT oldFont = (HFONT)SelectObject(hdc,hFont);


	DrawText(hdc,button.text,-1,&button.rect,DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	// 호버 상태일 때 밑줄 그리기
	if(button.state == ButtonState::HOVER || button.state == ButtonState::CLICKED) {
		SIZE textSize;
		GetTextExtentPoint32(hdc,button.text,wcslen(button.text),&textSize);

		int lineX1 = button.rect.left + (button.rect.right - button.rect.left - textSize.cx) / 2;
		int lineX2 = lineX1 + textSize.cx;
		int lineY = button.rect.bottom - 5;  // 텍스트 아래 위치

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
