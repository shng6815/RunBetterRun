#include "DataManager.h"
#include "TextureManager.h"

HRESULT DataManager::Init()
{
	header = FileHeader();
	tiles.clear();
	items.clear();
	monsters.clear();
	InitDefaultAnimations();

	return S_OK;
}

void DataManager::Release()
{
	tiles.clear();
	items.clear();
	monsters.clear();
}

void DataManager::InitDefaultAnimations()
{
	// 아이템 기본 애니메이션
	itemDefaultAnims[0] = {0.1f,0.1f,{250,250},{20,1},{0,0}}; // Key
	itemDefaultAnims[1] = {0.0f,0.0f,{250,250},{1,1},{0,0}};  // Phone
	itemDefaultAnims[2] = {0.0f,0.0f,{250,250},{1,1},{0,0}};  // Insight
	itemDefaultAnims[3] = {0.0f,0.0f,{250,250},{1,1},{0,0}};  // Stun
	itemDefaultAnims[4] = {0.1f,0.1f,{250,250},{20,1},{0,0}}; // Poo
	itemDefaultAnims[5] = {0.1f,0.1f,{250,250},{20,1},{0,0}}; // 소화기
	itemDefaultAnims[6] = {0.1f,0.1f,{250,250},{20,1},{0,0}}; // Pipe
	itemDefaultAnims[7] = {0.1f,0.1f,{250,250},{20,1},{0,0}}; // 드럼통
	itemDefaultAnims[11] = {0.1f,0.1f,{250,250},{20,1},{0,0}}; // Trash

	// 장애물 기본 애니메이션
	obstacleDefaultAnims[8] = {0.0f,0.0f,{128,128},{8,1},{0,0}}; // Pile
	obstacleDefaultAnims[9] = {0.2f,0.2f,{128,128},{8,1},{0,0}}; // Elevator
	obstacleDefaultAnims[12] = {0.2f,0.2f,{128,128},{8,1},{0,0}}; // Final Elevator
}

void DataManager::ClearAllData()
{
	header = FileHeader();
	tiles.clear();
	items.clear();
	monsters.clear();
	obstacles.clear();
}

void DataManager::SetMapData(const vector<Room>& mapTiles,int width,int height)
{
	tiles = mapTiles;
	header.mapWidth = width;
	header.mapHeight = height;
	header.tileCount = width * height;

	// 시작 위치 찾기
	for(int y = 0; y < height; y++)
	{
		for(int x = 0; x < width; x++)
		{
			int index = y * width + x;
			if(tiles[index].roomType == RoomType::START)
			{
				header.startPos.x = static_cast<float>(x) + 0.5f;
				header.startPos.y = static_cast<float>(y) + 0.5f;
				break;
			}
		}
	}
}

void DataManager::SetTextureInfo(LPCWCH texturePath,DWORD tileSize,DWORD rowSize,DWORD columnSize)
{
	wcscpy_s(header.texturePath,texturePath);
	header.textureTileSize = tileSize;
	header.textureTileRowSize = rowSize;
	header.textureTileColumnSize = columnSize;
}

void DataManager::SetStartPosition(const FPOINT& pos)
{
	header.startPos = pos;
}

void DataManager::AddItemData(const ItemData& item)
{
	items.push_back(item);
	header.itemCount = static_cast<int>(items.size());
}

void DataManager::AddMonsterData(const MonsterData& monster)
{
	monsters.push_back(monster);
	header.monsterCount = static_cast<int>(monsters.size());
}

void DataManager::AddObstacleData(const ObstacleData& obstacle)
{
	obstacles.push_back(obstacle);
	header.obstacleCount = static_cast<int>(obstacles.size());
}

bool DataManager::GetMapData(MapData& outMapData)
{
	if(tiles.empty() || header.mapWidth <= 0 || header.mapHeight <= 0)
	{
		return false;
	}

	outMapData.width = header.mapWidth;
	outMapData.height = header.mapHeight;
	outMapData.tiles = tiles;

	// 텍스처 정보 설정
	outMapData.texture = TextureManager::GetInstance()->GetTexture(header.texturePath);
	outMapData.textureTileSize = header.textureTileSize;
	outMapData.textureTileRowSize = header.textureTileRowSize;
	outMapData.textureTileColumnSize = header.textureTileColumnSize;

	return true;
}

