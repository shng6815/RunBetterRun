#pragma once
#include "GameObject.h"

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
private:
	enum class SceneStatus { IN_GAME, PAUSE, QUIT };

	SceneStatus status;
	RayCast* rayCasting;
	void LoadFile(LPCWCH path);

	HDC backBufferDC = nullptr;
	HBITMAP backBufferBitmap = nullptr;
	HBITMAP oldBitmap = nullptr;

    ShakeInfo screenShake;
};