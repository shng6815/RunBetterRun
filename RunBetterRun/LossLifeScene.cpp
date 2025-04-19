#include "LossLifeScene.h"
#include "ImageManager.h"
#include "SceneManager.h"
#include "TimerManager.h"
#include "Player.h"
#include "MonsterManager.h"
#include "MainGameScene.h"
#include "Image.h";

HRESULT LossLifeScene::Init()
{
	bg = ImageManager::GetInstance()->AddImage("DeadImage",
		L"horrorloading/horror_dead.bmp",WINSIZE_X,WINSIZE_Y);

	delayTime = 0.0f;

	//MainGameScene 
	
	return S_OK;
}

void LossLifeScene::Release()
{
}

void LossLifeScene::Update()
{
	delayTime += TimerManager::GetInstance()->GetDeltaTime();

	if(delayTime >= 3.0f) {
		SceneManager::GetInstance()->ChangeScene("MainGameScene");
	}
}

void LossLifeScene::Render(HDC hdc)
{
	if(bg)
		bg->Render(hdc);
}
