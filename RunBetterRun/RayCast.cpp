#include "RayCast.h"
#include "KeyManager.h"
#include "SpriteManager.h"
#include "MapManager.h"
#include "MonsterManager.h"
#include "Player.h"
#include "TextureManager.h"
#include <list>
#include <algorithm>


static DWORD WINAPI RaycastThread(LPVOID lpParam) {
	RayCast::ThreadData* data = static_cast<RayCast::ThreadData*>(lpParam);
	while(data && !(*data->exit))
	{
		WaitForSingleObject(*(data->queueMutex),INFINITE);
		if(data->queue && !data->queue->empty())
		{
			POINT column = data->queue->front();
			data->queue->pop();
			ReleaseMutex(*(data->queueMutex));
			data->pThis->FillScreen(column.x,column.y);
			WaitForSingleObject(*(data->threadMutex),INFINITE);
			*(data->done) += 1;
			ReleaseMutex(*(data->threadMutex));
		} else
			ReleaseMutex(*(data->queueMutex));
	}
	return 0;
}


HRESULT RayCast::Init(void)
{
	for(int i = 0; i < WINSIZE_X; ++i)
		screenWidthPixelUnitPos[i] = ((2.0f * FLOAT(i) / FLOAT(WINSIZE_X)) - 1.0f);

	for(int i = 0; i < WINSIZE_Y; ++i)
		screenHeightPixelDepths[i] = FLOAT(WINSIZE_Y) / (2.0f * FLOAT(i) - FLOAT(WINSIZE_Y));

	memset(&bmi,0,sizeof(bmi));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = WINSIZE_X;
	bmi.bmiHeader.biHeight = -WINSIZE_Y;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 24;
	bmi.bmiHeader.biCompression = BI_RGB;

	renderScale = SCALE;
	currentFPS = 60;
	fpsCheckCounter = 0;
	fpsCheckTime = 0.0f;

	if(threadTermination)
	{
		threadJobDone = 0;
		threadTermination = FALSE;
		queueMutex = CreateMutex(NULL,FALSE,NULL);
		threadMutex = CreateMutex(NULL,FALSE,NULL);
		colsPerThread = WINSIZE_X / THREAD_NUM;
		threadDatas = {
			this,
			&threadTermination,
			&threadJobDone,
			&threadQueue,
			&threadMutex,
			&queueMutex
		};
		for(DWORD i = 0; i < THREAD_NUM; ++i)
			threads[i] = CreateThread(NULL,0,RaycastThread,&threadDatas,0,NULL);
	}

	return S_OK;
}

void RayCast::Release(void)
{
	threadTermination = TRUE;
	WaitForMultipleObjects(THREAD_NUM,threads,TRUE,INFINITE);
	for(auto& thread : threads)
		CloseHandle(thread);

	CloseHandle(threadMutex);
	CloseHandle(queueMutex);

	while(!threadQueue.empty())
		threadQueue.pop();

	mapTile = nullptr;
}

void RayCast::Update(void)
{
	fpsCheckCounter++;
	fpsCheckTime += TimerManager::GetInstance()->GetDeltaTime();;

	if(fpsCheckTime >= 1.0f) {
		currentFPS = fpsCheckCounter;
		fpsCheckCounter = 0;
		fpsCheckTime = 0.0f;
		renderScale = GetRenderScaleBasedOnFPS();
	}
}


void RayCast::Render(HDC hdc)
{
	WaitForSingleObject(queueMutex,INFINITE);
	for(int i = 0; i < THREAD_NUM - 1; ++i)
		threadQueue.push({i * colsPerThread,(i + 1) * colsPerThread});
	threadQueue.push({(THREAD_NUM - 1) * colsPerThread,WINSIZE_X});
	ReleaseMutex(queueMutex);

	BOOL print = FALSE;
	while(!print)
	{
		WaitForSingleObject(threadMutex,INFINITE);
		if(threadJobDone == THREAD_NUM)
		{
			SetDIBitsToDevice(hdc,0,0,WINSIZE_X,WINSIZE_Y,0,0,0,
				WINSIZE_Y,pixelData,&bmi,DIB_RGB_COLORS);
			threadJobDone = 0;
			print = TRUE;
		}
		ReleaseMutex(threadMutex);
	}
}

