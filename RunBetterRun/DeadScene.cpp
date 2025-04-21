#include "DeadScene.h"
#include "ImageManager.h"
#include "SceneManager.h"
#include "TimerManager.h"
#include "Image.h";
#include "Player.h"
#include "VideoManager.h"
#include <mfapi.h>


HRESULT DeadScene::Init()
{
	bg = ImageManager::GetInstance()->AddImage("DeadImage",
	L"horrorloading/horror_loading2.bmp",WINSIZE_X,WINSIZE_Y);

	// 비디오 재생 시작
	if(!VideoManager::PlayVideo(L"SceneAnimation/Dead.mp4",10.0f)) {
		// 실패 시 바로 다음 씬으로
		Player::GetInstance()->InitPlayerLife();
		SceneManager::GetInstance()->ChangeScene("GameStartScene");
	}

	return S_OK;
}

void DeadScene::Release()
{
	
}

void DeadScene::Update()
{
	VideoManager::GetInstance()->Update();

	// 비디오 종료 확인
	if(VideoManager::IsFinished()) {
		Player::GetInstance()->InitPlayerLife();
		SceneManager::GetInstance()->ChangeScene("GameStartScene","LoadingScene");
	}
}


void DeadScene::Render(HDC hdc)
{

}
