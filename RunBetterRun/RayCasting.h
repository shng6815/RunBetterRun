#pragma once
#include "GameObject.h"
#include <queue>
#include <list>

#define CEILING_COLOR 0x383838
#define FLOOR_COLOR 0x717171

#define MAP_COLUME 24
#define MAP_ROW 24
#define MOVE_SPEED 1.2f
#define ROTATE_SPEED 0.8f
#define SHADE_VALUE 1.5f
#define FLOAT(n) static_cast<float>(n)
#define INT(n) static_cast<int>(n)

#define TILE_ROW_SIZE	20
#define TILE_COLUME_SIZE	9
#define TILE_SIZE	32

#define THREAD_NUM 4

class RayCasting;

typedef struct tagTexture
{
	vector<COLORREF>	bmp;
	DWORD				bmpWidth;
	DWORD				bmpHeight;
} Texture;

typedef struct tagSprite
{
	FPOINT		pos;
	double		distance;
	Texture*	texture;
} Sprite;

typedef struct tagThreadData {
	RayCasting*		pThis;
	BOOL			exit;
	BOOL			done;
	queue<POINT>*	queue;
	LPHANDLE		threadMutex;
	LPHANDLE		queueMutex;
} ThreadData;

typedef struct tagRay
{
	int			column;
	int			row;
	float		distance;
	int			side;
	int			height;
	FPOINT		ray_pos;
	FPOINT		ray_dir;
	FPOINT		map_pos;
	FPOINT		side_dist;
	FPOINT		delta_dist;
	FPOINT		step;
	float		wall_x;
	FPOINT		floor_wall;
	FPOINT		c_floor;

	tagRay(FPOINT pos, FPOINT plane, FPOINT cameraDir, float cameraX);
} Ray;

class RayCasting: public GameObject
{
	map<LPCWCH, Texture>	spritesTextureData;
	list<Sprite>			sprites;

	HANDLE			threads[THREAD_NUM];
	ThreadData		threadDatas[THREAD_NUM];
	HANDLE			threadMutex[THREAD_NUM];
	LONG			colsPerThread;
	queue<POINT>	threadQueue;
	HANDLE			queueMutex;

	BITMAPINFO		bmi;
	BYTE			pixelData[WINSIZE_X * WINSIZE_Y * 3];
	
	Texture			tile;

	int     renderScale;
	int     currentFPS;
	int     fpsCheckCounter;
	float   fpsCheckTime;

	float	fov;

	float	rotateSpeed;
	float	moveSpeed;

	FPOINT	cameraPos;
	FPOINT	cameraDir;
	FPOINT	cameraXDir;
	FPOINT	plane;

	FPOINT	move;
	FPOINT	x_move;
	FPOINT	rotate;
	static int map[MAP_ROW * MAP_COLUME];
	float	camera_x[WINSIZE_X];
	float	depth[WINSIZE_X];
	float	sf_dist[WINSIZE_Y];

	bool isShowMouse;

	void KeyInput(void);
	void MouseInput(void);
	void MoveCamera(float deltaTime);
	void MoveSideCamera(float deltaTime);
	void RotateCamera(float deltaTime);
	Ray RayCast(int colume);
	void RenderWall(Ray& ray, int colume);
	void RenderCeilingFloor(Ray& ray, int colume);
	void RenderCeilingFloor(Ray& ray, int colume, COLORREF ceiling, COLORREF floor);
	void RenderPixel(FPOINT pixel, int color);
	COLORREF GetDistanceShadeColor(int tile, FPOINT texturePixel, float distance, bool isSide = false);
	COLORREF GetDistanceShadeColor(COLORREF color, float distance);
	HRESULT LoadTexture(LPCWCH path, Texture& texture);
	void PutSprite(LPCWCH path, FPOINT pos);
	int GetRenderScaleBasedOnFPS(void);
	void SortSpritesByDistance(void);
	void RenderSprites(void);
	void RenderSpritePixel(FPOINT pixel, Sprite& sprite);

public:
	virtual HRESULT Init(void) override;
	virtual void Release(void) override;
	virtual void Update(void) override;
	virtual void Render(HDC hdc) override;

	void FillScreen(DWORD start, DWORD end);
};

