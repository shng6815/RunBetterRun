#pragma once
#include "Singleton.h"
#include "config.h"

typedef class Player : public Singleton<Player>
{
public:
	virtual HRESULT Init();                   // 멤버 변수의 초기화, 메모리 할당
	virtual void Release();                   // 메모리 해제
	virtual void Update();                    // 프레임 단위로 게임 로직 실행(데이터 계산)
	virtual void Render(HDC hdc);             // 프레임 단위로 출력(이미지, 텍스트 등)
	void KeyInput(void);
	void MouseInput(void);
	void MoveCamera(float deltaTime);
	void RotateCamera(float deltaTime);

	FPOINT GetCameraPos() { return cameraPos; }
	FPOINT GetCameraVerDir() { return cameraVerDir; }
	FPOINT GetPlane() { return plane; }

private:
	FPOINT cameraPos;                         // 카메라 위치
	FPOINT cameraVerDir;                      // 카메라 방향
	FPOINT cameraHorDir;                      // 카메라 수직 방향
	FPOINT plane;                             // 카메라 평면
	FPOINT moveInput;                         // 카메라 이동 방향
	FPOINT rotate;                            // 카메라 회전
	FLOAT moveSpeed;                          // 카메라 이동 속도
	FLOAT rotateSpeed;                        // 카메라 회전 속도
	float fov;                                // 시야각

	bool isShowMouse;
} Camera;


