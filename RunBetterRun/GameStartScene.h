#pragma once

#include "GameObject.h"

// 버튼 종류 정의
enum class ButtonType {
	START,
	MAP_EDITOR,
	EXIT
};

// 버튼 상태 정의
enum class ButtonState {
	NORMAL,
	HOVER,
	CLICKED
};

// 버튼 구조체
struct Button {
	RECT rect;             // 버튼 영역
	ButtonType type;       // 버튼 종류
	ButtonState state;     // 현재 상태
	LPCWSTR text;          // 버튼 텍스트

	// 버튼 초기화
	void Init(int x,int y,int width,int height,ButtonType buttonType,LPCWSTR buttonText) {
		rect = {x,y,x + width,y + height};
		type = buttonType;
		state = ButtonState::NORMAL;
		text = buttonText;
	}
};

class GameStartScene : public GameObject
{
private:
	Image* image;
	HBITMAP backgroundBitmap;  // 배경 이미지
	vector<Button> buttons;    // 버튼 배열
	LPCWSTR titleText;         // 제목 텍스트
	POINT mousePos;            // 마우스 위치

public:
	virtual HRESULT Init() override;
	virtual void Release() override;
	virtual void Update() override;
	virtual void Render(HDC hdc) override;

private:
	void InitButtons();                    // 버튼 초기화
	void CheckButtonHover();               // 마우스 오버 체크
	void HandleButtonClick(Button& button); // 버튼 클릭 처리
	void DrawTitle(HDC hdc);               // 제목 그리기
	void DrawButton(HDC hdc, Button& button); // 버튼 그리기
};

