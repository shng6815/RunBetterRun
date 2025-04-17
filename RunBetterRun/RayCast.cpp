#include "RayCast.h"
#include "KeyManager.h"
#include "SpriteManager.h"
#include "MapManager.h"
#include <fstream>
#include "Player.h"

static DWORD WINAPI RaycastThread(LPVOID lpParam) {
    RayCast::ThreadData* data = static_cast<RayCast::ThreadData*>(lpParam);
    while (data && !(*data->exit))
    {
        WaitForSingleObject(*(data->queueMutex), INFINITE);
        if (data->queue && !data->queue->empty())
        {
            POINT colume = data->queue->front();
            data->queue->pop();
            ReleaseMutex(*(data->queueMutex));
            data->pThis->FillScreen(colume.x, colume.y);
            WaitForSingleObject(*(data->threadMutex), INFINITE);
            *(data->done) += 1;
            ReleaseMutex(*(data->threadMutex));
        }
        else
            ReleaseMutex(*(data->queueMutex));
    }
    return 0;
}


HRESULT RayCast::Init(void)
{
    for (int i = 0; i < WINSIZE_X; ++i)
        screenWidthPixelUnitPos[i] = ((2.0f * FLOAT(i) / FLOAT(WINSIZE_X)) - 1.0f);

    for (int i = 0; i < WINSIZE_Y; ++i)
        screenHeightPixelDepths[i] = FLOAT(WINSIZE_Y) / (2.0f * FLOAT(i) - FLOAT(WINSIZE_Y));

    memset(&bmi, 0, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = WINSIZE_X;
    bmi.bmiHeader.biHeight = -WINSIZE_Y;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 24;
    bmi.bmiHeader.biCompression = BI_RGB;

    SpriteManager::GetInstance()->LoadMapTileTexture(TEXT("Image/maptiles.bmp"));
    mapTile = SpriteManager::GetInstance()->GetMapTileTexture();
    SpriteManager::GetInstance()->PutSprite(TEXT("Image/boss.bmp"), { 19, 12 });
    SpriteManager::GetInstance()->PutSprite(TEXT("Image/boss.bmp"), { 16, 12 });

    renderScale = SCALE;
    currentFPS = 60;
    fpsCheckCounter = 0;
    fpsCheckTime = 0.0f;

    threadJobDone = 0;
    threadTermination = FALSE;
    queueMutex = CreateMutex(NULL, FALSE, NULL);
    threadMutex = CreateMutex(NULL, FALSE, NULL);
    colsPerThread = WINSIZE_X / THREAD_NUM;
    for (DWORD i = 0; i < THREAD_NUM; ++i) {
        threadDatas[i] = {
            this,
            &threadTermination,
            &threadJobDone,
            &threadQueue,
            &threadMutex,
            &queueMutex
        };
        threads[i] = CreateThread(NULL, 0, RaycastThread, &threadDatas[i], 0, NULL);
    }

    return S_OK;
}

void RayCast::Release(void)
{
    threadTermination = TRUE;
    WaitForMultipleObjects(THREAD_NUM, threads, TRUE, INFINITE);
    for (auto& thread : threads)
        CloseHandle(thread);

    CloseHandle(threadMutex);
    CloseHandle(queueMutex);

    while (!threadQueue.empty())
        threadQueue.pop();

    mapTile = nullptr;
    SpriteManager::GetInstance()->ClearSprites();
}

void RayCast::Update(void)
{
    float deltaTime = TimerManager::GetInstance()->GetDeltaTime();

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


void RayCast::Render(HDC hdc)
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
        WaitForSingleObject(threadMutex, INFINITE);
        if (threadJobDone == THREAD_NUM)
        {
            SetDIBitsToDevice(hdc, 0, 0, WINSIZE_X, WINSIZE_Y, 0, 0, 0,
                WINSIZE_Y, pixelData, &bmi, DIB_RGB_COLORS);
            threadJobDone = 0;
            print = TRUE;
        }
        ReleaseMutex(threadMutex);
    }
}

