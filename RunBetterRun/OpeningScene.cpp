#include "OpeningScene.h"
#include "VideoManager.h"
#include "KeyManager.h"

HRESULT OpeningScene::Init()
{
	// 비디오 재생 시작
	if(!VideoManager::PlayVideo(L"SceneAnimation/Opening_new.mp4",17.00f)) {
		// 실패 시 바로 다음 씬으로
		SceneManager::GetInstance()->ChangeScene("MainGameScene","LoadingScene");
	}

	return S_OK;
}

void OpeningScene::Release()
{
}

void OpeningScene::Update()
{
	if(KeyManager::GetInstance()->IsOnceKeyDown(VK_LBUTTON))
	{
		VideoManager::GetInstance()->CloseVideoWindow();
		SceneManager::GetInstance()->ChangeScene("MainGameScene","LoadingScene");
	}
}

void OpeningScene::Render(HDC hdc)
{
	VideoManager::GetInstance()->Update();

	if(VideoManager::GetInstance()->IsFinished()) {
		SceneManager::GetInstance()->ChangeScene("MainGameScene","LoadingScene");
	}
}
