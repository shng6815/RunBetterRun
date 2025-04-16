#pragma once
#include "GameObject.h"
#include <queue>

#define CEILING_COLOR 0x383838       // 천장 색상
#define FLOOR_COLOR 0x717171         // 바닥 색상

#define MAP_COLUME 24                // 맵 가로 타일 수
#define MAP_ROW 24                   // 맵 세로 타일 수
#define MOVE_SPEED 1.2f              // 카메라 이동 속도
#define ROTATE_SPEED 0.8f            // 카메라 회전 속도
#define SHADE_VALUE 1.5f             // 거리 감쇠 쉐이딩 계수

#define FLOAT(n) static_cast<float>(n)  // float 캐스팅 헬퍼 매크로
#define INT(n) static_cast<int>(n)      // int 캐스팅 헬퍼 매크로

#define TILE_ROW_SIZE 20             // 텍스처 타일 행 수
#define TILE_COLUME_SIZE 9           // 텍스처 타일 열 수
#define TILE_SIZE 32                 // 타일 하나의 픽셀 크기

#define THREAD_NUM 4                 // 렌더링용 스레드 개수

class RayCasting;

// 스레드 관련 구조체
typedef struct tagThreadData {
	RayCasting* pThis;          // RayCasting 인스턴스 포인터
	BOOL			exit;           // 스레드 종료 여부
	BOOL			done;           // 스레드 완료 여부
	queue<POINT>* queue;          // 렌더링할 열 정보 큐
	LPHANDLE		threadMutex;    // 스레드 동기화용 뮤텍스
	LPHANDLE		queueMutex;     // 큐 동기화용 뮤텍스
} ThreadData;

// Ray(광선) 구조체
typedef struct tagRay
{
	int			column;         // 광선이 투사된 화면의 열
	int			row;            // 맵 좌표
	float		distance;       // 충돌 지점까지의 거리
	int			side;           // 벽면 방향 (x 또는 y축)
	int			height;         // 벽 높이
	FPOINT		ray_pos;        // 광선 시작 위치
	FPOINT		ray_dir;        // 광선 방향
	FPOINT		map_pos;        // 타일 좌표
	FPOINT		side_dist;      // 다음 x, y 경계까지 거리
	FPOINT		delta_dist;     // x, y 이동 시 거리 증분
	FPOINT		step;           // 이동 방향
	float		wall_x;         // 벽 충돌 지점의 x 좌표
	FPOINT		floor_wall;     // 바닥 계산용 좌표
	FPOINT		c_floor;        // 보간용 바닥 좌표

	tagRay(FPOINT pos, FPOINT plane, FPOINT cameraDir, float cameraX);
} Ray;


class RayCasting: public GameObject
{
	// 멀티스레딩 관련
	HANDLE			threads[THREAD_NUM];            // 스레드 핸들
	ThreadData		threadDatas[THREAD_NUM];        // 각 스레드용 데이터
	HANDLE			threadMutex[THREAD_NUM];        // 스레드별 뮤텍스
	LONG			colsPerThread;                  // 각 스레드가 처리할 열 수
	queue<POINT>	threadQueue;                    // 열 작업 큐
	HANDLE			queueMutex;                     // 큐 접근 동기화

	// 렌더링 관련
	BITMAPINFO		bmi;                            // 화면 출력용 비트맵 정보
	BYTE			pixelData[WINSIZE_X * WINSIZE_Y * 3]; // 픽셀 버퍼 (RGB)

	vector<COLORREF>textureData;                    // 타일 텍스처 데이터
	DWORD			tileWidth, tileHeight;          // 텍스처의 폭과 높이
	DWORD			tileRowSize;                    // 텍스처 행 개수

	BOOL			changeScreen;                   // 화면 갱신 여부

	// 카메라 및 렌더링
	int     renderScale;               // 해상도 스케
	int     currentFPS;                // FPS 추적
	int     fpsCheckCounter;          // FPS 계산용 카운터
	float   fpsCheckTime;             // FPS 측정 시간

	float	fov;                      // 시야각(FOV)
	float	rotateSpeed;             // 회전 속도
	float	moveSpeed;               // 이동 속도

	FPOINT	cameraPos;               // 카메라 위치
	FPOINT	cameraDir;               // 카메라 방향
	FPOINT	cameraXDir;              // x축 카메라 방향
	FPOINT	plane;                   // 카메라 평면 (시야 영역 결정)

	FPOINT	move;                    // 전진/후진 이동 벡터
	FPOINT	x_move;                  // 좌우 이동 벡터
	FPOINT	rotate;                  // 회전 벡터

	static int map[MAP_ROW * MAP_COLUME];   // 정적 맵 데이터
	float	camera_x[WINSIZE_X];            // 화면 열에 따른 카메라 X 좌표
	float	depth[WINSIZE_X];               // 거리 버퍼 (z-buffer)
	float	sf_dist[WINSIZE_Y];             // 천장/바닥 보간 거리
 
	void KeyInput(void);                            // 키 입력 처리
	void MoveCamera(float deltaTime);               // 전진/후진 이동
	void MoveSideCamera(float deltaTime);           // 좌우 이동
	void RotateCamera(float deltaTime);             // 카메라 회전

	Ray RayCast(int colume);                        // 하나의 열에서 광선 발사
	void RenderWall(Ray& ray, int colume);          // 벽 렌더링
	void RenderCeilingFloor(Ray& ray, int colume);  // 천장/바닥 렌더링
	void RenderCeilingFloor(Ray& ray, int colume, COLORREF ceiling, COLORREF floor); // 색상 지정 천장/바닥 렌더링
	void RenderPixel(FPOINT pixel, int color);      // 화면 픽셀 그리기

	COLORREF GetDistanceShadeColor(int tile, FPOINT texturePixel, float distance); // 거리 기반 쉐이딩 (타일용)
	COLORREF GetDistanceShadeColor(COLORREF color, float distance);               // 거리 기반 쉐이딩 (색상만)

	void LoadTextureTiles(LPCWCH path);             // 텍스처 로드
	int GetRenderScaleBasedOnFPS(void);             // FPS 기반 스케일 조정

public:
	virtual HRESULT Init(void) override;
	virtual void Release(void) override;
	virtual void Update(void) override;
	virtual void Render(HDC hdc) override;

	// 유틸리티
	void FillScreen(DWORD start, DWORD end); // 특정 범위 색상 채우기
};

