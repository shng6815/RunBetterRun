#include "LossLifeScene.h"
#include "ImageManager.h"
#include "SceneManager.h"
#include "TimerManager.h"
#include "VideoManager.h"
#include "Image.h";
#include "Player.h"
#include <mfapi.h>

HRESULT LossLifeScene::Init()
{
	bg = ImageManager::GetInstance()->AddImage("LossLifeImage",
	   L"horrorloading/horror_loading.bmp",WINSIZE_X,WINSIZE_Y);

	const wchar_t* videoPath = nullptr;
	float videoLength = 0.0f;

	//switch(playerLife)
	//{
	//case 2:
	//	videoPath = L"SceneAnimation/3Life2_new.mp4";
	//	videoLength = 8.0f;
	//	break;
	//case 1:
	//	videoPath = L"SceneAnimation/2Life1_new.mp4";
	//	videoLength = 7.0f;
	//	break;
	//default:
	//	// 기본 비디오 설정
	//	videoPath = L"SceneAnimation/loss_life.mp4";
	//	videoLength = 7.0f;
	//}

	// 비디오 재생 시작
	if(!VideoManager::PlayVideo(videoPath,videoLength)) {
		// 실패 시 바로 다음 씬으로
		SceneManager::GetInstance()->ChangeScene("MainGameScene");
	}

	return S_OK;
}

void LossLifeScene::Release()
{
	
}

void LossLifeScene::Update()
{
	// 비디오 상태 업데이트
	VideoManager::GetInstance()->Update();

	// 비디오 종료 확인
	if(VideoManager::IsFinished()) {
		SceneManager::GetInstance()->ChangeScene("MainGameScene");
	}
}

void LossLifeScene::Render(HDC hdc)
{
	
}
