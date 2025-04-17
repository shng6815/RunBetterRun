#pragma once
#include "GameObject.h"

class RayCast;
class MainGameScene : public GameObject
{
public:
	virtual HRESULT Init() override;
	virtual HRESULT Init(LPCWCH path);
	virtual void Release() override;
	virtual void Update() override;
	virtual void Render(HDC hdc) override;

	void ShakeScreen(float shakePower, float time, bool isStepShake);
	void AddShake(HDC hdc);
private:
	enum class SceneStatus { IN_GAME, PAUSE, QUIT };

	SceneStatus status;
	RayCast* rayCasting;
	void LoadFile(LPCWCH path);

	FLOAT shakeX;
	FLOAT shakeY;
	FLOAT shakeTime;
	FLOAT elapsedTime;
	FLOAT maxShakePower;
	bool isStepShake = false;

	HDC backBufferDC = nullptr;
	HBITMAP backBufferBitmap = nullptr;
	HBITMAP oldBitmap = nullptr;
};