void RayCast::FillScreen(DWORD start, DWORD end)
{
    DWORD x = start;
    while (x < end)
    {
        Ray ray = RayCasting(x);
        ray.height = fabs(FLOAT(WINSIZE_Y) / ray.distance);

        DWORD endX = min(x + renderScale, end);
        while (x < endX) {
            screenWidthRayDistance[x] = ray.distance;
            RenderWall(ray, x);
            if (ray.height < WINSIZE_Y)
                RenderCeilingFloor(ray, x);
            ++x;
        }
    }
    RenderSprites(start, end);
}

void RayCast::RenderSprites(DWORD start, DWORD end)
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
            if (fabs(transform.y) < EPSILON)
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

void RayCast::RenderSprite(const Sprite& sprite, POINT renderX, POINT renderY, FPOINT transform)
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
                && transform.y < screenWidthRayDistance[renderX.x]
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

Ray RayCast::RayCasting(int colume)
{
    bool    hit = false;
    bool    nextSide = false;
    Ray     ray(Player::GetInstance()->GetCameraPos(),
        Player::GetInstance()->GetPlane(),
        Player::GetInstance()->GetCameraVerDir(),
        screenWidthPixelUnitPos[colume]);

    while (!hit)
    {
        nextSide = ray.sideDist.x < ray.sideDist.y;
        ray.sideDist.x += nextSide * ray.deltaDist.x;
        ray.mapPos.x += nextSide * ray.step.x;
        ray.sideDist.y += (!nextSide) * ray.deltaDist.y;
        ray.mapPos.y += (!nextSide) * ray.step.y;

        ray.side = !nextSide;

        int x = INT(ray.mapPos.x);
        int y = INT(ray.mapPos.y);

        if (x < 0 || MAP_COLUME <= x || y < 0 || MAP_ROW <= y)
            break;
        int map_index = y * MAP_COLUME + x;
        if (MapManager::GetInstance()->GetMapData()->tiles[MAP_COLUME * y + x].roomType != RoomType::FLOOR)
            hit = true;
    }

    float pos;
    if (ray.side)
    {
        pos = (ray.mapPos.y - Player::GetInstance()->GetCameraPos().y + (1.0f - ray.step.y) / 2.0f);
        ray.distance = fabs(pos / ray.dir.y);
    }
    else
    {
        pos = (ray.mapPos.x - Player::GetInstance()->GetCameraPos().x + (1.0f - ray.step.x) / 2.0f);
        ray.distance = fabs(pos / ray.dir.x);
    }
    return ray;
}