bool DataManager::ValidateHeader(const FileHeader& validateHeader)
{
	// 시그니처 확인
	if(validateHeader.signature[0] != 'M' || validateHeader.signature[1] != 'P' ||
		validateHeader.signature[2] != 'D' || validateHeader.signature[3] != 'T')
	{
		return false;
	}

	// 버전 확인
	if(validateHeader.version <= 0 || validateHeader.version > 1)
	{
		return false;
	}

	// 맵 크기 확인
	if(validateHeader.mapWidth <= 0 || validateHeader.mapHeight <= 0)
	{
		return false;
	}

	// 타일 수 확인
	if(validateHeader.tileCount != validateHeader.mapWidth * validateHeader.mapHeight)
	{
		return false;
	}

	return true;
}

bool DataManager::SaveMapFile(LPCWCH filePath)
{
	CreateDirectory(L"Map",NULL);

	HANDLE hFile = CreateFile(
		filePath,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if(hFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	bool success = true;

	// 헤더 저장
	if(!WriteHeader(hFile))
	{
		success = false;
	}

	// 타일 데이터 저장
	if(success && !WriteTiles(hFile))
	{
		success = false;
	}

	// 아이템 데이터 저장
	if(success && !WriteItems(hFile))
	{
		success = false;
	}

	// 몬스터 데이터 저장
	if(success && !WriteMonsters(hFile))
	{
		success = false;
	}

	// 장애물 데이터 저장 
	if(success && !WriteObstacles(hFile))
	{
		success = false;
	}

	CloseHandle(hFile);
	return success;
}

bool DataManager::LoadMapFile(LPCWCH filePath)
{
	HANDLE hFile = CreateFile(
		filePath,
		GENERIC_READ,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if(hFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	bool success = true;

	// 기존 데이터 초기화
	ClearAllData();

	// 헤더 읽기
	if(!ReadHeader(hFile))
	{
		success = false;
	}

	// 타일 데이터 읽기
	if(success && !ReadTiles(hFile))
	{
		success = false;
	}

	// 아이템 데이터 읽기
	if(success && !ReadItems(hFile))
	{
		success = false;
	}

	// 몬스터 데이터 읽기
	if(success && !ReadMonsters(hFile))
	{
		success = false;
	}

	// 장애물 데이터 읽기
	if(success && !ReadObstacles(hFile))
	{
		success = false;
	}

	CloseHandle(hFile);
	return success;
}

bool DataManager::WriteHeader(HANDLE hFile)
{
	DWORD bytesWritten;
	return WriteFile(hFile,&header,sizeof(FileHeader),&bytesWritten,NULL) &&
		bytesWritten == sizeof(FileHeader);
}

bool DataManager::ReadHeader(HANDLE hFile)
{
	DWORD bytesRead;
	if(!ReadFile(hFile,&header,sizeof(FileHeader),&bytesRead,NULL) ||
		bytesRead != sizeof(FileHeader))
	{
		return false;
	}

	return ValidateHeader(header);
}

bool DataManager::WriteTiles(HANDLE hFile)
{
	DWORD bytesWritten;

	// 타일 데이터 크기 기록
	DWORD dataSize = tiles.size() * sizeof(Room);
	if(!WriteFile(hFile,&dataSize,sizeof(DWORD),&bytesWritten,NULL) ||
		bytesWritten != sizeof(DWORD))
	{
		return false;
	}

	// 타일 데이터 기록
	if(!WriteFile(hFile,tiles.data(),dataSize,&bytesWritten,NULL) ||
		bytesWritten != dataSize)
	{
		return false;
	}

	return true;
}

bool DataManager::ReadTiles(HANDLE hFile)
{
	DWORD bytesRead;

	// 타일 데이터 크기 읽기
	DWORD dataSize;
	if(!ReadFile(hFile,&dataSize,sizeof(DWORD),&bytesRead,NULL) ||
		bytesRead != sizeof(DWORD))
	{
		return false;
	}

	// 타일 데이터 읽기
	tiles.resize(dataSize / sizeof(Room));
	if(!ReadFile(hFile,tiles.data(),dataSize,&bytesRead,NULL) ||
		bytesRead != dataSize)
	{
		return false;
	}

	return true;
}

bool DataManager::WriteItems(HANDLE hFile)
{
	DWORD bytesWritten;

	// 아이템 수 기록
	int itemCount = static_cast<int>(items.size());
	if(!WriteFile(hFile,&itemCount,sizeof(int),&bytesWritten,NULL) ||
		bytesWritten != sizeof(int))
	{
		return false;
	}

	// 아이템 데이터 기록
	for(const auto& item : items)
	{
		if(!WriteFile(hFile,&item,sizeof(ItemData),&bytesWritten,NULL) ||
			bytesWritten != sizeof(ItemData))
		{
			return false;
		}
	}

	return true;
}

bool DataManager::ReadItems(HANDLE hFile)
{
	DWORD bytesRead;

	// 아이템 수 읽기
	int itemCount;
	if(!ReadFile(hFile,&itemCount,sizeof(int),&bytesRead,NULL) ||
		bytesRead != sizeof(int))
	{
		return false;
	}

	// 아이템 데이터 읽기
	items.clear();
	for(int i = 0; i < itemCount; i++)
	{
		ItemData item;
		if(!ReadFile(hFile,&item,sizeof(ItemData),&bytesRead,NULL) ||
			bytesRead != sizeof(ItemData))
		{
			return false;
		}
		items.push_back(item);
	}

	return true;
}

bool DataManager::WriteMonsters(HANDLE hFile)
{
	DWORD bytesWritten;

	// 몬스터 수 기록
	int monsterCount = static_cast<int>(monsters.size());
	if(!WriteFile(hFile,&monsterCount,sizeof(int),&bytesWritten,NULL) ||
		bytesWritten != sizeof(int))
	{
		return false;
	}

	// 몬스터 데이터 기록
	for(const auto& monster : monsters)
	{
		if(!WriteFile(hFile,&monster,sizeof(MonsterData),&bytesWritten,NULL) ||
			bytesWritten != sizeof(MonsterData))
		{
			return false;
		}
	}

	return true;
}

bool DataManager::ReadMonsters(HANDLE hFile)
{
	DWORD bytesRead;

	// 몬스터 수 읽기
	int monsterCount;
	if(!ReadFile(hFile,&monsterCount,sizeof(int),&bytesRead,NULL) ||
		bytesRead != sizeof(int))
	{
		return false;
	}

	// 몬스터 데이터 읽기
	monsters.clear();
	for(int i = 0; i < monsterCount; i++)
	{
		MonsterData monster;
		if(!ReadFile(hFile,&monster,sizeof(MonsterData),&bytesRead,NULL) ||
			bytesRead != sizeof(MonsterData))
		{
			return false;
		}
		monsters.push_back(monster);
	}

	return true;
}

bool DataManager::WriteObstacles(HANDLE hFile)
{
	DWORD bytesWritten;

	// 장애물 수 기록
	int obstacleCount = static_cast<int>(obstacles.size());
	if(!WriteFile(hFile,&obstacleCount,sizeof(int),&bytesWritten,NULL) ||
		bytesWritten != sizeof(int))
	{
		return false;
	}

	// 장애물 데이터 기록
	for(const auto& obstacle : obstacles)
	{
		if(!WriteFile(hFile,&obstacle,sizeof(ObstacleData),&bytesWritten,NULL) ||
			bytesWritten != sizeof(ObstacleData))
		{
			return false;
		}
	}

	return true;
}

bool DataManager::ReadObstacles(HANDLE hFile)
{
	DWORD bytesRead;

	// 장애물 수 읽기
	int obstacleCount;
	if(!ReadFile(hFile,&obstacleCount,sizeof(int),&bytesRead,NULL) ||
		bytesRead != sizeof(int))
	{
		return false;
	}

	// 장애물 데이터 읽기
	obstacles.clear();
	for(int i = 0; i < obstacleCount; i++)
	{
		ObstacleData obstacle;
		if(!ReadFile(hFile,&obstacle,sizeof(ObstacleData),&bytesRead,NULL) ||
			bytesRead != sizeof(ObstacleData))
		{
			return false;
		}
		obstacles.push_back(obstacle);
	}

	return true;
}