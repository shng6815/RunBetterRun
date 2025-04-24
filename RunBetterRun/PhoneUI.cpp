#include "PhoneUI.h"
#include "MinimapUI.h"
#include "NumberUI.h"
#include "Image.h"
#include <ctime>

HRESULT PhoneUI::Init(UIType type,FPOINT pos,FPOINT size,INT layer)
{
	this->uiType = type;
	defaultPos = this->pos = pos;
	defaultSize = this->size = size;
	this->layer = layer;

	runningPos = {defaultPos.x + 150,defaultPos.y + 50.0f};
	runningSize = {defaultSize.x * 0.6f,defaultSize.y * 0.6f};

	walkingPos = {defaultPos.x + 60,defaultPos.y + 30.0f};
	walkingSize = {defaultSize.x * 0.8f,defaultSize.y * 0.8f};

	phoneImage = ImageManager::GetInstance()->AddImage("PhoneUI",
		L"Image/PhoneUI.bmp",size.x,size.y,true,RGB(255,0,255));

	isActive = false;

	const float REL_X = 20.f / 200;
	const float REL_Y = 77.f / 300;
	const float REL_W = (200 - 40) / 200.f;
	const float REL_H = (300 - 120) / 300.f;

	FPOINT minimapPos = {pos.x + size.x * REL_X,pos.y + size.y * REL_Y};
	FPOINT minimapSize = {size.x * REL_W,size.y * REL_H};

	FPOINT relPos = {REL_X,REL_Y};
	FPOINT relSize = {REL_W,REL_H};

	MinimapUI* minimapUI = new MinimapUI();
	minimapUI->Init(type,relPos,relSize,this,1);
	
	// 남은 아이템 수의 UI를 위한 상대적 위치와 크기 설정

	relPos = {0.5f,0.11f};
	relSize = {0.8f,0.1f};
	// 상대적 위치와 크기를 사용하여 UIUnit 초기화
	NumberUI* numberUI = new NumberUI();
	numberUI->Init(type,relPos,relSize,this,1);

	toggleTime = 0.5f;
	toggleDelay = 0.f;
	toggleActive = false;
	isSlideIn = false;

	// 상태 변화 애니메이션 관련 변수 초기화
	isStateChanging = false;
	stateChangeTime = 0.3f;  // 상태 변화 애니메이션 시간 (300ms)
	stateChangeDelay = 0.f;
	startPos = defaultPos;
	targetPos = defaultPos;
	startSize = defaultSize;
	targetSize = defaultSize;

	// 흔들림 효과 관련 변수 초기화
	useWobbleEffect = true;
	basePos = pos;
	wobbleTime = 3.0f;       // 전체 흔들림 주기 (3초)
	wobbleTimer = 0.0f;
	wobbleAmount = 5.0f;     // 최대 흔들림 정도 (5픽셀)
	wobbleOffset = {0.0f,0.0f};
	targetWobbleOffset = {0.0f,0.0f};
	wobbleChangeTime = 1.0f; // 새로운 목표 흔들림 방향 변경 주기 (1초)
	wobbleChangeTimer = 0.0f;

	// 아이템 획득 흔들림 효과 관련 변수 초기화
	isItemShaking = false;
	itemShakeTime = 0.5f;    // 기본 지속 시간 (0.5초)
	itemShakeTimer = 0.0f;
	itemShakeIntensity = 1.0f;  // 기본 강도
	currentShakeIntensity = 0.0f;

	// 랜덤 생성기 초기화
	std::random_device rd;
	rng = std::mt19937(static_cast<unsigned int>(time(nullptr)));

	currentState = PlayerState::IDLE;

	// 기본 화면 밖 위치는 아래쪽으로 설정 (나중에 변경 가능)
	screenOutPos = {defaultPos.x - 100,WINSIZE_Y + size.y};

	hasInsight = false;
	insightTime = 0.0f;
	insightTimer = 0.0f;

	ToggleActive();

	return S_OK;
}