void RayCast::RenderWall(Ray& ray, int colume)
{
    FPOINT pixel = { colume, max(0, WINSIZE_Y / 2.0f - (ray.height / 2.0f)) };

    if (ray.side)
        ray.wallTextureX = ray.pos.x
        + ray.distance
        * ray.dir.x;
    else
        ray.wallTextureX = ray.pos.y
        + ray.distance
        * ray.dir.y;
    ray.wallTextureX -= INT(ray.wallTextureX);

    FPOINT texture = { INT(ray.wallTextureX * TILE_SIZE), 0.0f };
    if ((ray.side == 0 && ray.dir.x > 0.0f)
        || (ray.side == 1 && ray.dir.y < 0.0f))
        texture.x = TILE_SIZE - texture.x - 1.0f;

    int tile = MapManager::GetInstance()->GetMapData()->tiles[INT(ray.mapPos.y) * MAP_COLUME + INT(ray.mapPos.x)].tilePos;
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

void RayCast::RenderCeilingFloor(Ray& ray, int colume)
{
    FPOINT floorTextureStartPos = { 0, 0 };
    if (ray.side == 0 && ray.dir.x >= 0)
        floorTextureStartPos = { ray.mapPos.x, ray.mapPos.y + ray.wallTextureX };
    else if (ray.side == 0 && ray.dir.x < 0)
        floorTextureStartPos = { ray.mapPos.x + 1, ray.mapPos.y + ray.wallTextureX };
    else if (ray.side && ray.dir.y >= 0)
        floorTextureStartPos = { ray.mapPos.x + ray.wallTextureX, ray.mapPos.y };
    else if (ray.side && ray.dir.y < 0)
        floorTextureStartPos = { ray.mapPos.x + ray.wallTextureX, ray.mapPos.y + 1 };

    FPOINT pixel = { colume, 0 };
    int y = INT(WINSIZE_Y / 2 + ray.height / 2.0f);
    while (y < WINSIZE_Y)
    {
        float weight = screenHeightPixelDepths[y] / ray.distance;
        FPOINT currentFloor = { weight * floorTextureStartPos.x + (1.0f - weight) * Player::GetInstance()->GetCameraPos().x,
                       weight * floorTextureStartPos.y + (1.0f - weight) * Player::GetInstance()->GetCameraPos().y };
        FPOINT texture = { INT(currentFloor.x * TILE_SIZE) % TILE_SIZE,
            INT(currentFloor.y * TILE_SIZE) % TILE_SIZE };
        int endY = min(y + renderScale, WINSIZE_Y);
        while (y < endY)
        {
            pixel.y = y;
            RenderPixel(pixel, GetDistanceShadeColor(8, texture, screenHeightPixelDepths[y]));
            RenderPixel({ pixel.x, WINSIZE_Y - (pixel.y + 1) }, GetDistanceShadeColor(9, texture, screenHeightPixelDepths[y++]));
        }
    }
}

void RayCast::RenderCeilingFloor(Ray& ray, int colume, COLORREF ceiling, COLORREF floor)
{
    FPOINT pixel = { colume, 0 };
    int y = INT(WINSIZE_Y / 2 + ray.height / 2.0f);
    while (y < WINSIZE_Y)
    {
        int endY = min(y + renderScale, WINSIZE_Y);
        while (y < endY)
        {
            pixel.y = y;
            RenderPixel(pixel, GetDistanceShadeColor(floor, screenHeightPixelDepths[y]));
            RenderPixel({ pixel.x, WINSIZE_Y - (pixel.y + 1) }, GetDistanceShadeColor(ceiling, screenHeightPixelDepths[y]));
        }
    }
}

void RayCast::RenderPixel(FPOINT pixel, int color)
{
    int pixelPos = (WINSIZE_X * INT(pixel.y) + INT(pixel.x)) * 3;
    //*reinterpret_cast<LPDWORD>(&pixelData[pixelPos]) += color;
    pixelData[pixelPos] = GetRValue(color);
    pixelData[pixelPos + 1] = GetGValue(color);
    pixelData[pixelPos + 2] = GetBValue(color);
}

COLORREF RayCast::GetDistanceShadeColor(int tile, FPOINT texturePixel, float distance, bool isSide)
{
    float divide = distance / SHADE_VALUE;

    //--tile;
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

COLORREF RayCast::GetDistanceShadeColor(COLORREF color, float distance)
{
    float divide = distance / SHADE_VALUE;

    if (divide <= 1.0f)
        return color;
    else
        return RGB(INT(GetRValue(color) / divide),
            INT(GetGValue(color) / divide),
            INT(GetBValue(color) / divide));
}

int RayCast::GetRenderScaleBasedOnFPS(void)
{
    if (currentFPS < 15) return SCALE << 3;
    else if (currentFPS < 25) return SCALE << 2;
    else if (currentFPS < 40) return SCALE << 1;
    else return SCALE;
}


Ray::tagRay(FPOINT pos, FPOINT plane, FPOINT cameraDir, float cameraX)
{
    this->pos = pos;
    mapPos = { FLOAT(INT(pos.x)), FLOAT(INT(pos.y)) };
    dir = { Player::GetInstance()->GetCameraVerDir().x + Player::GetInstance()->GetPlane().x * cameraX,
        Player::GetInstance()->GetCameraVerDir().y + Player::GetInstance()->GetPlane().y * cameraX };
    deltaDist = { fabs(1.0f / dir.x), fabs(1.0f / dir.y) };
    if (dir.x < 0)
    {
        step = { -1.0f, (dir.y < 0 ? -1.0f : 1.0f) };
        sideDist.x = (pos.x - mapPos.x) * deltaDist.x;
    }
    else
    {
        step = { 1.0f, (dir.y < 0 ? -1.0f : 1.0f) };
        sideDist.x = (mapPos.x + 1.0f - pos.x) * deltaDist.x;
    }
    if (dir.y < 0)
        sideDist.y = (pos.y - mapPos.y) * deltaDist.y;
    else
        sideDist.y = (mapPos.y + 1.0f - pos.y) * deltaDist.y;
}