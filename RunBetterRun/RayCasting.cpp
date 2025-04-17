#include "RayCasting.h"
#include "KeyManager.h"
#include "SpriteManager.h"
#include "MapManager.h"
#include <fstream>
#include "Player.h"

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

DWORD WINAPI RaycastThread(LPVOID lpParam) {
    RayCasting::ThreadData* data = static_cast<RayCasting::ThreadData*>(lpParam);
    while (data && !data->exit)
    {
        WaitForSingleObject(*(data->queueMutex), INFINITE);
        if (data->queue && !data->queue->empty() && !data->done)
        {
            POINT colume = data->queue->front();
            data->queue->pop();
            ReleaseMutex(*(data->queueMutex));
            data->pThis->FillScreen(colume.x, colume.y);
            WaitForSingleObject(*(data->threadMutex), INFINITE);
            data->done = TRUE;
            ReleaseMutex(*(data->threadMutex));
        }
        else
            ReleaseMutex(*(data->queueMutex));
    }
    return 0;
}


HRESULT RayCasting::Init(void)
{
    Player::GetInstance()->Init();

    for (int i = 0; i < WINSIZE_X; ++i)
        camera_x[i] = ((2.0f * FLOAT(i) / FLOAT(WINSIZE_X)) - 1.0f);

    for (int i = 0; i < WINSIZE_Y; ++i)
        sf_dist[i] = FLOAT(WINSIZE_Y) / (2.0f * FLOAT(i) - FLOAT(WINSIZE_Y));

    memset(&bmi, 0, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = WINSIZE_X;
    bmi.bmiHeader.biHeight = -WINSIZE_Y;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 24;
    bmi.bmiHeader.biCompression = BI_RGB;

    ReloadMapData();

    SpriteManager::GetInstance()->LoadMapTileTexture(TEXT("Image/maptiles.bmp"));
    mapTile = SpriteManager::GetInstance()->GetMapTileTexture();
    SpriteManager::GetInstance()->PutSprite(TEXT("Image/rocket.bmp"), { 19, 12 });
    SpriteManager::GetInstance()->PutSprite(TEXT("Image/rocket.bmp"), { 16, 12 });
    renderScale = SCALE;
    currentFPS = 60;
    fpsCheckCounter = 0;
    fpsCheckTime = 0.0f;

    queueMutex = CreateMutex(NULL, FALSE, NULL);
    colsPerThread = WINSIZE_X / THREAD_NUM;
    for (DWORD i = 0; i < THREAD_NUM; ++i) {
        threadMutex[i] = CreateMutex(NULL, FALSE, NULL);
        threadDatas[i] = {
            this,
            FALSE,
            FALSE,
            &threadQueue,
            &threadMutex[i],
            &queueMutex
        };
        threads[i] = CreateThread(NULL, 0, RaycastThread, &threadDatas[i], 0, NULL);
    }

    return S_OK;
}

void RayCasting::Release(void)
{
    for (int i = 0; i < THREAD_NUM; ++i)
        threadDatas[i].exit = TRUE;

    WaitForMultipleObjects(THREAD_NUM, threads, TRUE, INFINITE);

    for (int i = 0; i < THREAD_NUM; ++i)
    {
        CloseHandle(threadMutex[i]);
        CloseHandle(threads[i]);
    }
    CloseHandle(queueMutex);

    while (!threadQueue.empty())
        threadQueue.pop();

    mapTile = nullptr;
    SpriteManager::GetInstance()->ClearSprites();
    Player::GetInstance()->Release();
}

void RayCasting::Update(void)
{
    float deltaTime = TimerManager::GetInstance()->GetDeltaTime();

    Player::GetInstance()->Update();

    SpriteManager::GetInstance()->UpdatePlayerPosition(Player::GetInstance()->GetCameraPos());
    SpriteManager::GetInstance()->SortSpritesByDistance();

    fpsCheckCounter++;
    fpsCheckTime += deltaTime;

    if (fpsCheckTime >= 1.0f) {
        currentFPS = fpsCheckCounter;
        fpsCheckCounter = 0;
        fpsCheckTime = 0.0f;
        renderScale = GetRenderScaleBasedOnFPS();
    }
}


void RayCasting::Render(HDC hdc)
{
    WaitForSingleObject(queueMutex, INFINITE);
    for (int i = 0; i < THREAD_NUM - 1; ++i)
        threadQueue.push({ i * colsPerThread,  (i + 1) * colsPerThread });
    threadQueue.push({ (THREAD_NUM - 1) * colsPerThread ,WINSIZE_X });
    ReleaseMutex(queueMutex);

    BOOL print = FALSE;
    while (!print)
    {
        BOOL ready = TRUE;
        WaitForMultipleObjects(THREAD_NUM, threadMutex, TRUE, INFINITE);
        for (int i = 0; i < THREAD_NUM; ++i)
        {
            if (!threadDatas[i].done)
            {
                ready = FALSE;
                break;
            }
        }
        if (ready)
        {
            SetDIBitsToDevice(hdc, 0, 0, WINSIZE_X, WINSIZE_Y, 0, 0, 0,
                WINSIZE_Y, pixelData, &bmi, DIB_RGB_COLORS);
            for (int i = 0; i < THREAD_NUM; ++i)
                threadDatas[i].done = FALSE;
            print = TRUE;
        }
        for (auto& mutex : threadMutex)
            ReleaseMutex(mutex);
    }
}

void RayCasting::FillScreen(DWORD start, DWORD end)
{
    DWORD x = start;
    while (x < end)
    {
        Ray ray = RayCast(x);
        ray.height = fabs(FLOAT(WINSIZE_Y) / ray.distance);

        DWORD endX = min(x + renderScale, end);
        while (x < endX) {
            depth[x] = ray.distance;
            RenderWall(ray, x);
            if (ray.height < WINSIZE_Y)
                RenderCeilingFloor(ray, x);
            ++x;
        }
    }
    RenderSprites(start, end);
}

void RayCasting::ReloadMapData()
{
    MapManager* mapManager = MapManager::GetInstance();
    MapData* currentMap = mapManager->GetCurrMapData();

    if (currentMap) {
        mapData = currentMap->tiles.data();
        mapWidth = currentMap->width;
        mapHeight = currentMap->height;
    }
    else {
        mapData = nullptr;
        mapWidth = MAP_COLUME;
        mapHeight = MAP_ROW;
    }

}

void RayCasting::RenderSprites(DWORD start, DWORD end)
{
    const list<Sprite>& sprites = SpriteManager::GetInstance()->GetSprites();
    
    float invDet = 1.0f / ((Player::GetInstance()->GetPlane().x * Player::GetInstance()->GetCameraVerDir().y) - (Player::GetInstance()->GetPlane().y * Player::GetInstance()->GetCameraVerDir().x));

    for (auto& sprite : sprites)
    {
        if (sprite.distance > 0.1f)
        {
            FPOINT pos = { sprite.pos.x - Player::GetInstance()->GetCameraPos().x, sprite.pos.y - Player::GetInstance()->GetCameraPos().y };
            FPOINT transform = { invDet * (Player::GetInstance()->GetCameraVerDir().y * pos.x - Player::GetInstance()->GetCameraVerDir().x * pos.y),
                                invDet * (-Player::GetInstance()->GetPlane().y * pos.x + Player::GetInstance()->GetPlane().x * pos.y) };
            if (fabs(transform.y) < 1e-6f)
                continue;
            int screen = INT((WINSIZE_X / 2) * (1.0f + transform.x / transform.y));
            float size = fabs(WINSIZE_Y / transform.y);
            POINT renderX = { INT(max(0, -size / 2.0f + screen)),
                                INT(max(0, size / 2.0f + screen)) };
            if (renderX.y < start || end <= renderX.x)
                continue;
            if (renderX.x < start && start < renderX.y)
                renderX.x = start;
            if (start <= renderX.x && end < renderX.y)
                renderX.y = end;
            POINT renderY = { INT(max(0, -size / 2.0f + WINSIZE_Y / 2.0f)),
                                INT(max(0, size / 2.0f + WINSIZE_Y / 2.0f)) };
            if (WINSIZE_Y < renderY.y)
                renderY.y = WINSIZE_Y;
            RenderSprite(sprite, renderX, renderY, transform);
        }
    }
}

void RayCasting::RenderSprite(const Sprite& sprite, POINT renderX, POINT renderY, FPOINT transform)
{
    float renderYOrg = renderY.x;
    int screen = INT((WINSIZE_X / 2) * (1.0f + transform.x / transform.y));
    float size = fabs(WINSIZE_Y / transform.y);

    while (renderX.x < renderX.y)
    {
        POINT texturePos = { INT(256 * ((INT(renderX.x) - (-size / 2.0f + screen)))
                       * sprite.texture->bmpWidth / size) / 256, 0 };

        DWORD endX = min(renderX.x + renderScale, renderX.y);
        while (renderX.x < endX)
        {
            if (transform.y > 0
                && transform.y < depth[renderX.x]
                && texturePos.x < sprite.texture->bmpWidth)
            {
                renderY.x = renderYOrg;
                while (renderY.x < renderY.y)
                {
                    LONG fact = (INT(renderY.x) * 256.0f) - (WINSIZE_Y * 128.0f) + (size * 128.0f);
                    texturePos.y = ((fact * sprite.texture->bmpHeight) / size) / 256.0f;
                    if (texturePos.y < sprite.texture->bmpHeight)
                    {
                        COLORREF color =
                            sprite.texture->bmp[texturePos.y * sprite.texture->bmpWidth + texturePos.x];
                        DWORD endY = min(renderY.x + renderScale, renderY.y);
                        while (renderY.x < endY)
                        {
                            FPOINT pixel = { renderX.x, renderY.x };
                            if (color != 0xFF00FF)
                                RenderPixel(pixel, GetDistanceShadeColor(color, sprite.distance));
                            ++renderY.x;
                        }
                    }
                    else
                        ++renderY.x;
                }
            }
            ++renderX.x;
        }
    }
}

Ray RayCasting::RayCast(int colume)
{
    bool    hit = false;
    bool    nextSide = false;
    Ray     ray(Player::GetInstance()->GetCameraPos(), Player::GetInstance()->GetPlane(), Player::GetInstance()->GetCameraVerDir(), camera_x[colume]);

    while (!hit)
    {
        nextSide = ray.side_dist.x < ray.side_dist.y;
        ray.side_dist.x += nextSide * ray.delta_dist.x;
        ray.map_pos.x += nextSide * ray.step.x;
        ray.side_dist.y += (!nextSide) * ray.delta_dist.y;
        ray.map_pos.y += (!nextSide) * ray.step.y;

        ray.side = !nextSide;

        int x = INT(ray.map_pos.x);
        int y = INT(ray.map_pos.y);

        if (x < 0 || MAP_COLUME <= x || y < 0 || MAP_ROW <= y)
            break;
        int map_index = y * MAP_COLUME + x;
        if (map[map_index] > 0)
            hit = true;
    }

    float pos;
    if (ray.side)
    {
        pos = (ray.map_pos.y - Player::GetInstance()->GetCameraPos().y + (1.0f - ray.step.y) / 2.0f);
        ray.distance = fabs(pos / ray.ray_dir.y);
    }
    else
    {
        pos = (ray.map_pos.x - Player::GetInstance()->GetCameraPos().x + (1.0f - ray.step.x) / 2.0f);
        ray.distance = fabs(pos / ray.ray_dir.x);
    }
    return ray;
}


void RayCasting::RenderWall(Ray& ray, int colume)
{
    FPOINT pixel = { colume, max(0, WINSIZE_Y / 2.0f - (ray.height / 2.0f)) };

    if (ray.side)
        ray.wall_x = ray.ray_pos.x
        + ray.distance
        * ray.ray_dir.x;
    else
        ray.wall_x = ray.ray_pos.y
        + ray.distance
        * ray.ray_dir.y;
    ray.wall_x -= INT(ray.wall_x);

    FPOINT texture = { INT(ray.wall_x * TILE_SIZE), 0.0f };
    if ((ray.side == 0 && ray.ray_dir.x > 0.0f)
        || (ray.side == 1 && ray.ray_dir.y < 0.0f))
        texture.x = TILE_SIZE - texture.x - 1.0f;

    int tile = map[INT(ray.map_pos.y) * MAP_COLUME + INT(ray.map_pos.x)];
    int y = max(0, INT(WINSIZE_Y / 2.0f - ray.height / 2.0f));
    int end = (WINSIZE_Y - y < ray.height ? WINSIZE_Y : y + ray.height);
    while (y < end)
    {
        texture.y = INT((y * 2 - WINSIZE_Y + ray.height)
            * ((TILE_SIZE / 2.0f) / ray.height));
        int endY = min(y + renderScale, end);
        while (y < endY)
        {
            pixel.y = y++;
            RenderPixel(pixel, GetDistanceShadeColor(tile, texture, ray.distance));
        }
    }
}

void RayCasting::RenderCeilingFloor(Ray& ray, int colume)
{
    if (ray.side == 0 && ray.ray_dir.x >= 0)
        ray.floor_wall = { ray.map_pos.x, ray.map_pos.y + ray.wall_x };
    else if (ray.side == 0 && ray.ray_dir.x < 0)
        ray.floor_wall = { ray.map_pos.x + 1, ray.map_pos.y + ray.wall_x };
    else if (ray.side && ray.ray_dir.y >= 0)
        ray.floor_wall = { ray.map_pos.x + ray.wall_x, ray.map_pos.y };
    else if (ray.side && ray.ray_dir.y < 0)
        ray.floor_wall = { ray.map_pos.x + ray.wall_x, ray.map_pos.y + 1 };

    FPOINT pixel = { colume, 0 };
    int y = INT(WINSIZE_Y / 2 + ray.height / 2.0f);
    while (y < WINSIZE_Y)
    {
        float weight = sf_dist[y] / ray.distance;
        ray.c_floor = { weight * ray.floor_wall.x + (1.0f - weight) * Player::GetInstance()->GetCameraPos().x,
                       weight * ray.floor_wall.y + (1.0f - weight) * Player::GetInstance()->GetCameraPos().y };
        FPOINT texture = { INT(ray.c_floor.x * TILE_SIZE) % TILE_SIZE,
            INT(ray.c_floor.y * TILE_SIZE) % TILE_SIZE };
        int endY = min(y + renderScale, WINSIZE_Y);
        while (y < endY)
        {
            pixel.y = y;
            RenderPixel(pixel, GetDistanceShadeColor(8, texture, sf_dist[y]));
            RenderPixel({ pixel.x, WINSIZE_Y - (pixel.y + 1) }, GetDistanceShadeColor(9, texture, sf_dist[y++]));
        }
    }
}

void RayCasting::RenderCeilingFloor(Ray& ray, int colume, COLORREF ceiling, COLORREF floor)
{
    FPOINT pixel = { colume, 0 };
    int y = INT(WINSIZE_Y / 2 + ray.height / 2.0f);
    while (y < WINSIZE_Y)
    {
        int endY = min(y + renderScale, WINSIZE_Y);
        while (y < endY)
        {
            pixel.y = y;
            RenderPixel(pixel, GetDistanceShadeColor(floor, sf_dist[y]));
            RenderPixel({ pixel.x, WINSIZE_Y - (pixel.y + 1) }, GetDistanceShadeColor(ceiling, sf_dist[y]));
        }
    }
}

void RayCasting::RenderPixel(FPOINT pixel, int color)
{
    int pixelPos = (WINSIZE_X * INT(pixel.y) + INT(pixel.x)) * 3;
    //*reinterpret_cast<LPDWORD>(&pixelData[pixelPos]) += color;
    pixelData[pixelPos] = GetRValue(color);
    pixelData[pixelPos + 1] = GetGValue(color);
    pixelData[pixelPos + 2] = GetBValue(color);
}

COLORREF RayCasting::GetDistanceShadeColor(int tile, FPOINT texturePixel, float distance, bool isSide)
{
    float divide = distance / SHADE_VALUE;

    --tile;
    int row = tile / TILE_ROW_SIZE;
    int colume = tile % TILE_ROW_SIZE;

    texturePixel.x += colume * TILE_SIZE;
    texturePixel.y += row * TILE_SIZE;

    COLORREF color = this->mapTile->bmp[INT(texturePixel.y * this->mapTile->bmpWidth + texturePixel.x)];

    if (isSide)
    {
        color = RGB(
            INT(GetRValue(color) * 0.7f),
            INT(GetGValue(color) * 0.7f),
            INT(GetBValue(color) * 0.7f));
    }

    if (divide <= 1.0f)
        return color;
    else
        return RGB(INT(GetRValue(color) / divide),
            INT(GetGValue(color) / divide),
            INT(GetBValue(color) / divide));
}

COLORREF RayCasting::GetDistanceShadeColor(COLORREF color, float distance)
{
    float divide = distance / SHADE_VALUE;

    if (divide <= 1.0f)
        return color;
    else
        return RGB(INT(GetRValue(color) / divide),
            INT(GetGValue(color) / divide),
            INT(GetBValue(color) / divide));
}

int RayCasting::GetRenderScaleBasedOnFPS(void)
{
    if (currentFPS < 15) return SCALE << 3;
    else if (currentFPS < 25) return SCALE << 2;
    else if (currentFPS < 40) return SCALE << 1;
    else return SCALE;
}


Ray::tagRay(FPOINT pos, FPOINT plane, FPOINT cameraDir, float cameraX)
{
    ray_pos = pos;
    map_pos = { FLOAT(INT(pos.x)), FLOAT(INT(pos.y)) };
    ray_dir = { Player::GetInstance()->GetCameraVerDir().x + Player::GetInstance()->GetPlane().x * cameraX, Player::GetInstance()->GetCameraVerDir().y + Player::GetInstance()->GetPlane().y * cameraX };
    delta_dist = { fabs(1.0f / ray_dir.x), fabs(1.0f / ray_dir.y) };
    if (ray_dir.x < 0)
    {
        step = { -1.0f, (ray_dir.y < 0 ? -1.0f : 1.0f) };
        side_dist.x = (ray_pos.x - map_pos.x) * delta_dist.x;
    }
    else
    {
        step = { 1.0f, (ray_dir.y < 0 ? -1.0f : 1.0f) };
        side_dist.x = (map_pos.x + 1.0f - ray_pos.x) * delta_dist.x;
    }
    if (ray_dir.y < 0)
        side_dist.y = (ray_pos.y - map_pos.y) * delta_dist.y;
    else
        side_dist.y = (map_pos.y + 1.0f - ray_pos.y) * delta_dist.y;
}