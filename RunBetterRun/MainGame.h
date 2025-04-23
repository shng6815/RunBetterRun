#pragma once
#include "GameObject.h"

class Timer;
class Image;
class EnemyManager;
class MapEditor;
class MainGame : public GameObject
{
private:
	HDC hdc;
	PAINTSTRUCT ps;
	wchar_t szText[128];

	Image* backBuffer;
	MapEditor* tilemapTool;

	Timer* timer;

public:
	virtual HRESULT Init() override;
	virtual void Release() override;
	virtual void Update() override;
	void Render();

	LRESULT MainProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

	MainGame();
	~MainGame();
};

