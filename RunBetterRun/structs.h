#pragma once
#include "config.h"

enum class RoomType: BYTE
{ NONE, WALL, FLOOR, START, GOAL };

enum class Direction: BYTE
{
	NORTH, SOUTH, WEST, EAST
};

enum class SpriteType: BYTE
{ NONE, KEY, ITEM, MONSTER };

enum class UIType: BYTE
{
	NONE, PLAYING, GAMEOVER, PAUSE, TITLE
};

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

typedef struct tagObstacle
{
	BOOL			block;
	Direction		dir;
	Texture*		texture;
	AnimationInfo	aniInfo;
} Obstacle;

typedef struct tagRoom
{
	RoomType	roomType;
	DWORD		tilePos;
	Obstacle*	obstacle;
	tagRoom() :roomType(RoomType::FLOOR),
		tilePos(1), obstacle(nullptr) {}
} Room;

typedef struct tagLevel
{
	FPOINT	mosterPos;
	FPOINT	cameraDir;
	FPOINT	cameraPos;
	DWORD	itemCount;
	DWORD	monsterCount;
} Level;


typedef struct tagSprite
{
	SpriteType		type;
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
	Obstacle*	obstacle;
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