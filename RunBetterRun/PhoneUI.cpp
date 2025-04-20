#include "PhoneUI.h"
#include "MinimapUI.h"
#include "Image.h"

HRESULT PhoneUI::Init(UIType type,FPOINT pos,FPOINT size,INT layer)
{
	this->uiType = type;
	defaultPos = this->pos = pos;
	this->size = size;
	this->layer = layer;

	phoneImage = ImageManager::GetInstance()->AddImage("PhoneUI",
		L"Image/PhoneUI.bmp",size.x,size.y,true,RGB(255,0,255));

	isActive = true;

	const float REL_X = 20.f / 200;
	const float REL_Y = 77.f / 300;
	const float REL_W = (200 - 40) / 200.f;
	const float REL_H = (300 - 120) / 300.f;

	FPOINT minimapPos = {pos.x + size.x * REL_X,pos.y + size.y * REL_Y};
	FPOINT minimapSize = {size.x * REL_W,size.y * REL_H};

	MinimapUI* minimapUI = new MinimapUI();
	minimapUI->Init(type,minimapPos,minimapSize,1);
	uiUnits.push_back(minimapUI);

	toggleTime = 0.5f;
	toggleDelay = 0.f;
	toggleActive = false;
	isSlideIn = false;

	// 기본 화면 밖 위치는 아래쪽으로 설정 (나중에 변경 가능)
	screenOutPos = {defaultPos.x - 50, WINSIZE_Y + size.y};

	return S_OK;
}

void PhoneUI::Release()
{
	for(auto& uiUnit : uiUnits)
	{
		uiUnit->Release();
		delete uiUnit;
		uiUnit = nullptr;
	}
	uiUnits.clear();
}

void PhoneUI::Update()
{
	if(toggleActive)
	{
		float deltaTime = TimerManager::GetInstance()->GetDeltaTime();
		toggleDelay += deltaTime;

		// 시간 비율 계산 (0.0 ~ 1.0)
		float ratio = min(toggleDelay / toggleTime,1.0f);

		// isSlideIn이 true면 UI가 화면 밖에서 안으로 들어오는 상태
		if(isSlideIn)
		{
			// 화면 안으로 들어오는 애니메이션 (Ease-out 효과)
			float smoothRatio = 1.0f - (1.0f - ratio) * (1.0f - ratio);

			// x와 y 좌표를 모두 애니메이션
			pos.x = screenOutPos.x + smoothRatio * (defaultPos.x - screenOutPos.x);
			pos.y = screenOutPos.y + smoothRatio * (defaultPos.y - screenOutPos.y);
		}
		// isSlideIn이 false면 UI가 화면 안에서 밖으로 나가는 상태
		else
		{
			// 화면 밖으로 나가는 애니메이션 (Ease-in 효과)
			float smoothRatio = ratio * ratio;

			// x와 y 좌표를 모두 애니메이션
			pos.x = defaultPos.x + smoothRatio * (screenOutPos.x - defaultPos.x);
			pos.y = defaultPos.y + smoothRatio * (screenOutPos.y - defaultPos.y);
		}

		// 토글 시간이 끝나면 토글 상태 종료
		if(toggleDelay >= toggleTime)
		{
			toggleActive = false;
			toggleDelay = 0.f;

			// UI를 밖으로 내보내는 상태였다면 비활성화
			if(!isSlideIn)
			{
				isActive = false;
				pos = screenOutPos; // 완전히 화면 밖으로
			} else
			{
				pos = defaultPos; // 원래 위치로 정확히 설정
			}
		}

		// MinimapUI 위치 업데이트
		UpdateUIUnitsPosition();
	}
}

void PhoneUI::Render(HDC hdc)
{
	if(!isActive)
		return;

	//휴대폰 UI 배경 그리기
	if(phoneImage)
	{
		phoneImage->Render(hdc,pos.x,pos.y);
	}

	for(auto& uiUnit : uiUnits)
	{
		uiUnit->Render(hdc);
	}
}

void PhoneUI::ToggleActive()
{
	toggleDelay = 0.f;
	toggleActive = true;
	isSlideIn = false;

	// UI가 비활성화 상태에서 활성화 시, UI를 화면 안으로 들여오기
	if(!isActive)
	{
		isSlideIn = true;
		isActive = true;
		pos = screenOutPos;
	}
}

void PhoneUI::UpdateUIUnitsPosition()
{
	if(!uiUnits.empty())
	{
		const float REL_X = 20.f / 200;
		const float REL_Y = 77.f / 300;

		for(auto& uiUnit : uiUnits)
		{
			MinimapUI* minimapUI = dynamic_cast<MinimapUI*>(uiUnit);
			if(minimapUI)
			{
				FPOINT minimapPos = {pos.x + size.x * REL_X,pos.y + size.y * REL_Y};
				minimapUI->SetPosition(minimapPos);
			}
		}
	}
}