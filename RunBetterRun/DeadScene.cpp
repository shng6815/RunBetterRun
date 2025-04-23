#include "DeadScene.h"
#include "ImageManager.h"
#include "SceneManager.h"
#include "TimerManager.h"
#include "Image.h";
#include "Player.h"
#include "VideoManager.h"
#include <random>
#include <mfapi.h>
#include <ctime>

#define DEAD_IMAGE_COUNT 6

HRESULT DeadScene::Init()
{
	// 현재 시간 기반 시드 생성
	static std::random_device rd;
	static std::mt19937 gen(rd());

	std::uniform_int_distribution<> dist(1,DEAD_IMAGE_COUNT);
	int randomImage = dist(gen);

	char keyBuffer[100];
	sprintf_s(keyBuffer,"DeadImage%d",randomImage);
	string imageKey = keyBuffer;

	wchar_t imagePath[100];
	swprintf_s(imagePath,L"Image/DeadMent%d.bmp",randomImage);

	bg = ImageManager::GetInstance()->AddImage(imageKey,
	imagePath,WINSIZE_X,WINSIZE_Y);

	displayDeadTime = 0.0f;
	minDeadTime = 3.0f;

	fadeAlpha = 255;
	fadeState = FADESTATE::FADE_IN;

	return S_OK;
}

void DeadScene::Release()
{
	if(bg)
		bg->Release();
}

void DeadScene::Update()
{
	if(fadeState == FADESTATE::FADE_IN)
	{
		fadeAlpha -=200.0f * TimerManager::GetInstance()->GetDeltaTime();
		if(fadeAlpha < 0)
		{
			fadeAlpha = 0;
			fadeState = FADESTATE::DISPLAY;
		}
	}

	else if(fadeState == FADESTATE::DISPLAY)
	{
		displayDeadTime += TimerManager::GetInstance()->GetDeltaTime();

		if(displayDeadTime > minDeadTime)
		{
			fadeState = FADESTATE::FADE_OUT;
		}
	}

	else if(fadeState == FADESTATE::FADE_OUT)
	{
		fadeAlpha +=200.0f * TimerManager::GetInstance()->GetDeltaTime();

		if(fadeAlpha > 255)
		{
			fadeAlpha = 255;

			Player::GetInstance()->InitPlayerLife();
			SceneManager::GetInstance()->ChangeScene("GameStartScene", "LoadingScene");
		}
	}
}


void DeadScene::Render(HDC hdc)
{
	HDC memDC = bg->GetMemDC();

	BLENDFUNCTION bf; // 알파 블렌딩 구조체
	bf.BlendOp = AC_SRC_OVER; // 블렌딩 연산 방식, 원본 파일 블렌딩
	bf.BlendFlags = 0;

	if(fadeState == FADESTATE::FADE_IN)
	{
		bf.SourceConstantAlpha = (BYTE)(255 - fadeAlpha);
	} 
	else // DISPLAY
	{
		bf.SourceConstantAlpha = 255; // 완전 불투명
	}

	bf.AlphaFormat = 0;

	AlphaBlend(hdc,0,0,WINSIZE_X,WINSIZE_Y,
			   memDC,0,0,WINSIZE_X,WINSIZE_Y,bf);

}
