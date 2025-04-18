#pragma once
#include "GameObject.h"
#include "structs.h"
#include <queue>

#define CEILING_COLOR 0x383838
#define FLOOR_COLOR 0x717171

#define SHADE_VALUE 0.4f
#define FLOAT(n) static_cast<float>(n)
#define INT(n) static_cast<int>(n)

#define EPSILON 1e-6f
#define THREAD_NUM 5
#define SCALE 4

class RayCast : public GameObject
{
public:
	typedef struct tagThreadData {
		RayCast*		pThis;
		LPBOOL			exit;
		LPDWORD			done;
		queue<POINT>*	queue;
		LPHANDLE		threadMutex;
		LPHANDLE		queueMutex;
	} ThreadData;

private:
	HANDLE			threads[THREAD_NUM];
	ThreadData		threadDatas[THREAD_NUM];
	DWORD			threadJobDone;
	HANDLE			threadMutex;
	LONG			colsPerThread;
	BOOL			threadTermination;
	queue<POINT>	threadQueue;
	HANDLE			queueMutex;

	BITMAPINFO		bmi;
	BYTE			pixelData[WINSIZE_X * WINSIZE_Y * 3];
	
	Texture	*		mapTile;

	int     renderScale;
	int     currentFPS;
	int     fpsCheckCounter;
	float   fpsCheckTime;
	
	const int* mapData;
	int mapWidth;
	int mapHeight;

	//static int map[MAP_ROW * MAP_COLUME];
	float	screenWidthPixelUnitPos[WINSIZE_X];
	float	screenWidthRayDistance[WINSIZE_X];
	float	screenHeightPixelDepths[WINSIZE_Y];

	Ray RayCasting(int column);
	void RenderWall(Ray& ray, int column);
	void RenderCeilingFloor(Ray& ray, int column);
	void RenderCeilingFloor(Ray& ray, int column, COLORREF ceiling, COLORREF floor);
	void RenderPixel(FPOINT pixel, int color);
	COLORREF GetDistanceShadeColor(int tile, FPOINT texturePixel, float distance);
	COLORREF GetDistanceShadeColor(COLORREF color, float distance);
	void RenderSprites(DWORD start, DWORD end);
	void RenderSprite(const Sprite* sprite, POINT renderX, POINT renderY, FPOINT transform);
	int GetRenderScaleBasedOnFPS(void);

public:
	virtual HRESULT Init(void) override;
	virtual void Release(void) override;
	virtual void Update(void) override;
	virtual void Render(HDC hdc) override;

	void FillScreen(DWORD start, DWORD end);
};
