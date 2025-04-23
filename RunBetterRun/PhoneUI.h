#pragma once
#include "config.h"
#include "UIUnit.h"
#include <random>

// Define player states
enum class PlayerState
{
	IDLE,
	WALKING,
	RUNNING
};

class PhoneUI: public UIUnit
{
public:
	HRESULT Init(UIType type,FPOINT pos,FPOINT size,INT layer) override;
	void Release() override;
	void Update() override;
	void Render(HDC hdc) override;
	virtual void ToggleActive() override;
	void ToggleUdate();

	// 상태 변경 함수
	void UpdateByPlayerState(PlayerState state);

	// 아이템 획득 흔들림 효과 발동 함수
	void ShakeOnItemGet(float intensity = 1.0f,float duration = 0.5f);

	// 인사이트 아이템 획득시 몬스터 위치 표시
	void GetInsight(FLOAT inSightTime);

private:
	Image* phoneImage;
	float toggleTime;
	float toggleDelay;
	bool toggleActive;
	bool isSlideIn;
	FPOINT defaultPos;
	FPOINT defaultSize;
	FPOINT screenOutPos;
	FPOINT runningPos;
	FPOINT runningSize;
	FPOINT walkingPos;
	FPOINT walkingSize;

	// 현재 플레이어 상태
	PlayerState currentState;

	// 상태 변화 애니메이션 관련 변수
	bool isStateChanging;       // 상태 변화 중인지 여부
	float stateChangeTime;      // 상태 변화 애니메이션 시간
	float stateChangeDelay;     // 상태 변화 애니메이션 진행 시간
	FPOINT startPos;            // 애니메이션 시작 위치
	FPOINT targetPos;           // 애니메이션 목표 위치
	FPOINT startSize;           // 애니메이션 시작 크기
	FPOINT targetSize;          // 애니메이션 목표 크기

	// 흔들림 효과 관련 변수
	bool useWobbleEffect;       // 흔들림 효과 사용 여부
	FPOINT basePos;             // 흔들림 기준 위치
	float wobbleTime;           // 흔들림 주기
	float wobbleTimer;          // 흔들림 타이머
	float wobbleAmount;         // 흔들림 정도
	FPOINT wobbleOffset;        // 현재 흔들림 오프셋
	FPOINT targetWobbleOffset;  // 목표 흔들림 오프셋
	float wobbleChangeTime;     // 흔들림 변화 주기
	float wobbleChangeTimer;    // 흔들림 변화 타이머
	std::mt19937 rng;           // 랜덤 생성기

	// 아이템 획득 흔들림 효과 관련 변수
	bool isItemShaking;         // 아이템 흔들림 활성화 여부
	float itemShakeTime;        // 아이템 흔들림 지속 시간
	float itemShakeTimer;       // 아이템 흔들림 진행 시간
	float itemShakeIntensity;   // 아이템 흔들림 강도
	float currentShakeIntensity; // 현재 아이템 흔들림 강도

	// 인사이트 관련 변수
	bool hasInsight;         // 인사이트 활성화 여부
	float insightTime;       // 인사이트 지속 시간
	float insightTimer;      // 현재 인사이트 타이머
	BOOL isInsight;         // 인사이트 활성화 여부

	// 상태 변화 애니메이션 업데이트
	void UpdateStateChangeAnimation();

	// 흔들림 효과 업데이트
	void UpdateWobbleEffect();

	// 아이템 흔들림 효과 업데이트
	void UpdateItemShakeEffect();

	void UpdateInsight();
};