void RayCast::FillScreen(DWORD start,DWORD end)
{
	list<Obstacle*> obstacles;
	MapData* md = MapManager::GetInstance()->GetMapData();
	DWORD x = start;
	while(x < end)
	{
		Ray ray = RayCastingWall(x,obstacles);
		DWORD endX = min(x + renderScale,end);
		while(x < endX) {
			screenWidthRayDistance[x] = ray.distance;
			if(ray.mapPos.x < 0 || md->width <= ray.mapPos.x || ray.mapPos.y < 0 || md->height <= ray.mapPos.y)
				RenderOutOfWorld(ray,x);
			else
				RenderWall(ray,x);
			if(ray.height < WINSIZE_Y)
				RenderCeilingFloor(ray,x);
			++x;
		}
	}

	obstacles.sort([](Obstacle* a,Obstacle* b) -> BOOL {
		return a->distance > b->distance;
	});

	RenderObjects(start,end,obstacles);
}

void RayCast::RenderObjects(DWORD start,DWORD end,list<Obstacle*>& obstacles)
{
	auto sprites = SpriteManager::GetInstance()->GetSprites();
	auto sprite = sprites.begin();
	auto obstacle = obstacles.begin();
	float inverseDeterminant = 1.0f
		/ ((Player::GetInstance()->GetPlane().x
		* Player::GetInstance()->GetCameraVerDir().y)
		- (Player::GetInstance()->GetPlane().y
		* Player::GetInstance()->GetCameraVerDir().x));

	while(obstacle != obstacles.end() || sprite != sprites.end())
	{
		if(obstacle == obstacles.end() && sprite != sprites.end())
		{
			RenderSprites(start,end,*sprite,inverseDeterminant);
			++sprite;
		} else if(obstacle != obstacles.end() && sprite == sprites.end())
		{
			RenderObstacles(start,end,*obstacle);
			++obstacle;
		} else
		{
			if((*obstacle)->distance >= (*sprite)->distance)
			{
				RenderObstacles(start,end,*obstacle);
				++obstacle;
			} else
			{
				RenderSprites(start,end,*sprite,inverseDeterminant);
				++sprite;
			}
		}
	}
}

void RayCast::RenderSprites(DWORD start,DWORD end,const Sprite* sprite,float inverseDeterminant)
{
	if(sprite->distance <= 0.1f)
		return;

	FPOINT pos = {sprite->pos.x - Player::GetInstance()->GetCameraPos().x,
		sprite->pos.y - Player::GetInstance()->GetCameraPos().y};
	FPOINT transform = {inverseDeterminant
		* (Player::GetInstance()->GetCameraVerDir().y * pos.x
		- Player::GetInstance()->GetCameraVerDir().x * pos.y),
		inverseDeterminant
		* (-Player::GetInstance()->GetPlane().y * pos.x
		+ Player::GetInstance()->GetPlane().x * pos.y)};

	if(fabs(transform.y) < EPSILON)
		return;

	int screen = INT((WINSIZE_X / 2) * (1.0f + transform.x / transform.y));
	float size = fabs(WINSIZE_Y / transform.y);
	POINT renderX = {INT(max(0,-size / 2.0f + screen)),
		INT(max(0,size / 2.0f + screen))};

	if(renderX.y < start || end <= renderX.x)
		return;

	if(renderX.x < start && start < renderX.y)
		renderX.x = start;

	if(start <= renderX.x && end < renderX.y)
		renderX.y = end;

	POINT renderY = {INT(max(0,-size / 2.0f + WINSIZE_Y / 2.0f)),
		INT(max(0,size / 2.0f + WINSIZE_Y / 2.0f))};

	if(WINSIZE_Y < renderY.y)
		renderY.y = WINSIZE_Y;

	RenderSprite(sprite,renderX,renderY,transform);
}

