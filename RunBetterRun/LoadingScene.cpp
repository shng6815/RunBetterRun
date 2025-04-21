#include "LoadingScene.h"
#include "TimerManager.h"
#include "SceneManager.h"
#include <cmath>

HRESULT LoadingScene::Init()
{
	loadingTime = 0.0f;
	minLoadingTime = 0.0f;
	isLoadingComplete = false;

	// 로딩 애니메이션 초기화
	rotationAngle = 0.0f;
	pulseScale = 1.0f;
	pulseDirection = 1.0f;

	return S_OK;
}

void LoadingScene::Release()
{
}

void LoadingScene::Update()
{
	float deltaTime = TimerManager::GetInstance()->GetDeltaTime();

	// 로딩 경과 시간 증가
	loadingTime += deltaTime;

	// 로딩 원 회전 애니메이션 업데이트
	rotationAngle += 180.0f * deltaTime; // 초당 180도 회전
	if(rotationAngle >= 360.0f)
		rotationAngle -= 360.0f;

	// 맥동 효과 업데이트
	pulseScale += pulseDirection * 0.4f * deltaTime;
	if(pulseScale > 1.2f) {
		pulseScale = 1.2f;
		pulseDirection = -1.0f;
	} else if(pulseScale < 0.8f) {
		pulseScale = 0.8f;
		pulseDirection = 1.0f;
	}

	// 로딩 완료 및 최소 시간 경과 확인
	if(isLoadingComplete && loadingTime >= minLoadingTime)
	{
		SceneManager::GetInstance()->LoadingComplete();
	}
}

void LoadingScene::Render(HDC hdc)
{
	// 배경 - 어두운 검붉은 색
	RECT rc;
	GetClientRect(g_hWnd,&rc);

	// 배경 그리기
	HBRUSH bgBrush = CreateSolidBrush(RGB(25,5,5));
	FillRect(hdc,&rc,bgBrush);
	DeleteObject(bgBrush);

	// 혈흔 효과 그리기
	DrawBloodEffect(hdc);

	// 가운데 로딩 원 그리기
	int centerX = rc.right / 2;
	int centerY = rc.bottom / 2;
	DrawLoadingCircle(hdc,centerX,centerY,40);

	// "LOADING" 텍스트를 직접 그리기
	DrawLoadingVisual(hdc,centerX,centerY,70);
}

void LoadingScene::DrawLoadingCircle(HDC hdc,int centerX,int centerY,int radius)
{
	// 회전하는 로딩 원 그리기
	// 반투명 검은 배경 원
	int scaledRadius = (int)(radius * pulseScale);
	HBRUSH circleBgBrush = CreateSolidBrush(RGB(10,0,0));
	HGDIOBJ oldBrush = SelectObject(hdc,circleBgBrush);

	Ellipse(hdc,centerX - scaledRadius,centerY - scaledRadius,
			centerX + scaledRadius,centerY + scaledRadius);

	// 안쪽 회전 부분 (호)
	SelectObject(hdc,GetStockObject(NULL_BRUSH));
	HPEN arcPen = CreatePen(PS_SOLID,3,RGB(180,0,0)); // 진한 붉은색
	HGDIOBJ oldPen = SelectObject(hdc,arcPen);

	// 두 개의 호를 그려 회전 효과 생성
	// 첫 번째 호
	int startAngle = (int)rotationAngle;
	int sweepAngle = 120; // 120도 호

	// 호의 시작점과 끝점 계산
	int x1 = centerX + (int)(scaledRadius * cos(startAngle * 3.14159f / 180.0f));
	int y1 = centerY + (int)(scaledRadius * sin(startAngle * 3.14159f / 180.0f));
	int x2 = centerX + (int)(scaledRadius * cos((startAngle + sweepAngle) * 3.14159f / 180.0f));
	int y2 = centerY + (int)(scaledRadius * sin((startAngle + sweepAngle) * 3.14159f / 180.0f));

	// 첫 번째 호 그리기
	Arc(hdc,centerX - scaledRadius,centerY - scaledRadius,
		centerX + scaledRadius,centerY + scaledRadius,
		x1,y1,x2,y2);

	// 두 번째 호 (반대편)
	startAngle = (int)(rotationAngle + 180);
	x1 = centerX + (int)(scaledRadius * cos(startAngle * 3.14159f / 180.0f));
	y1 = centerY + (int)(scaledRadius * sin(startAngle * 3.14159f / 180.0f));
	x2 = centerX + (int)(scaledRadius * cos((startAngle + sweepAngle) * 3.14159f / 180.0f));
	y2 = centerY + (int)(scaledRadius * sin((startAngle + sweepAngle) * 3.14159f / 180.0f));

	// 두 번째 호 그리기
	Arc(hdc,centerX - scaledRadius,centerY - scaledRadius,
		centerX + scaledRadius,centerY + scaledRadius,
		x1,y1,x2,y2);

	// 자원 해제
	SelectObject(hdc,oldPen);
	SelectObject(hdc,oldBrush);
	DeleteObject(arcPen);
	DeleteObject(circleBgBrush);
}

