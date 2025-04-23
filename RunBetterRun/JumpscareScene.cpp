#include "JumpscareScene.h"
#include "VideoManager.h"

HRESULT JumpscareScene::Init()
{
	// 비디오 재생 시작
	if(!VideoManager::PlayVideo(L"SceneAnimation/jumpscare.mp4",2.0f)) {
		// 실패 시 바로 다음 씬으로
		SceneManager::GetInstance()->ChangeScene("LossLifeScene");
	}

	return S_OK;
}

void JumpscareScene::Release()
{
}

void JumpscareScene::Update()
{
	VideoManager::GetInstance()->Update();

	// 비디오 종료 확인
	if(VideoManager::IsFinished()) {
		SceneManager::GetInstance()->ChangeScene("LossLifeScene");
	}
}

void JumpscareScene::Render(HDC hdc)
{
	RECT rc;
	GetClientRect(g_hWnd,&rc);
	HBRUSH blackBrush = CreateSolidBrush(RGB(0,0,0));
	FillRect(hdc,&rc,blackBrush);
	DeleteObject(blackBrush);
}
