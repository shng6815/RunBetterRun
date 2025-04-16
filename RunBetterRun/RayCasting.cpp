#include "RayCasting.h"
#include "KeyManager.h"
#include <fstream>

int RayCasting::map[MAP_ROW * MAP_COLUME] =
{
  1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,1,
  1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,24,
  90,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,25,
  89,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,26,
  88,0,0,0,0,0,2,2,2,2,2,0,0,0,0,3,0,3,0,3,0,0,0,27,
  87,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,28,
  86,0,0,0,0,0,2,0,0,0,2,0,0,0,0,3,0,0,0,3,0,0,0,29,
  85,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,30,
  84,0,0,0,0,0,2,2,0,2,2,0,0,0,0,3,0,3,0,3,0,0,0,31,
  83,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,32,
  82,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,34,
  81,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,35,
  80,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,36,
  79,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,37,
  78,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,38,
  77,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,39,
  76,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,
  75,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,41,
  74,4,0,0,0,0,5,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,42,
  73,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,43,
  72,4,0,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,44,
  71,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,45,
  70,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,46,
  1,68,67,66,65,64,63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,47,1
};

DWORD WINAPI RaycastThread(LPVOID lpParam) { // 렌더링을 수행하는 쓰레드 함수
    ThreadData* data = static_cast<ThreadData*>(lpParam); // 전달된 데이터를 ThreadData 구조체로 캐스팅
    while (data && !data->exit) // 쓰레드 종료 플래그가 설정될 때까지 반복
    {
        WaitForSingleObject(*(data->queueMutex), INFINITE); // 작업 큐 접근을 위한 뮤텍스 잠금
        if (data->queue && !data->queue->empty()) // 큐가 존재하고 비어있지 않으면
        {
            POINT colume = data->queue->front(); // 큐에서 하나의 작업(컬럼 범위)을 꺼냄
            data->queue->pop(); // 큐에서 제거
            ReleaseMutex(*(data->queueMutex)); // 큐에 대한 뮤텍스 해제
            data->pThis->FillScreen(colume.x, colume.y); // 해당 컬럼 범위에 대해 화면을 렌더링
            WaitForSingleObject(*(data->threadMutex), INFINITE); // 쓰레드 상태 갱신을 위한 뮤텍스 잠금
            data->done = TRUE; // 작업 완료 플래그 설정
            ReleaseMutex(*(data->threadMutex)); // 쓰레드 상태 뮤텍스 해제
        }
        else
            ReleaseMutex(*(data->queueMutex)); // 큐가 비어 있으면 그냥 뮤텍스 해제
    }
    return 0; // 쓰레드 종료
}