void LoadingScene::DrawBloodEffect(HDC hdc)
{
	RECT rc;
	GetClientRect(g_hWnd,&rc);

	// 화면 가장자리에 그라데이션 효과
	for(int i = 0; i < 20; i++) {
		int alpha = 255 - i * 12; // 바깥쪽으로 갈수록 투명해짐
		if(alpha < 0) alpha = 0;

		int red = 50 - i * 2;
		if(red < 0) red = 0;

		HPEN pen = CreatePen(PS_SOLID,1,RGB(red,0,0));
		HGDIOBJ oldPen = SelectObject(hdc,pen);

		// 화면 가장자리를 따라 타원 그리기
		int inset = i * 3;
		RoundRect(hdc,rc.left + inset,rc.top + inset,
				 rc.right - inset,rc.bottom - inset,20,20);

		SelectObject(hdc,oldPen);
		DeleteObject(pen);
	}

	// 랜덤 혈흔 얼룩
	srand(12345); // 고정된 시드
	for(int i = 0; i < 10; i++) {
		int x = rand() % rc.right;
		int y = rand() % rc.bottom;
		int size = 20 + rand() % 30;

		HBRUSH bloodBrush = CreateSolidBrush(RGB(80 + rand() % 40,0,0));
		HGDIOBJ oldBrush = SelectObject(hdc,bloodBrush);
		SelectObject(hdc,GetStockObject(NULL_PEN));

		// 불규칙한 혈흔 그리기
		Ellipse(hdc,x - size,y - size,x + size,y + size);

		SelectObject(hdc,oldBrush);
		DeleteObject(bloodBrush);
	}
}

void LoadingScene::DrawLoadingVisual(HDC hdc,int centerX,int centerY,int yOffset)
{
	// "LOADING..." 텍스트를 시각적으로 표시 (직접 그리기)
	int textY = centerY + yOffset;
	int letterSpacing = 25; // 글자 간격 넓게
	int letterHeight = 20;  // 글자 높이
	int letterWidth = 15;   // 글자 너비
	int startX = centerX - (letterSpacing * 3); // "LOADING" 중앙 정렬
	int dotCount = (int)(loadingTime * 2) % 4; // 주기적으로 점 개수 변경 (0-3)

	HPEN textPen = CreatePen(PS_SOLID,3,RGB(180,0,0)); // 진한 붉은색, 더 굵게
	HGDIOBJ oldPen = SelectObject(hdc,textPen);

	// L 그리기
	MoveToEx(hdc,startX,textY - letterHeight,NULL);
	LineTo(hdc,startX,textY + letterHeight);
	LineTo(hdc,startX + letterWidth,textY + letterHeight);

	// O 그리기
	Ellipse(hdc,startX + letterSpacing - letterWidth,textY - letterHeight,
			startX + letterSpacing + letterWidth,textY + letterHeight);

	// A 그리기
	MoveToEx(hdc,startX + letterSpacing*2 - letterWidth,textY + letterHeight,NULL);
	LineTo(hdc,startX + letterSpacing*2,textY - letterHeight);
	LineTo(hdc,startX + letterSpacing*2 + letterWidth,textY + letterHeight);
	// A의 가로선
	MoveToEx(hdc,startX + letterSpacing*2 - letterWidth/2,textY,NULL);
	LineTo(hdc,startX + letterSpacing*2 + letterWidth/2,textY);

	// D 그리기
	MoveToEx(hdc,startX + letterSpacing*3 - letterWidth/2,textY - letterHeight,NULL);
	LineTo(hdc,startX + letterSpacing*3 - letterWidth/2,textY + letterHeight);

	// 반원 그리기 (D의 오른쪽 부분)
	HBRUSH nullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
	HBRUSH oldBrush = (HBRUSH)SelectObject(hdc,nullBrush);

	Ellipse(hdc,
			startX + letterSpacing*3 - letterWidth/2,
			textY - letterHeight,
			startX + letterSpacing*3 + letterWidth + letterWidth/2,
			textY + letterHeight);

	SelectObject(hdc,oldBrush);

	// I 그리기
	MoveToEx(hdc,startX + letterSpacing*4,textY - letterHeight,NULL);
	LineTo(hdc,startX + letterSpacing*4,textY + letterHeight);

	// N 그리기
	MoveToEx(hdc,startX + letterSpacing*5 - letterWidth/2,textY + letterHeight,NULL);
	LineTo(hdc,startX + letterSpacing*5 - letterWidth/2,textY - letterHeight);
	LineTo(hdc,startX + letterSpacing*5 + letterWidth,textY + letterHeight);
	LineTo(hdc,startX + letterSpacing*5 + letterWidth,textY - letterHeight);

	// G 그리기
	// 반원 그리기
	Arc(hdc,
		startX + letterSpacing*6 - letterWidth,
		textY - letterHeight,
		startX + letterSpacing*6 + letterWidth,
		textY + letterHeight,
		startX + letterSpacing*6 + letterWidth,textY - letterHeight/2,
		startX + letterSpacing*6 - letterWidth,textY - letterHeight/2);

	// G의 아래쪽 가로선
	MoveToEx(hdc,startX + letterSpacing*6 + letterWidth/2,textY,NULL);
	LineTo(hdc,startX + letterSpacing*6 + letterWidth,textY);

	// G의 오른쪽 세로선
	MoveToEx(hdc,startX + letterSpacing*6 + letterWidth,textY,NULL);
	LineTo(hdc,startX + letterSpacing*6 + letterWidth,textY + letterHeight);

	// 점 그리기 (애니메이션)
	HBRUSH dotBrush = CreateSolidBrush(RGB(180,0,0));
	SelectObject(hdc,dotBrush);

	for(int i = 0; i < dotCount; i++) {
		Ellipse(hdc,
				startX + letterSpacing*7 + i*10 - 3,textY + 10,
				startX + letterSpacing*7 + i*10 + 3,textY + 16);
	}

	DeleteObject(dotBrush);

	// 자원 해제
	SelectObject(hdc,oldPen);
	DeleteObject(textPen);
}