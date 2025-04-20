#include "LoadingScene.h"
#include "Image.h"
#include "TimerManager.h"

HRESULT LoadingScene::Init()
{
	bg = ImageManager::GetInstance()->AddImage("loadingImage",
		L"horrorloading/horror_loading7.bmp", WINSIZE_X, WINSIZE_Y);

	loadingTime = 0.0f;
	minLoadingTime = 3.0f;
	isLoadingComplete = false;

	while(ShowCursor(FALSE)>=0);

	return S_OK;
}

void LoadingScene::Release()
{
}

void LoadingScene::Update()
{
	// 로딩 경과 시간 증가
	loadingTime+=TimerManager::GetInstance()->GetDeltaTime();

	// 로딩 완료 및 최소 시간 경과 확인
	if(isLoadingComplete && loadingTime >= minLoadingTime)
	{
		SceneManager::GetInstance()->LoadingComplete();
	}

}

void LoadingScene::Render(HDC hdc)
{
	if (bg)
		bg->Render(hdc);
}
