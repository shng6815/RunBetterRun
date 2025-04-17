#pragma once
#include "GameObject.h"
#include "structs.h"
#include <vector>

class Monster : public GameObject
{
private:
	
	// 더 추가할거?
	FPOINT position;
	float speed;
	bool isActive;

public:
	virtual HRESULT Init(FPOINT startPos);		// 멤버 변수의 초기화, 메모리 할당
	virtual void Release();		// 메모리 해제
	virtual void Update();		// 프레임 단위로 게임 로직 실행(데이터 계산)
	virtual void Render(HDC hdc);	// 프레임 단위로 출력(이미지, 텍스트 등)

	FPOINT GetPostion() { return position; }
	void SetPosition(FPOINT pos) { this->position = pos; }
	bool GetIsActive() { return isActive; }
	void SetActive(bool active) { active = isActive; }
	float GetSpeed() { return speed; }
};

