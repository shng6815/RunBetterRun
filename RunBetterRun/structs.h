#pragma once
#include "config.h"

enum class RoomType
{ NONE, WALL, FLOOR, START };

typedef struct tagRoom
{
	RoomType	roomType;
	DWORD		tilePos;
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

typedef struct tagSprite
{
	FPOINT		pos;
	double		distance;
	Texture*	texture;
} Sprite;


typedef struct tagRay
{
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
