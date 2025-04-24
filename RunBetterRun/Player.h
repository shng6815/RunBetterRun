#pragma once
#include "Singleton.h"
#include "config.h"
#include <functional>

typedef class Player : public Singleton<Player>
{
public:
	virtual HRESULT Init(function<void(float, float, bool)> shakeFunc);                   // 멤버 변수의 초기화, 메모리 할당
	virtual void Release();                   // 메모리 해제
	virtual void Update();                    // 프레임 단위로 게임 로직 실행(데이터 계산)
	virtual void Render(HDC hdc);             // 프레임 단위로 출력(이미지, 텍스트 등)

	void KeyInput(void);
	void MouseInput(void);
	void MoveCamera(float deltaTime);
	void Move(FPOINT pos);
	void RotateCamera(float deltaTime);
	void UpdateFOV();

	FPOINT GetCameraPos() { return cameraPos; }
	FPOINT GetCameraVerDir() { return cameraVerDir; }
	FPOINT GetPlane() { return plane; }
	void InitPlayerLife() { playerLife = 3; }
	int GetPlayerLife(){ return playerLife; }
	void LossPlayerLife();

	void SetCameraDirection(FPOINT newDirection) {
		cameraVerDir = newDirection;

		// 수직 방향도 업데이트
		cameraHorDir.x = cameraVerDir.y;
		cameraHorDir.y = -cameraVerDir.x;

		// FOV 업데이트
		UpdateFOV();
	}

private:
	FPOINT cameraPos;                         // 카메라 위치
	FPOINT cameraVerDir;                      // 카메라 방향
	FPOINT cameraHorDir;                      // 카메라 수직 방향
	FPOINT plane;                             // 카메라 평면

	FPOINT moveInput;                         // 카메라 이동 방향
	FLOAT moveSpeed;                          // 카메라 이동 속도
	FLOAT stepElapsedTime;                   // 카메라 이동 시간
	FLOAT stepTime;                           // 카메라 이동 시간
	FLOAT runTime;                           // 카메라 이동 시간

	FLOAT runSpeed;
	FLOAT defaultSpeed;

	FPOINT rotate;                            // 카메라 회전
	FLOAT rotateSpeed;                        // 카메라 회전 속도

	float fov;                                // 시야각
	float targetFOV;
	float fovLerpSpeed = 5.0f;

	// 카메라 흔들기
	function<void(float, float, bool)> shakeFunc;
	void Save();
	void Load();

	// 플레이어 목숨
	int playerLife;
} Camera;