// Update 함수에 아이템 흔들림 효과 업데이트 코드 추가
void PhoneUI::Update()
{
	// 토글 업데이트 (슬라이드 인/아웃)
	ToggleUdate();

	// 상태 변화 애니메이션 업데이트
	if(isStateChanging && !toggleActive)
	{
		UpdateStateChangeAnimation();
	}

	// 아이템 획득 흔들림 효과 업데이트 (최우선)
	if(isItemShaking && isActive)
	{
		UpdateItemShakeEffect();
	}
	// 일반 흔들림 효과 업데이트 (아이템 획득 흔들림이 아닐 때만)
	else if(useWobbleEffect && !toggleActive && !isStateChanging && isActive)
	{
		UpdateWobbleEffect();
	}

	UpdateInsight();

	// UI 유닛 업데이트
	for(auto& uiUnit : uiUnits)
	{
		if(uiUnit)
		{
			uiUnit->Update();
		}
	}
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

void PhoneUI::Render(HDC hdc)
{
	if(!isActive)
		return;

	if(phoneImage)
	{
		// 현재 설정된 size를 사용하여 이미지 크기 조절
		phoneImage->RenderResized(hdc,pos.x,pos.y,size.x,size.y);
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

void PhoneUI::UpdateByPlayerState(PlayerState state)
{
	// 현재 토글 애니메이션 중이거나 동일한 상태로 전환하는 경우에는 무시
	if(toggleActive || state == currentState)
	{
		return;
	}

	// 새로운 상태 저장
	PlayerState prevState = currentState;
	currentState = state;

	// 상태 변화 애니메이션 시작
	isStateChanging = true;
	stateChangeDelay = 0.f;

	// 애니메이션 시작 위치와 크기 설정 (현재 위치와 크기)
	startPos = pos;
	startSize = size;

	// 애니메이션 목표 위치와 크기 설정
	switch(state)
	{
	case PlayerState::IDLE:
	targetPos = defaultPos;
	targetSize = defaultSize;
	break;
	case PlayerState::WALKING:
	targetPos = walkingPos;
	targetSize = walkingSize;
	break;
	case PlayerState::RUNNING:
	targetPos = runningPos;
	targetSize = runningSize;
	break;
	}

	// 흔들림 효과 리셋
	wobbleOffset = {0.0f,0.0f};
	targetWobbleOffset = {0.0f,0.0f};
	wobbleChangeTimer = 0.0f;
}

void PhoneUI::UpdateStateChangeAnimation()
{
	float deltaTime = TimerManager::GetInstance()->GetDeltaTime();
	stateChangeDelay += deltaTime;

	// 시간 비율 계산 (0.0 ~ 1.0)
	float ratio = min(stateChangeDelay / stateChangeTime,1.0f);

	// Ease-in-out 효과 (부드러운 시작과 끝)
	float smoothRatio = ratio < 0.5f ? 2.0f * ratio * ratio : 1.0f - pow(-2.0f * ratio + 2.0f,2.0f) / 2.0f;

	// 위치와 크기 선형 보간
	pos.x = startPos.x + smoothRatio * (targetPos.x - startPos.x);
	pos.y = startPos.y + smoothRatio * (targetPos.y - startPos.y);
	size.x = startSize.x + smoothRatio * (targetSize.x - startSize.x);
	size.y = startSize.y + smoothRatio * (targetSize.y - startSize.y);

	// 애니메이션 종료 체크
	if(stateChangeDelay >= stateChangeTime)
	{
		isStateChanging = false;
		stateChangeDelay = 0.f;

		// 최종 위치와 크기 설정
		pos = targetPos;
		size = targetSize;

		// 흔들림 효과의 기준 위치 업데이트
		basePos = pos;
	}
}

void PhoneUI::UpdateWobbleEffect()
{
	float deltaTime = TimerManager::GetInstance()->GetDeltaTime();
	wobbleTimer += deltaTime;
	wobbleChangeTimer += deltaTime;

	// 새로운 목표 흔들림 방향 생성
	if(wobbleChangeTimer >= wobbleChangeTime)
	{
		wobbleChangeTimer = 0.0f;

		// 랜덤 방향 생성 (-1.0 ~ 1.0)
		std::uniform_real_distribution<float> dist(-1.0f,1.0f);
		targetWobbleOffset.x = dist(rng) * wobbleAmount;
		targetWobbleOffset.y = dist(rng) * wobbleAmount;
	}

	// 현재 흔들림에서 목표 흔들림으로 부드럽게 전환
	float lerpFactor = min(deltaTime * 2.0f,1.0f); // 0.5초 동안 보간
	wobbleOffset.x = wobbleOffset.x + lerpFactor * (targetWobbleOffset.x - wobbleOffset.x);
	wobbleOffset.y = wobbleOffset.y + lerpFactor * (targetWobbleOffset.y - wobbleOffset.y);

	// sin/cos을 이용한 추가적인 부드러운 움직임
	float sinValue = sin(wobbleTimer * 2.0f * 3.14159f / wobbleTime);
	float cosValue = cos(wobbleTimer * 2.0f * 3.14159f / wobbleTime);

	// 최종 위치 계산 (기준 위치 + 흔들림 효과)
	FPOINT newPos;

	// 플레이어 상태에 따라 기준 위치 설정
	switch(currentState)
	{
	case PlayerState::IDLE:
	basePos = defaultPos;
	break;
	case PlayerState::WALKING:
	basePos = walkingPos;
	break;
	case PlayerState::RUNNING:
	basePos = runningPos;
	break;
	}

	// 기본 흔들림 + sin/cos 효과 추가
	newPos.x = basePos.x + wobbleOffset.x * 0.7f + sinValue * wobbleAmount * 0.3f;
	newPos.y = basePos.y + wobbleOffset.y * 0.7f + cosValue * wobbleAmount * 0.3f;

	// 현재 위치 업데이트
	pos = newPos;
}

void PhoneUI::ToggleUdate()
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

			// Get target position based on current player state
			FPOINT targetPosTmp;
			switch(currentState)
			{
			case PlayerState::RUNNING:
			targetPosTmp = runningPos;
			break;
			case PlayerState::WALKING:
			targetPosTmp = walkingPos;
			break;
			default: // IDLE
			targetPosTmp = defaultPos;
			break;
			}

			// x와 y 좌표를 모두 애니메이션
			pos.x = screenOutPos.x + smoothRatio * (targetPosTmp.x - screenOutPos.x);
			pos.y = screenOutPos.y + smoothRatio * (targetPosTmp.y - screenOutPos.y);
		}
		// isSlideIn이 false면 UI가 화면 안에서 밖으로 나가는 상태
		else
		{
			// 화면 밖으로 나가는 애니메이션 (Ease-in 효과)
			float smoothRatio = ratio * ratio;

			// Get current position based on player state
			FPOINT currentPos;
			switch(currentState)
			{
			case PlayerState::RUNNING:
			currentPos = runningPos;
			break;
			case PlayerState::WALKING:
			currentPos = walkingPos;
			break;
			default: // IDLE
			currentPos = defaultPos;
			break;
			}

			// x와 y 좌표를 모두 애니메이션
			pos.x = currentPos.x + smoothRatio * (screenOutPos.x - currentPos.x);
			pos.y = currentPos.y + smoothRatio * (screenOutPos.y - currentPos.y);
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
				// Set position based on player state
				switch(currentState)
				{
				case PlayerState::RUNNING:
				pos = runningPos;
				size = runningSize;
				break;
				case PlayerState::WALKING:
				pos = walkingPos;
				size = walkingSize;
				break;
				default: // IDLE
				pos = defaultPos;
				size = defaultSize;
				break;
				}

				// 흔들림 효과의 기준 위치 업데이트
				basePos = pos;
			}
		}
	}
}

