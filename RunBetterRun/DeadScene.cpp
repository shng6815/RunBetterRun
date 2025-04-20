#include "DeadScene.h"
#include "ImageManager.h"
#include "SceneManager.h"
#include "TimerManager.h"
#include "Image.h";
#include "Player.h"

HRESULT DeadScene::Init()
{
	bg = ImageManager::GetInstance()->AddImage("DeadImage",
	L"horrorloading/horror_loading2.bmp",WINSIZE_X,WINSIZE_Y);

	delayTime = 0.0f;

	return S_OK;
}

void DeadScene::Release()
{
}

void DeadScene::Update()
{
	delayTime += TimerManager::GetInstance()->GetDeltaTime();

	if(delayTime >= 3.0f) {
		Player::GetInstance()->InitPlayerLife();

		SceneManager::GetInstance()->ChangeScene("GameStartScene");
	}
}

void DeadScene::Render(HDC hdc)
{
	if(bg)
		bg->Render(hdc);
}
