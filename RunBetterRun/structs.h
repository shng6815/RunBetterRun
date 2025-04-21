#pragma once
#include "config.h"

enum class RoomType
{ NONE, WALL, FLOOR, START, GOAL };

enum class SpriteType
{ NONE, KEY, ITEM, MONSTER };

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

// 저장용 데이터
typedef struct tagFileHeader
{
	char signature[4];      // 파일타입구분
	int version;            // 파일 포맷 버전
	int mapWidth;          
	int mapHeight;          
	int tileCount;          // 전체 타일 수
	int itemCount;         
	int monsterCount;       
	FPOINT startPos;        

	// 텍스처 정보
	wchar_t texturePath[MAX_PATH];  // 텍스처 경로
	DWORD textureTileSize;          // 타일 크기
	DWORD textureTileRowSize;       // 가로 타일 수
	DWORD textureTileColumnSize;    // 세로 타일 수

	tagFileHeader()
	{
		signature[0] = 'M'; signature[1] = 'P';
		signature[2] = 'D'; signature[3] = 'T';
		version = 1;
		mapWidth = 0;
		mapHeight = 0;
		tileCount = 0;
		itemCount = 0;
		monsterCount = 0;
		startPos = {0.0f,0.0f};
		texturePath[0] = L'\0';
		textureTileSize = 0;
		textureTileRowSize = 0;
		textureTileColumnSize = 0;
	}
}FileHeader;

typedef struct tagItemSaveData
{
	FPOINT pos;             // 위치
	AnimationInfo aniInfo;  // 애니메이션 정보
	int itemType;           // 아이템 타입 - 0: Key
}ItemData;

typedef struct tagMonsterSaveData
{
	FPOINT pos;             // 위치
	AnimationInfo aniInfo;  // 애니메이션 정보
	int monsterType;        // 몬스터 타입 - 0: Tentacle
}MonsterData;