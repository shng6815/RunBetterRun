#pragma once
#include "GameObject.h"

// 버튼 종류 정의
enum class PauseButtonType {
	STARTSCREEN,
	MAP_EDITOR,
	EXIT
};

// 버튼 상태 정의
enum class PauseButtonState {
	NORMAL,
	HOVER,
	CLICKED
};

// 버튼 구조체
struct PauseButton {
	RECT rect;             // 버튼 영역
	PauseButtonType type;       // 버튼 종류
	PauseButtonState state;     // 현재 상태
	LPCWSTR text;          // 버튼 텍스트

	// 버튼 초기화
	void Init(int x,int y,int width,int height,PauseButtonType buttonType,LPCWSTR buttonText) {
		rect = {x,y,x + width,y + height};
		type = buttonType;
		state = PauseButtonState::NORMAL;
		text = buttonText;
	}
};

class RayCast;
class MainGameScene : public GameObject
{
private:
    struct ShakeInfo
    {
        float maxPower = 0.0f;
        float duration = 0.0f;
        float elapsed = 0.0f;
        float offsetX = 0.0f;
        float offsetY = 0.0f;
        bool isStep = false;

        void Start(float power, float time, bool step)
        {
            maxPower = power;
            duration = time;
            elapsed = 0.0f;
            isStep = step;

            if (!isStep)
            {
                offsetX = ((rand() % 3) - 1) * power;
                offsetY = ((rand() % 3) - 1) * power;
            }
        }

        void Update(float dt)
        {
            if (duration <= 0.0f) return;

            elapsed += dt;

            if (elapsed >= duration)
            {
                offsetX = 0.0f;
                offsetY = 0.0f;
                duration = 0.0f;
                elapsed = 0.0f;
            }
            else
            {
                float progress = elapsed / duration;
                float damping = 1.0f - progress;

                if (isStep)
                {
                    float half = 0.5f;
                    float ratio = (progress < half) ? (progress / half) : ((1.0f - progress) / half);
                    offsetY = maxPower * ratio;
                    offsetX = 0.0f;
                }
                else
                {
                    offsetX = ((rand() % 3) - 1) * damping * maxPower;
                    offsetY = ((rand() % 3) - 1) * damping * maxPower;
                }
            }
        }

        POINT GetOffset() const
        {
            return POINT{ static_cast<int>(offsetX), static_cast<int>(offsetY) };
        }

        bool IsShaking() const { return duration > 0.0f; }
    };
public:
	virtual HRESULT Init() override;
	virtual HRESULT Init(LPCWCH path);
	virtual void Release() override;
	virtual void Update() override;
	virtual void Render(HDC hdc) override;

	void ShakeScreen(float shakePower, float time, bool isStepShake);
	void ApplyShake(HDC hdc);
	void RenderPauseOverlay(HDC hdc);
	void RenderPauseMenu(HDC hdc, PauseButton& button);

	// 일시 정지 메뉴 메소드
	void InitButtons();                    // 버튼 초기화
	void CheckButtonHover();               // 마우스 오버 체크
	void HandleButtonClick(PauseButton& button); // 버튼 클릭 처리

	void StartMonsterCatchAnimation(FPOINT monsterPos);
	void UpdateMonsterCatchAnimation();
	void ShowPhoneGuide();

	void SetInGameStatus();
private:
	enum class SceneStatus { IN_GAME, PAUSE, QUIT, MONSTER_CATCH, PHONE_GUIDE };

	SceneStatus status;
	RayCast* rayCasting;

	HDC backBufferDC = nullptr;
	HBITMAP backBufferBitmap = nullptr;
	HBITMAP oldBitmap = nullptr;

    ShakeInfo screenShake;

	// 일시정지 메뉴
	vector<PauseButton> buttons;   
	POINT mousePos;

	bool isCaught = false;
    float catchRotationTime = 0.0f;
    float catchRotationDuration = 1.0f; // 회전에 걸리는 시간(초)
    FPOINT originalDirection;
    FPOINT targetDirection;
};