void RayCast::RenderSprite(const Sprite* sprite,POINT renderX,POINT renderY,FPOINT transform)
{
	float renderYOrg = renderY.x;
	int screen = INT((WINSIZE_X / 2) * (1.0f + transform.x / transform.y));
	float size = fabs(WINSIZE_Y / transform.y);

	while(renderX.x < renderX.y)
	{
		POINT texturePos = {INT(256 * ((INT(renderX.x) - (-size / 2.0f + screen)))
						 * sprite->aniInfo.frameSize.x / size) / 256,0};

		texturePos.x += sprite->aniInfo.currentFrame.x * sprite->aniInfo.frameSize.x;

		DWORD endX = min(renderX.x + renderScale,renderX.y);
		while(renderX.x < endX)
		{
			if(transform.y > 0
				&& transform.y < screenWidthRayDistance[renderX.x]
				&& texturePos.x < sprite->texture->bmpWidth)
			{
				renderY.x = renderYOrg;
				while(renderY.x < renderY.y)
				{
					LONG fact = (INT(renderY.x) * 256.0f) - (WINSIZE_Y * 128.0f) + (size * 128.0f);
					texturePos.y = ((fact * sprite->aniInfo.frameSize.y) / size) / 256.0f;

					texturePos.y += sprite->aniInfo.currentFrame.y * sprite->aniInfo.frameSize.y;

					if(texturePos.y < sprite->texture->bmpHeight)
					{
						COLORREF color =
							sprite->texture->bmp[texturePos.y * sprite->texture->bmpWidth + texturePos.x];

						DWORD endY = min(renderY.x + renderScale,renderY.y);
						while(renderY.x < endY)
						{
							FPOINT pixel = {renderX.x,renderY.x};
							if(color != 0xFF00FF)
								RenderPixel(pixel,GetDistanceShadeColor(color,sprite->distance,SPRITE_SHADE_VALUE));
							++renderY.x;
						}
					} else
						++renderY.x;
				}
			}
			++renderX.x;
		}
	}
}

Obstacle* RayCast::HitObstacle(Ray& ray)
{

	MapData* md = MapManager::GetInstance()->GetMapData();

	if(ray.mapCoordinate >= md->width * md->height)
		return nullptr;

	if(ray.side && ray.dir.y < 0)
	{
		if(md->tiles[ray.mapCoordinate].obstacle
			&& md->tiles[ray.mapCoordinate].obstacle->dir == Direction::NORTH)
			return md->tiles[ray.mapCoordinate].obstacle;
		else if(ray.mapPos.y + 1 < md->height &&
			(md->tiles[ray.mapCoordinate + md->width].obstacle
			&& md->tiles[ray.mapCoordinate + md->width].obstacle->dir == Direction::SOUTH))
			return md->tiles[ray.mapCoordinate + md->width].obstacle;
	} else if(ray.side && ray.dir.y > 0)
	{
		if(md->tiles[ray.mapCoordinate].obstacle
			&& md->tiles[ray.mapCoordinate].obstacle->dir == Direction::SOUTH)
			return md->tiles[ray.mapCoordinate].obstacle;
		else if(ray.mapPos.y > 0 &&
			(md->tiles[ray.mapCoordinate - md->width].obstacle
			&& md->tiles[ray.mapCoordinate - md->width].obstacle->dir == Direction::NORTH))
			return md->tiles[ray.mapCoordinate - md->width].obstacle;
	} else if(!ray.side && ray.dir.x < 0)
	{
		if(md->tiles[ray.mapCoordinate].obstacle
			&& md->tiles[ray.mapCoordinate].obstacle->dir == Direction::WEST)
			return md->tiles[ray.mapCoordinate].obstacle;
		else if(ray.mapPos.x + 1 < md->width &&
			(md->tiles[ray.mapCoordinate + 1].obstacle
			&& md->tiles[ray.mapCoordinate + 1].obstacle->dir ==  Direction::EAST))
			return md->tiles[ray.mapCoordinate + 1].obstacle;
	} else if(!ray.side && ray.dir.x > 0)
	{
		if(md->tiles[ray.mapCoordinate].obstacle
			&& md->tiles[ray.mapCoordinate].obstacle->dir == Direction::EAST)
			return md->tiles[ray.mapCoordinate].obstacle;
		else if(ray.mapPos.x > 0 &&
			(md->tiles[ray.mapCoordinate - 1].obstacle
			&& md->tiles[ray.mapCoordinate - 1].obstacle->dir == Direction::WEST))
			return md->tiles[ray.mapCoordinate - 1].obstacle;
	}
	return nullptr;
}