void PhoneUI::ShakeOnItemGet(float intensity,float duration)
{
	// 토글 애니메이션 중에는 무시
	if(toggleActive)
		return;

	isItemShaking = true;
	itemShakeTime = duration;
	itemShakeTimer = 0.0f;
	itemShakeIntensity = intensity;
	currentShakeIntensity = intensity;
}

void PhoneUI::UpdateItemShakeEffect()
{
	float deltaTime = TimerManager::GetInstance()->GetDeltaTime();
	itemShakeTimer += deltaTime;

	// 지속 시간 기반으로 강도 감소 (Ease-out)
	float ratio = itemShakeTimer / itemShakeTime;
	if(ratio > 1.0f) ratio = 1.0f;

	// 강도를 점점 줄임 (빠르게 줄다가 천천히 줄어듦)
	currentShakeIntensity = itemShakeIntensity * (1.0f - ratio * ratio);

	// 랜덤한 방향으로 흔들림
	std::uniform_real_distribution<float> dist(-1.0f,1.0f);
	float shakeOffsetX = dist(rng) * 15.0f * currentShakeIntensity;
	float shakeOffsetY = dist(rng) * 15.0f * currentShakeIntensity;

	// 플레이어 상태에 따라 기준 위치 설정
	switch(currentState)
	{
	case PlayerState::IDLE:
	basePos = defaultPos;
	break;
	case PlayerState::WALKING:
	basePos = walkingPos;
	break;
	case PlayerState::RUNNING:
	basePos = runningPos;
	break;
	}

	// 기준 위치에 흔들림 오프셋 적용
	pos.x = basePos.x + shakeOffsetX;
	pos.y = basePos.y + shakeOffsetY;

	// 흔들림 시간이 끝나면 상태 초기화
	if(itemShakeTimer >= itemShakeTime)
	{
		isItemShaking = false;
		itemShakeTimer = 0.0f;
		currentShakeIntensity = 0.0f;
		pos = basePos; // 원래 위치로 복귀
	}
}

void PhoneUI::GetInsight(float duration) {
	hasInsight = true;
	insightTime = duration;
	insightTimer = 0.0f;

	// MinimapUI를 찾아 몬스터 표시 활성화
	for (auto& uiUnit : uiUnits) {
		MinimapUI* minimapUI = dynamic_cast<MinimapUI*>(uiUnit);
		if (minimapUI) {
			minimapUI->SetShowMonsters(true);
			break;
		}
	}

	// 인사이트 활성화 시각적 피드백
	ShakeOnItemGet(1.5f, 0.7f);
}

void PhoneUI::UpdateInsight() {
	if (hasInsight) {
		float deltaTime = TimerManager::GetInstance()->GetDeltaTime();
		insightTimer += deltaTime;

		if (insightTimer >= insightTime) {
			hasInsight = false;
			insightTimer = 0.0f;

			// MinimapUI를 찾아 몬스터 표시 비활성화
			for (auto& uiUnit : uiUnits) {
				MinimapUI* minimapUI = dynamic_cast<MinimapUI*>(uiUnit);
				if (minimapUI) {
					minimapUI->SetShowMonsters(false);
					break;
				}
			}
		}
	}
}