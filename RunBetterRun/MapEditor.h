#pragma once
#include "GameObject.h"
#include "structs.h"

#define MAP_EDITOR_WIDTH  60       
#define MAP_EDITOR_HEIGHT 60     

#define VISIBLE_MAP_WIDTH  60       
#define VISIBLE_MAP_HEIGHT 60

#define TILE_SIZE 16               
#define SAMPLE_TILE_X 11            
#define SAMPLE_TILE_Y 11            

enum class EditorMode
{
	TILE,
	START,
	ITEM,
	MONSTER
};

typedef struct EditorTile
{
	RECT rc;
	int tileIndex;
	RoomType type;
} editTile;

class Image;
class Button;

class MapEditor: public GameObject
{
private:
	EditorMode mode;
	POINT selectedTile;
	RoomType selectedRoomType;

	Image* sampleTileImage;
	RECT sampleArea;
	RECT mapArea;

	editTile mapTiles[MAP_EDITOR_HEIGHT][MAP_EDITOR_WIDTH];
	vector<Sprite> editorSprites;
	vector<Button*> buttons;
	POINT mousePos;
	FPOINT startPosition; 

public:
	virtual HRESULT Init() override;
	virtual void Release() override;
	virtual void Update() override;
	virtual void Render(HDC hdc) override;

	void InitTiles();
	void InitButtons();

	// 기본 기능
	void TileSelect();
	void MapEdit();
	void Shortcut();

	void RenderTiles(HDC hdc);
	void RenderSampleTiles(HDC hdc);
	void RenderSprites(HDC hdc);

	void ChangeMode(EditorMode newMode);
	void PlaceTile(int x,int y);
	void PlaceStartPoint(int x,int y);
	void PlaceItem(int x,int y);
	void PlaceMonster(int x,int y);

	// 스프라이트 관리
	void AddSprite(FPOINT position,Texture* texture,SpriteType type);
	void RemoveSprite(int x,int y);
	int FindSprite(int x,int y);

	void SaveMap();
	void LoadMap();
	void ClearMap();

private:
	void PrepareDataForSave();
	void LoadFromDataManager();
};