Ray RayCast::RayCastingWall(int column,list<Obstacle*>& obstacles)
{
	bool    hit = false;
	bool    nextSide = false;
	Ray     ray(Player::GetInstance()->GetCameraPos(),
				Player::GetInstance()->GetPlane(),
				Player::GetInstance()->GetCameraVerDir(),
				screenWidthPixelUnitPos[column]);
	MapData* md = MapManager::GetInstance()->GetMapData();

	while(!hit)
	{
		nextSide = ray.sideDist.x < ray.sideDist.y;
		ray.sideDist.x += nextSide * ray.deltaDist.x;
		ray.mapPos.x += nextSide * ray.step.x;
		ray.sideDist.y += (!nextSide) * ray.deltaDist.y;
		ray.mapPos.y += (!nextSide) * ray.step.y;

		ray.side = !nextSide;

		ray.mapCoordinate = ray.mapPos.y * md->width + ray.mapPos.x;
		if(ray.mapCoordinate >= md->width * md->height)
			break;

		if(md->tiles[ray.mapCoordinate].roomType == RoomType::WALL)
			hit = TRUE;

		Obstacle* obstacle = HitObstacle(ray);
		if(obstacle
			&& find(obstacles.begin(),obstacles.end(),obstacle) == obstacles.end())
			obstacles.push_back(obstacle);
	}

	float pos;
	if(ray.side)
	{
		pos = (ray.mapPos.y - ray.pos.y + (1.0f - ray.step.y) / 2.0f);
		ray.distance = fabs(pos / ray.dir.y);
	} else
	{
		pos = (ray.mapPos.x - ray.pos.x + (1.0f - ray.step.x) / 2.0f);
		ray.distance = fabs(pos / ray.dir.x);
	}

	ray.height = fabs(FLOAT(WINSIZE_Y) / ray.distance);
	return ray;
}

Ray RayCast::RayCastingObstacle(int column,Obstacle*& obstacle)
{
	bool    hit = false;
	bool    nextSide = false;
	Ray     ray(Player::GetInstance()->GetCameraPos(),
				Player::GetInstance()->GetPlane(),
				Player::GetInstance()->GetCameraVerDir(),
				screenWidthPixelUnitPos[column]);
	MapData* md = MapManager::GetInstance()->GetMapData();

	while(!ray.obstacle)
	{
		nextSide = ray.sideDist.x < ray.sideDist.y;
		ray.sideDist.x += nextSide * ray.deltaDist.x;
		ray.mapPos.x += nextSide * ray.step.x;
		ray.sideDist.y += (!nextSide) * ray.deltaDist.y;
		ray.mapPos.y += (!nextSide) * ray.step.y;
		ray.side = !nextSide;

		ray.mapCoordinate = ray.mapPos.y * md->width + ray.mapPos.x;
		if(ray.mapCoordinate >= md->width * md->height)
			return ray;

		if(HitObstacle(ray) == obstacle)
			ray.obstacle = obstacle;
	}
	float pos;

	if(ray.side)
	{
		pos = (ray.mapPos.y - ray.pos.y + (1.0f - ray.step.y) / 2.0f);
		ray.distance = fabs(pos / ray.dir.y);
	} else
	{
		pos = (ray.mapPos.x - ray.pos.x + (1.0f - ray.step.x) / 2.0f);
		ray.distance = fabs(pos / ray.dir.x);
	}

	ray.height = fabs(FLOAT(WINSIZE_Y) / ray.distance);
	return ray;
}


void RayCast::RenderWall(Ray& ray,int column)
{
	FPOINT pixel = {column,max(0,WINSIZE_Y / 2.0f - (ray.height / 2.0f))};

	MapData* md = MapManager::GetInstance()->GetMapData();

	if(ray.side)
		ray.wallTextureX = ray.pos.x
		+ ray.distance
		* ray.dir.x;
	else
		ray.wallTextureX = ray.pos.y
		+ ray.distance
		* ray.dir.y;
	ray.wallTextureX -= INT(ray.wallTextureX);

	FPOINT texture = {INT(ray.wallTextureX * md->textureTileSize),0.0f};
	if((ray.side == 0 && ray.dir.x > 0.0f)
		|| (ray.side == 1 && ray.dir.y < 0.0f))
		texture.x = md->textureTileSize - texture.x - 1.0f;

	int tile = md->tiles[ray.mapPos.y * md->width + ray.mapPos.x].tilePos;
	int y = max(0,WINSIZE_Y / 2 - INT(ray.height / 2.0f));
	int end = min(WINSIZE_Y,y + ray.height);
	while(y < end)
	{
		texture.y = INT((y * 2.0f - WINSIZE_Y + ray.height)
			* ((md->textureTileSize / 2.0f) / ray.height));
		int endY = min(y + renderScale,end);
		while(y < endY)
		{
			pixel.y = y++;
			RenderPixel(pixel,GetDistanceShadeColor(tile,texture,ray.distance));
		}
	}
}

