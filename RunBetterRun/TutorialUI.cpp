#include "TutorialUI.h"
#include "Image.h"
#include "PhoneUI.h"
#include "UIManager.h"
#include "SceneManager.h"
#include "MainGameScene.h"

HRESULT TutorialUI::Init(UIType type,FPOINT pos,FPOINT size,INT layer)
{
	UIUnit::Init(type,pos,size,layer);
	tutorialImage = ImageManager::GetInstance()->AddImage("TutorialImage",
		L"Image/phoneText.bmp",size.x*28,size.y,28,1,true,RGB(255,0,255));

	frameIDX = tutorialImage->GetMaxFrameX();
	isUp = true;
	isDone = false;

	return S_OK;
}

void TutorialUI::Release()
{
}

void TutorialUI::Update()
{
	if(isUp)
	{
		frameIDX--;
		if(frameIDX < 0)
		{
			frameIDX = 0;
			isDone = true;
		}
	}
	else
	{
		frameIDX++;
		if(frameIDX > tutorialImage->GetMaxFrameX())
		{
			dynamic_cast<MainGameScene*>(SceneManager::GetInstance()->currentScene)->SetInGameStatus();

			return;
		}
	}

	if(isDone && KeyManager::GetInstance()->IsOnceKeyDown(VK_SPACE)) {
		// 폰 UI 등록
		PhoneUI* uiUnit = new PhoneUI();
		uiUnit->Init(UIType::PLAYING,FPOINT{100,WINSIZE_Y - 500},FPOINT{300,400},0);
		UIManager::GetInstance()->AddUIUnit("PhoneUI",uiUnit);
		isUp = false;
	}
}

void TutorialUI::Render(HDC hdc)
{
	if(!isActive)
		return;

	if(tutorialImage)
	{
		tutorialImage->RenderResized(hdc,pos.x,pos.y,size.x,size.y,frameIDX,true);

	}
}

void TutorialUI::ToggleActive()
{}
