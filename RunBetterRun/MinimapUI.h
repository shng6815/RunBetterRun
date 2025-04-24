#pragma once
#include "UIUnit.h"

class MinimapUI: public UIUnit
{
public:
	HRESULT Init(UIType type,FPOINT relPos,FPOINT relSize,UIUnit* parent,INT layer);
	void Release() override;
	void Update() override;
	void Render(HDC hdc) override;

	void DrawMiniMapToDC(HDC hdc,int drawSize);
	void DrawMiniMapWithRotation(HDC hdc,int size,float angle);
	void SetShowMonsters(bool show) {
		showMonsters = show;
	}
private:
	// 회전 최적화 관련 멤버 변수
	int frameSkip;           // 몇 프레임마다 회전 연산을 수행할지
	int frameCounter;        // 현재 프레임 카운터
	float lastAngle;         // 마지막으로 계산된 회전 각도
	HDC cachedDC;            // 캐시된 미니맵 DC
	HBITMAP cachedBitmap;    // 캐시된 미니맵 비트맵
	HBITMAP oldCachedBitmap; // 이전 비트맵 저장용
	bool needsUpdate;        // 강제 업데이트 필요 여부

	bool showMonsters;      // 몬스터 표시 여부 플래그
	void DrawDirectionTriangle(HDC hdc,float angle,int drawSize,int halfTile,COLORREF color);
};