void RayCast::RenderOutOfWorld(Ray& ray,int column)
{
	FPOINT pixel = {column,max(0,WINSIZE_Y / 2.0f - (ray.height / 2.0f))};

	COLORREF black = 0;
	int y = max(0, WINSIZE_Y / 2 - INT(ray.height / 2.0f));
	int end = min(WINSIZE_Y, y + ray.height);
	while(y < end)
	{
		pixel.y = y++;
		RenderPixel(pixel,black);
	}
}

void RayCast::RenderObstacles(DWORD start,DWORD end,Obstacle*& obstacle)
{
	DWORD x = start;
	while(x < end)
	{
		Ray ray = RayCastingObstacle(x,obstacle);
		if(ray.obstacle)
		{
			DWORD endX = min(x + renderScale,end);
			while(x < endX) {
				if(screenWidthRayDistance[x] > ray.distance)
					RenderObstacle(ray,x);
				++x;
			}
		} else
			++x;
	}
}

void RayCast::RenderObstacle(Ray& ray,int column)
{
	FPOINT pixel = {column,max(0,WINSIZE_Y / 2.0f - (ray.height / 2.0f))};

	if(ray.side)
		ray.wallTextureX = ray.pos.x
		+ ray.distance
		* ray.dir.x;
	else
		ray.wallTextureX = ray.pos.y
		+ ray.distance
		* ray.dir.y;
	ray.wallTextureX -= INT(ray.wallTextureX);

	Obstacle* obstacle = ray.obstacle;
	Texture* texture = obstacle->texture;
	FPOINT texturePos= {INT(ray.wallTextureX * obstacle->aniInfo.frameSize.x),0.0f};

	if((ray.side == 0 && ray.dir.x > 0.0f)
		|| (ray.side == 1 && ray.dir.y < 0.0f))
		texturePos.x = obstacle->aniInfo.frameSize.x - texturePos.x - 1.0f;

	texturePos.x += obstacle->aniInfo.frameSize.x * obstacle->aniInfo.currentFrame.x;
	int y = max(0,WINSIZE_Y / 2 - INT(ray.height / 2.0f));
	int end = min(WINSIZE_Y,y + ray.height);
	while(y < end)
	{
		texturePos.y = INT((y * 2.0f - WINSIZE_Y + ray.height)
			* ((obstacle->aniInfo.frameSize.y / 2.0f) / ray.height));
		texturePos.y += obstacle->aniInfo.frameSize.y * obstacle->aniInfo.currentFrame.y;
		COLORREF color = texture->bmp[texturePos.y * texture->bmpWidth + texturePos.x];
		if(color != 0x00FF00FF)
		{
			int endY = min(y + renderScale,end);
			while(y < endY)
			{
				pixel.y = y++;
				RenderPixel(pixel,GetDistanceShadeColor(color,ray.distance, SHADE_VALUE));
			}
		} else
			++y;
	}
}

void RayCast::RenderCeilingFloor(Ray& ray,int column)
{
	FPOINT floorTextureStartPos = {0,0};
	if(ray.side == 0 && ray.dir.x >= 0)
		floorTextureStartPos = {FLOAT(ray.mapPos.x),FLOAT(ray.mapPos.y) + ray.wallTextureX};
	else if(ray.side == 0 && ray.dir.x < 0)
		floorTextureStartPos = {FLOAT(ray.mapPos.x + 1),FLOAT(ray.mapPos.y) + ray.wallTextureX};
	else if(ray.side && ray.dir.y >= 0)
		floorTextureStartPos = {FLOAT(ray.mapPos.x) + ray.wallTextureX,FLOAT(ray.mapPos.y)};
	else if(ray.side && ray.dir.y < 0)
		floorTextureStartPos = {FLOAT(ray.mapPos.x) + ray.wallTextureX,FLOAT(ray.mapPos.y + 1)};

	FPOINT pixel = {column,0};
	int y = WINSIZE_Y / 2 + INT(ray.height / 2.0f);
	while(y < WINSIZE_Y)
	{
		float weight = screenHeightPixelDepths[y] / ray.distance;
		FPOINT currentFloor = {weight * floorTextureStartPos.x + (1.0f - weight) * Player::GetInstance()->GetCameraPos().x,
			weight * floorTextureStartPos.y + (1.0f - weight) * Player::GetInstance()->GetCameraPos().y};
		MapData* md = MapManager::GetInstance()->GetMapData();
		DWORD mapIndex = INT(currentFloor.y) * md->width + INT(currentFloor.x);
		mapIndex = min(md->width * md->height - 1,mapIndex);
        DWORD tileCeilingIndex = md->tiles[mapIndex].tilePos;
		if(tileCeilingIndex < md->textureTileRowSize * 2)
			tileCeilingIndex = 17;
		DWORD tileFloorIndex = min(tileCeilingIndex + md->textureTileRowSize, md->textureTileRowSize * md->textureTileColumnSize - 3);
        FPOINT texture = { INT(currentFloor.x * md->textureTileSize) % md->textureTileSize,
            INT(max(currentFloor.y, 0) * md->textureTileSize) % md->textureTileSize };
        int endY = min(y + renderScale, WINSIZE_Y);
        while (y < endY)
        {
            pixel.y = y;
            RenderPixel(pixel, GetDistanceShadeColor(tileFloorIndex, texture, screenHeightPixelDepths[y]));
            RenderPixel({ pixel.x, WINSIZE_Y - (pixel.y + 1) }, GetDistanceShadeColor(tileCeilingIndex, texture, screenHeightPixelDepths[y++]));
        }
    }
}

