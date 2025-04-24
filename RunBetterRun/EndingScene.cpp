#include "EndingScene.h"
#include "SceneManager.h"
#include "TimerManager.h"
#include "VideoManager.h"
#include <mfapi.h>

HRESULT EndingScene::Init()
{
	// 비디오 재생 시작
	if(!VideoManager::PlayVideo(L"SceneAnimation/Ending.mp4",15.00f)) {
		// 실패 시 바로 다음 씬으로
		SceneManager::GetInstance()->ChangeScene("GameStartScene","LoadingScene");
	}

	return S_OK;
}

void EndingScene::Release()
{
}

void EndingScene::Update()
{
	if(KeyManager::GetInstance()->IsOnceKeyDown(VK_LBUTTON))
	{
		VideoManager::GetInstance()->CloseVideoWindow();
		SceneManager::GetInstance()->ChangeScene("GameStartScene","LoadingScene");
	}
}

void EndingScene::Render(HDC hdc)
{
	VideoManager::GetInstance()->Update();

	if(VideoManager::GetInstance()->IsFinished()) {
		SceneManager::GetInstance()->ChangeScene("GameStartScene","LoadingScene");
	}
}
