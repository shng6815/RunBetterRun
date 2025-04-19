#pragma once
#include "config.h"

enum class RoomType
{ NONE, WALL, FLOOR, START, GOAL };

enum class UIType
{
	NONE, PLAYING, GAMEOVER, PAUSE, TITLE
};

typedef struct tagRoom
{
	RoomType	roomType;
	DWORD		tilePos;
	tagRoom() :roomType(RoomType::FLOOR), tilePos(1) {}
} Room;

typedef struct tagLevel
{
	FPOINT	mosterPos;
	FPOINT	cameraDir;
	FPOINT	cameraPos;
	DWORD	itemCount;
	DWORD	monsterCount;
} Level;

typedef struct tagTexture
{
	vector<COLORREF>	bmp;
	DWORD				bmpWidth;
	DWORD				bmpHeight;
} Texture;

typedef struct tagAnimationInfo
{
	FLOAT	frameTime;
	FLOAT	currentTime;
	POINT	frameSize;
	POINT	maxFrame;
	POINT	currentFrame;

} AnimationInfo;

typedef struct tagSprite
{
	FPOINT			pos;
	float			distance;
	Texture*		texture;
	AnimationInfo	aniInfo;
} Sprite;

typedef struct tagRay
{
	float		distance;
	int			side;
	int			height;
	FPOINT		pos;
	FPOINT		dir;
	FPOINT		mapPos;
	FPOINT		sideDist;
	FPOINT		deltaDist;
	FPOINT		step;
	float		wallTextureX;

	tagRay(FPOINT pos, FPOINT plane, FPOINT cameraDir, float cameraX);
} Ray;

typedef struct tagMapData
{
	int width;
	int height;
	vector<Room> tiles;
	Texture* texture;
	DWORD textureTileSize ;
	DWORD textureTileRowSize;
	DWORD textureTileColumnSize;
} MapData;