void RayCast::RenderCeilingFloor(Ray& ray,int column,COLORREF ceiling,COLORREF floor)
{
	FPOINT pixel = {column,0};
	int y = INT(WINSIZE_Y / 2 + ray.height / 2.0f);
	while(y < WINSIZE_Y)
	{
		int endY = min(y + renderScale,WINSIZE_Y);
		while(y < endY)
		{
			pixel.y = y;
			RenderPixel(pixel,GetDistanceShadeColor(floor,screenHeightPixelDepths[y],SHADE_VALUE));
			RenderPixel({pixel.x,WINSIZE_Y - (pixel.y + 1)},GetDistanceShadeColor(ceiling,screenHeightPixelDepths[y],SHADE_VALUE));
		}
	}
}

void RayCast::RenderPixel(FPOINT pixel,int color)
{
	int pixelPos = (WINSIZE_X * INT(pixel.y) + INT(pixel.x)) * 3;
	pixelData[pixelPos] = GetRValue(color);
	pixelData[pixelPos + 1] = GetGValue(color);
	pixelData[pixelPos + 2] = GetBValue(color);
}

COLORREF RayCast::GetDistanceShadeColor(int tile,FPOINT texturePixel,float distance)
{
	float divide = distance / SHADE_VALUE;
	MapData* md = MapManager::GetInstance()->GetMapData();
	int row = tile / md->textureTileRowSize;
	int column = tile % md->textureTileRowSize;

	texturePixel.x += column * md->textureTileSize;
	texturePixel.y += row * md->textureTileSize;

	COLORREF color = md->texture->bmp[INT(texturePixel.y * md->texture->bmpWidth + texturePixel.x)];

    if (divide <= 1.0f || column == md->textureTileRowSize - 1)
        return color;
    else
        return RGB(INT(GetRValue(color) / divide),
            INT(GetGValue(color) / divide),
            INT(GetBValue(color) / divide));
}

COLORREF RayCast::GetDistanceShadeColor(COLORREF color,float distance,float shade)
{
	float divide = distance / shade;

	if(divide <= 1.0f)
		return color;
	else
		return RGB(INT(GetRValue(color) / divide),
			INT(GetGValue(color) / divide),
			INT(GetBValue(color) / divide));
}

int RayCast::GetRenderScaleBasedOnFPS(void)
{
	if(currentFPS < 15) return SCALE << 3;
	else if(currentFPS < 25) return SCALE << 2;
	else if(currentFPS < 40) return SCALE << 1;
	else return SCALE;
}


Ray::tagRay(FPOINT pos,FPOINT plane,FPOINT cameraDir,float cameraX)
{
	this->pos = pos;
	obstacle = nullptr;
	mapPos = {INT(pos.x),INT(pos.y)};
	dir = {cameraDir.x + plane.x * cameraX,cameraDir.y + plane.y * cameraX};
	deltaDist = {fabs(1.0f / dir.x),fabs(1.0f / dir.y)};
	if(dir.x < 0)
	{
		step = {-1,(dir.y < 0 ? -1 : 1)};
		sideDist.x = (pos.x - mapPos.x) * deltaDist.x;
	} else
	{
		step = {1,(dir.y < 0 ? -1 : 1)};
		sideDist.x = (mapPos.x + 1.0f - pos.x) * deltaDist.x;
	}
	if(dir.y < 0)
		sideDist.y = (pos.y - mapPos.y) * deltaDist.y;
	else
		sideDist.y = (mapPos.y + 1.0f - pos.y) * deltaDist.y;
}