HRESULT RayCasting::Init(void) // RayCasting 클래스의 초기화 함수
{
    fov = 0.66f; // 시야각(Field of View) 설정

    cameraPos = { 22, 12 }; // 카메라(플레이어) 위치 초기화
    cameraDir = { -1, 0 }; // 카메라(플레이어) 방향 초기화
    cameraXDir = { cameraDir.y, -cameraDir.x }; // X축 방향 벡터 (직교 방향)
    plane = { 0, -2.0f }; // 뷰포트 거리 (카메라 방향에 수직)

    // 각 화면 컬럼에 대응하는 카메라 X 좌표 미리 계산
    for (int i = 0; i < WINSIZE_X; ++i)
        camera_x[i] = ((2.0f * FLOAT(i) / FLOAT(WINSIZE_X)) - 1.0f);

    // 수직 거리 계산용 테이블 생성
    for (int i = 0; i < WINSIZE_Y; ++i)
        sf_dist[i] = FLOAT(WINSIZE_Y) / (2.0f * FLOAT(i) - FLOAT(WINSIZE_Y));

    move = { 0, 0 }; // 이동 벡터 초기화
    x_move = { 0, 0 }; // 횡이동 벡터 초기화
    rotate = { 0, 0 }; // 회전 값 초기화

    // 비트맵 정보 초기화 (후면 버퍼용)
    memset(&bmi, 0, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = WINSIZE_X;
    bmi.bmiHeader.biHeight = -WINSIZE_Y; // 음수: 위에서 아래로 출력
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 24; // 24비트 RGB
    bmi.bmiHeader.biCompression = BI_RGB; // 압축 없음

    LoadTextureTiles(TEXT("Image/maptiles.bmp")); // 텍스처 이미지 로딩

    renderScale = 1; // 렌더 스케일 기본값
    currentFPS = 60; // 기본 FPS 설정
    fpsCheckCounter = 0; // FPS 카운터 초기화
    fpsCheckTime = 0.0f; // FPS 측정 시간 초기화

    changeScreen = FALSE; // 화면 갱신 플래그 초기화

    // 렌더링 쓰레드 관련 초기화
    queueMutex = CreateMutex(NULL, FALSE, NULL); // 작업 큐 보호용 뮤텍스 생성
    colsPerThread = WINSIZE_X / THREAD_NUM; // 쓰레드당 처리할 컬럼 수 계산
    for (DWORD i = 0; i < THREAD_NUM; ++i) {
        threadMutex[i] = CreateMutex(NULL, FALSE, NULL); // 각 쓰레드의 상태 보호용 뮤텍스 생성
        threadDatas[i] = {
            this,                 // RayCasting 인스턴스 포인터
            FALSE,                // 쓰레드 완료 상태 초기화
            FALSE,                // 쓰레드 종료 플래그 초기화
            &threadQueue,         // 작업 큐 포인터
            &threadMutex[i],      // 상태 보호용 뮤텍스 포인터
            &queueMutex           // 큐 보호용 뮤텍스 포인터
        };
        threads[i] = CreateThread(NULL, 0, RaycastThread, &threadDatas[i], 0, NULL); // 쓰레드 생성
    }
    return S_OK; // 초기화 성공
}


void RayCasting::Release(void)
{
    // 모든 쓰레드 종료 플래그 설정
    for (int i = 0; i < THREAD_NUM; ++i)
        threadDatas[i].exit = TRUE;

    // 모든 쓰레드가 종료될 때까지 대기
    WaitForMultipleObjects(THREAD_NUM, threads, TRUE, INFINITE);

    // 각 쓰레드의 핸들과 뮤텍스 해제
    for (int i = 0; i < THREAD_NUM; ++i)
    {
        CloseHandle(threadMutex[i]);
        CloseHandle(threads[i]);
    }

    // 큐 뮤텍스 해제
    CloseHandle(queueMutex);

    // 남아 있는 작업 큐 비우기
    while (!threadQueue.empty())
        threadQueue.pop();

    // 텍스처 데이터 제거
    textureData.clear();
}

void RayCasting::Update(void)
{
    KeyInput(); // 키 입력 처리

    float deltaTime = TimerManager::GetInstance()->GetDeltaTime(); // 프레임 시간 가져오기

    if (move.x || move.y)
        MoveCamera(deltaTime); // 전후 이동 처리

    if (x_move.x || x_move.y)
        MoveSideCamera(deltaTime); // 좌우 이동 처리

    if (rotate.x || rotate.y)
        RotateCamera(deltaTime); // 회전 처리

    // FPS 체크를 위한 프레임 카운트 누적
    fpsCheckCounter++;
    fpsCheckTime += deltaTime;

    // 1초가 지나면 FPS 측정 및 렌더링 스케일 조절
    if (fpsCheckTime >= 1.0f) {
        currentFPS = fpsCheckCounter;
        fpsCheckCounter = 0;
        fpsCheckTime = 0.0f;
        renderScale = GetRenderScaleBasedOnFPS(); // FPS 기반 렌더링 품질 조절
    }
}

void RayCasting::Render(HDC hdc)
{
    // 화면 전체 컬럼을 쓰레드 수만큼 나눠서 작업 큐에 할당
    WaitForSingleObject(queueMutex, INFINITE);
    for (int i = 0; i < THREAD_NUM - 1; ++i)
        threadQueue.push({ i * colsPerThread,  (i + 1) * colsPerThread });
    threadQueue.push({ (THREAD_NUM - 1) * colsPerThread ,WINSIZE_X });
    ReleaseMutex(queueMutex);

    BOOL print = FALSE;
    while (!print)
    {
        BOOL ready = TRUE;
        // 모든 쓰레드가 완료될 때까지 대기
        WaitForMultipleObjects(THREAD_NUM, threadMutex, TRUE, INFINITE);
        for (int i = 0; i < THREAD_NUM; ++i)
        {
            if (!threadDatas[i].done)
            {
                ready = FALSE;
                // 완료되지 않은 쓰레드가 있으면 뮤텍스 해제 후 루프 탈출
                for (auto& mutex : threadMutex)
                    ReleaseMutex(mutex);
                break;
            }
        }
        if (ready)
        {
            // 모든 쓰레드가 완료되었으면 화면에 출력
            SetDIBitsToDevice(hdc, 0, 0, WINSIZE_X, WINSIZE_Y, 0, 0, 0,
                WINSIZE_Y, pixelData, &bmi, DIB_RGB_COLORS);

            // 쓰레드 완료 상태 초기화
            for (int i = 0; i < THREAD_NUM; ++i)
                threadDatas[i].done = FALSE;

            print = TRUE;
        }
        // 쓰레드 뮤텍스 해제
        for (auto& mutex : threadMutex)
            ReleaseMutex(mutex);
    }
}

void RayCasting::FillScreen(DWORD start, DWORD end)
{
    for (DWORD i = start; i < end; i += renderScale)
    {
        Ray ray = RayCast(i); // 해당 컬럼에서 레이캐스팅 수행
        depth[i] = ray.distance; // 거리 정보를 깊이 버퍼에 저장
        ray.height = fabs(FLOAT(WINSIZE_Y) / ray.distance); // 거리 기반으로 벽 높이 계산

        for (DWORD j = 0; j < renderScale && i + j < WINSIZE_X; ++j) {
            RenderWall(ray, i + j); // 벽 렌더링
            if (ray.height < WINSIZE_Y)
                RenderCeilingFloor(ray, i + j); // 천장 및 바닥 렌더링 (높이가 화면보다 작을 때만)
        }
    }
}



void RayCasting::KeyInput(void)
{
    KeyManager* km = KeyManager::GetInstance();
    changeScreen = FALSE;

    if (km->IsStayKeyDown('W'))
    {
        move.x = 1;
        changeScreen = TRUE;
    }
    else
        move.x = 0;

    if (km->IsStayKeyDown('S'))
    {
        move.y = 1;
        changeScreen = TRUE;
    }
    else
        move.y = 0;

    if (km->IsStayKeyDown('A'))
    {
        x_move.x = 1;
        changeScreen = TRUE;
    }
    else
        x_move.x = 0;

    if (km->IsStayKeyDown('D'))
    {
        x_move.y = 1;
        changeScreen = TRUE;
    }
    else
        x_move.y = 0;

    if (km->IsStayKeyDown('Q'))
    {
        rotate.x = 1;
        changeScreen = TRUE;
    }
    else
        rotate.x = 0;

    if (km->IsStayKeyDown('E'))
    {
        rotate.y = 1;
        changeScreen = TRUE;
    }
    else
        rotate.y = 0;
}

void RayCasting::MoveCamera(float deltaTime)
{
    bool direction = move.x ? false : true; // move.x가 있으면 뒤로, 아니면 앞으로

    FPOINT pos = cameraPos;
    pos.x += (direction ? -1 : 1) * (cameraDir.x * MOVE_SPEED * deltaTime); // x축 이동 시도
    int x = INT(pos.x);
    int y = INT(pos.y);

    // 충돌 검사: 맵 범위 내이고 벽이 없으면 이동
    if ((0 <= x && x < MAP_COLUME && 0 <= y && y < MAP_ROW)
        && map[MAP_COLUME * y + x] == 0)
        cameraPos = pos;

    pos = cameraPos;
    pos.y += (direction ? -1 : 1) * (cameraDir.y * MOVE_SPEED * deltaTime); // y축 이동 시도
    x = INT(pos.x);
    y = INT(pos.y);

    if ((0 <= x && x < MAP_COLUME && 0 <= y && y < MAP_ROW)
        && map[MAP_COLUME * y + x] == 0)
        cameraPos = pos;
}

void RayCasting::MoveSideCamera(float deltaTime)
{
    bool direction = x_move.x ? false : true; // x_move.x가 있으면 왼쪽, 아니면 오른쪽

    FPOINT pos = cameraPos;
    pos.x += (direction ? -1 : 1) * (cameraXDir.x * MOVE_SPEED * deltaTime) + 1e-6f; // x축 이동
    int x = INT(pos.x);
    int y = INT(pos.y);

    // 충돌 검사
    if ((0 <= x && x < MAP_COLUME && 0 <= y && y < MAP_ROW)
        && map[MAP_COLUME * y + x] == 0)
        cameraPos = pos;

    pos = cameraPos;
    pos.y += (direction ? -1 : 1) * (cameraXDir.y * MOVE_SPEED * deltaTime) + 1e-6f; // y축 이동
    x = INT(pos.x);
    y = INT(pos.y);

    if ((0 <= x && x < MAP_COLUME && 0 <= y && y < MAP_ROW)
        && map[MAP_COLUME * y + x] == 0)
        cameraPos = pos;
}

void RayCasting::RotateCamera(float deltaTime)
{
    float rotateCos, rotateSin;
    if (rotate.x)
    {
        rotateCos = cosf(-ROTATE_SPEED * deltaTime); // 왼쪽 회전
        rotateSin = sinf(-ROTATE_SPEED * deltaTime);
    }
    else
    {
        rotateCos = cosf(ROTATE_SPEED * deltaTime); // 오른쪽 회전
        rotateSin = sinf(ROTATE_SPEED * deltaTime);
    }

    FPOINT old = cameraDir;

    // 카메라 방향 회전
    cameraDir.x = (cameraDir.x * rotateCos) - (cameraDir.y * rotateSin);
    cameraDir.y = (old.x * rotateSin) + (cameraDir.y * rotateCos);

    // 시야 평면 회전
    old = plane;
    plane.x = (plane.x * rotateCos) - (plane.y * rotateSin);
    plane.y = (old.x * rotateSin) + (plane.y * rotateCos);

    // 카메라의 x축 방향도 회전
    old = cameraXDir;
    cameraXDir.x = (cameraXDir.x * rotateCos) - (cameraXDir.y * rotateSin);
    cameraXDir.y = (old.x * rotateSin) + (cameraXDir.y * rotateCos);
}

Ray RayCasting::RayCast(int colume)
{
    bool hit = false;
    bool nextSide = false;
    Ray ray(cameraPos, plane, cameraDir, camera_x[colume]);

    ray.column = colume;

    // 벽이 감지될 때까지 DDA로 탐색
    while (!hit)
    {
        nextSide = ray.side_dist.x < ray.side_dist.y;

        ray.side_dist.x += nextSide * ray.delta_dist.x;
        ray.map_pos.x += nextSide * ray.step.x;

        ray.side_dist.y += (!nextSide) * ray.delta_dist.y;
        ray.map_pos.y += (!nextSide) * ray.step.y;

        ray.side = !nextSide; // 수직 벽이면 false, 수평 벽이면 true

        int x = INT(ray.map_pos.x);
        int y = INT(ray.map_pos.y);

        if (x < 0 || MAP_COLUME <= x || y < 0 || MAP_ROW <= y)
            break;

        int map_index = y * MAP_COLUME + x;
        if (map[map_index] > 0) // 벽에 충돌
            hit = true;
    }

    // 거리 계산 (수직/수평에 따라 다르게 계산)
    float pos;
    if (ray.side)
    {
        pos = (ray.map_pos.y - cameraPos.y + (1.0f - ray.step.y) / 2.0f);
        ray.distance = fabs(pos / ray.ray_dir.y);
    }
    else
    {
        pos = (ray.map_pos.x - cameraPos.x + (1.0f - ray.step.x) / 2.0f);
        ray.distance = fabs(pos / ray.ray_dir.x);
    }
    return ray; // 충돌지점
}

void RayCasting::RenderWall(Ray& ray, int colume)
{
    FPOINT pixel = { colume, max(0, WINSIZE_Y / 2.0f - (ray.height / 2.0f)) }; // 시작 픽셀 위치

    // 벽의 x좌표 계산 (정확한 텍스처 좌표를 위한 보정)
    if (ray.side)
        ray.wall_x = ray.ray_pos.x
        + ((ray.map_pos.y - ray.ray_pos.y
            + (1. - ray.step.y) / 2.) / ray.ray_dir.y)
        * ray.ray_dir.x;
    else
        ray.wall_x = ray.ray_pos.y
        + ((ray.map_pos.x - ray.ray_pos.x
            + (1. - ray.step.x) / 2.) / ray.ray_dir.x)
        * ray.ray_dir.y;
    ray.wall_x -= INT(ray.wall_x); // 소수점만 추출

    // 텍스처의 x좌표 계산
    FPOINT texture = { INT(ray.wall_x * TILE_SIZE), 0.0f };
    if ((ray.side == 0 && ray.ray_dir.x > 0.0f)
        || (ray.side == 1 && ray.ray_dir.y < 0.0f))
        texture.x = TILE_SIZE - texture.x - 1.0f;

    int i = max(0, INT(WINSIZE_Y / 2.0f - ray.height / 2.0f));
    int j = 0;

    int tile = map[INT(ray.map_pos.y) * MAP_COLUME + INT(ray.map_pos.x)];
    while (j < ray.height && i < WINSIZE_Y)
    {
        texture.y = INT((i * 2 - WINSIZE_Y + ray.height)
            * ((TILE_SIZE / 2.0f) / ray.height)); // 텍스처 y좌표 계산

        for (int k = 0; k < renderScale && k + i < WINSIZE_Y; ++k)
        {
            pixel.y = k + i;
            RenderPixel(pixel, GetDistanceShadeColor(tile, texture, ray.distance)); // 음영 포함 렌더링
        }
        j += renderScale;
        i += renderScale;
    }
}


void RayCasting::RenderCeilingFloor(Ray& ray, int colume)
{
    // 충돌 방향과 광선 방향에 따라 바닥/천장 위치 결정
    if (ray.side == 0 && ray.ray_dir.x > 0)
        ray.floor_wall = { ray.map_pos.x, ray.map_pos.y + ray.wall_x };
    else if (ray.side == 0 && ray.ray_dir.x < 0)
        ray.floor_wall = { ray.map_pos.x + 1, ray.map_pos.y + ray.wall_x };
    else if (ray.side && ray.ray_dir.y > 0)
        ray.floor_wall = { ray.map_pos.x + ray.wall_x, ray.map_pos.y };
    else if (ray.side && ray.ray_dir.y < 0)
        ray.floor_wall = { ray.map_pos.x + ray.wall_x, ray.map_pos.y + 1 };

    FPOINT pixel = { colume, 0 };
    for (int i = INT(WINSIZE_Y / 2 + ray.height / 2.0f); i < WINSIZE_Y; i += renderScale) // 수직 스캔
    {
        float weight = sf_dist[i] / ray.distance; // 보간 가중치 계산
        ray.c_floor = { weight * ray.floor_wall.x + (1.0f - weight) * cameraPos.x,
                        weight * ray.floor_wall.y + (1.0f - weight) * cameraPos.y }; // 현재 floor 좌표 보간

        FPOINT texture = { INT(ray.c_floor.x * TILE_SIZE) % TILE_SIZE,
                           INT(ray.c_floor.y * TILE_SIZE) % TILE_SIZE }; // 텍스처 좌표 계산

        for (int j = 0; i + j < WINSIZE_Y && j < renderScale; ++j) {
            int row = i + j;
            pixel.y = row;

            RenderPixel(pixel, GetDistanceShadeColor(8, texture, sf_dist[row])); // 바닥 그리기
            RenderPixel({ pixel.x, WINSIZE_Y - (pixel.y + 1) }, GetDistanceShadeColor(9, texture, sf_dist[row])); // 천장 그리기
        }
    }
}

void RayCasting::RenderCeilingFloor(Ray& ray, int colume, COLORREF ceiling, COLORREF floor)
{
    FPOINT pixel = { colume, 0 };
    for (int i = INT(WINSIZE_Y / 2 + ray.height / 2.0f); i < WINSIZE_Y; i += renderScale) // 수직 라인별로 반복
    {
        for (int j = 0; j < renderScale && i + j < WINSIZE_Y; ++j) {
            int row = i + j;
            pixel.y = row;

            RenderPixel(pixel, GetDistanceShadeColor(floor, sf_dist[row])); // 바닥 색상
            RenderPixel({ pixel.x, WINSIZE_Y - (pixel.y + 1) }, GetDistanceShadeColor(ceiling, sf_dist[row])); // 천장 색상
        }
    }
}

void RayCasting::RenderPixel(FPOINT pixel, int color)
{
    int pixelPos = (WINSIZE_X * INT(pixel.y) + INT(pixel.x)) * 3; // RGB 기준 위치 계산
    *reinterpret_cast<LPDWORD>(&pixelData[pixelPos]) = color; // 색상 저장
}

COLORREF RayCasting::GetDistanceShadeColor(int tile, FPOINT texturePixel, float distance)
{
    float divide = distance / SHADE_VALUE; // 거리 기반 명암 조정 비율

    --tile;
    int row = tile / TILE_ROW_SIZE;
    int colume = tile % TILE_ROW_SIZE;

    texturePixel.x += colume * TILE_SIZE; // 텍스처 시트 내 타일 오프셋 적용
    texturePixel.y += row * TILE_SIZE;

    COLORREF color = textureData[INT(texturePixel.y * tileWidth + texturePixel.x)]; // 픽셀 색상 추출
    if (divide <= 1.0f)
        return color; // 가까우면 원래 색상
    else
        return RGB(INT(GetRValue(color) / divide), // 멀면 명암 적용
            INT(GetGValue(color) / divide),
            INT(GetBValue(color) / divide));
}

COLORREF RayCasting::GetDistanceShadeColor(COLORREF color, float distance)
{
    float divide = distance / SHADE_VALUE;
    if (divide <= 1.0f)
        return color; // 가까우면 그대로
    else
        return RGB(INT(GetRValue(color) / divide), // 멀면 어둡게
            INT(GetGValue(color) / divide),
            INT(GetBValue(color) / divide));
}

void RayCasting::LoadTextureTiles(LPCWCH path)
{
    std::ifstream file(path, std::ios::binary);

    if (!file.is_open()) {
        std::wcerr << TEXT("Error opening file: ") << path << std::endl;
        return;
    }

    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;

    file.read(reinterpret_cast<LPCH>(&fileHeader), sizeof(fileHeader)); // BMP 헤더 읽기
    file.read(reinterpret_cast<LPCH>(&infoHeader), sizeof(infoHeader));

    if (fileHeader.bfType != 0x4D42) { // 'BMP' 확인
        std::wcerr << TEXT("Not a valid BMP file!") << std::endl;
        return;
    }

    tileWidth = infoHeader.biWidth;
    tileHeight = infoHeader.biHeight;

    if (infoHeader.biBitCount != 24) {
        std::wcerr << TEXT("Only 24-bit BMP files are supported!") << std::endl;
        return;
    }

    tileRowSize = (tileWidth * 3 + 3) & ~3; // row padding 계산
    vector<BYTE> tileData(tileRowSize * tileHeight);

    file.seekg(fileHeader.bfOffBits, std::ios::beg); // 픽셀 시작 위치로 이동
    file.read(reinterpret_cast<LPCH>(tileData.data()), tileData.size());
    file.close();

    textureData.resize(tileWidth * tileHeight);
    for (DWORD i = 0; i < tileWidth * tileHeight; ++i)
    {
        DWORD index = (tileHeight - (i / tileWidth) - 1) * tileRowSize + (i % tileWidth) * 3; // BMP는 bottom-up 방식
        // (tileHeight - (i / tileWidth) - 1) * tileRowSize -> y값
        // (i % tileWidth) * 3; -> X 값
        textureData[i] = *reinterpret_cast<LPDWORD>(&tileData[index]); // 픽셀 저장
    }
}

int RayCasting::GetRenderScaleBasedOnFPS(void)
{
    if (currentFPS < 15) return 8;      // 매우 낮은 FPS → 저해상도
    else if (currentFPS < 25) return 4; // 낮음
    else if (currentFPS < 40) return 2; // 보통
    else return 1;                      // 충분한 FPS → 고해상도
}

tagRay::tagRay(FPOINT pos, FPOINT plane, FPOINT cameraDir, float cameraX)
{
    ray_pos = pos;
    map_pos = { FLOAT(INT(pos.x)), FLOAT(INT(pos.y)) }; // 현재 정수 맵 위치
    ray_dir = { cameraDir.x + plane.x * cameraX, cameraDir.y + plane.y * cameraX }; // 광선 방향
    delta_dist = { fabs(1.0f / ray_dir.x), fabs(1.0f / ray_dir.y) }; // delta 거리 계산

    if (ray_dir.x < 0) {
        step = { -1.0f, (ray_dir.y < 0 ? -1.0f : 1.0f) }; // x 방향 음수일 경우
        side_dist.x = (ray_pos.x - map_pos.x) * delta_dist.x;
    }
    else {
        step = { 1.0f, (ray_dir.y < 0 ? -1.0f : 1.0f) }; // x 방향 양수일 경우
        side_dist.x = (map_pos.x + 1.0f - ray_pos.x) * delta_dist.x;
    }

    // 비례식으로 거리를 구함
    if (ray_dir.y < 0)
        side_dist.y = (ray_pos.y - map_pos.y) * delta_dist.y;
    else
        side_dist.y = (map_pos.y + 1 - ray_pos.y) * delta_dist.y;
}
