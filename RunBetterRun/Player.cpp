#include "Player.h"
#include "MapManager.h"
#include "DataManager.h"

HRESULT Player::Init(function<void(float, float,bool)> shakeFunc)
{
    fov = 0.66f;
    targetFOV = fov;

	FPOINT startPos = DataManager::GetInstance()->GetStartPosition();

	if(startPos.x != 0.0f || startPos.y != 0.0f)
	{
		cameraPos = startPos;
	} 
	else
	{
		cameraPos = {22,12};
	}


	cameraVerDir = { -1, 0 };
	cameraHorDir = { 0, 1.f };

    plane = {
        cameraVerDir.y * fov,
        -cameraVerDir.x * fov
    };

	moveInput = { 0, 0 };
	rotate = { 0, 0 };

    defaultSpeed = moveSpeed = 1.2f;
    runSpeed = defaultSpeed * 2;

	rotateSpeed = 0.8f;
	stepElapsedTime = 0;
	stepTime = 0.5f;
	runTime = 0.3f;

	static bool isFirstInit = true;

	if(isFirstInit)
	{
		playerLife = 3;
		isFirstInit = false;
	}

	// ī�޶� ����
	this->shakeFunc = shakeFunc;

	return S_OK;
}

void Player::Release()
{
}

void Player::Update()
{
	KeyInput();
	MouseInput();
	
	float deltaTime = TimerManager::GetInstance()->GetDeltaTime();

	MoveCamera(deltaTime);
	RotateCamera(deltaTime);

    fov += (targetFOV - fov) * deltaTime * fovLerpSpeed;
    UpdateFOV();

}

void Player::Render(HDC hdc)
{
}

void Player::KeyInput(void)
{
    KeyManager* km = KeyManager::GetInstance();

	moveInput = { 0, 0 };

    if (km->IsStayKeyDown('W'))
        moveInput.x = -1;

    if (km->IsStayKeyDown('S'))
        moveInput.x = 1;

    if (km->IsStayKeyDown('A'))
        moveInput.y = -1;

    if (km->IsStayKeyDown('D'))
        moveInput.y = 1;
    
    if (km->IsOnceKeyDown(VK_SHIFT))
    {
        targetFOV = 0.5f;
        moveSpeed = runSpeed;
    }

    if (km->IsOnceKeyUp(VK_SHIFT))
    {
        targetFOV = 0.66f;
        moveSpeed = defaultSpeed;
    }
    if (km->IsOnceKeyDown('O'))
        Save(); 

    if (km->IsOnceKeyDown('P'))
        Load();
}

void Player::MouseInput(void)
{
	POINT currentPos;
	GetCursorPos(&currentPos);
	int deltaX = currentPos.x - WINSIZE_X / 2;

    rotate.x = deltaX < 0 ? deltaX : 0;
    rotate.y = deltaX > 0 ? -deltaX : 0;

	SetCursorPos(WINSIZE_X / 2, WINSIZE_Y / 2);
}

void Player::MoveCamera(float deltaTime)
{
    if (!moveInput.x && !moveInput.y)
    {
		stepElapsedTime = 0;
		return;
    }

	stepElapsedTime += deltaTime;

    float interval = (runSpeed == moveSpeed ? runTime : stepTime);
    if (stepElapsedTime > interval)
    {
        shakeFunc(moveSpeed * 10 + 3, 0.2f, true);
        stepElapsedTime = 0;
    }

    bool moveForward = moveInput.x > 0;
    bool moveBackward = moveInput.x < 0;
    bool moveLeft = moveInput.y < 0;
    bool moveRight = moveInput.y > 0;

    FPOINT pos = cameraPos;

    if (moveForward || moveBackward) {
        pos.x += (moveForward ? -1 : 1) * (cameraVerDir.x * moveSpeed * deltaTime);
        pos.y += (moveForward ? -1 : 1) * (cameraVerDir.y * moveSpeed * deltaTime);
    }

    if (moveLeft || moveRight) {
        pos.x += (moveLeft ? -1 : 1) * (cameraHorDir.x * moveSpeed * deltaTime);
        pos.y += (moveLeft ? -1 : 1) * (cameraHorDir.y * moveSpeed * deltaTime);
    }
	
	Move(pos);
}

void Player::Move(FPOINT pos)
{
	int x = INT(pos.x);
	int y = INT(pos.y);
	BOOL result = TRUE;
	MapData* md = MapManager::GetInstance()->GetMapData();

	if ((0 <= x && x < md->width && 0 <= y && y < md->height)
		&& md->tiles[y * md->width + x].roomType == RoomType::FLOOR)
	{
		int oldX = INT(cameraPos.x);
		int oldY = INT(cameraPos.y);
		Obstacle* newMove = md->tiles[y * md->width + x].obstacle;
		Obstacle* oldMove = md->tiles[oldY * md->width + oldX].obstacle;
		if (newMove && newMove->block)
		{
			if ((oldX < x && newMove->dir == Direction::EAST)
				|| (oldX > x && newMove->dir == Direction::WEST)
				|| (oldY < y && newMove->dir == Direction::SOUTH)
				|| (oldY > y && newMove->dir == Direction::NORTH))
				result = FALSE;
		}
		if(oldMove && oldMove->block)
		{
			if ((oldX < x && oldMove->dir == Direction::WEST)
				|| (oldX > x && oldMove->dir == Direction::EAST)
				|| (oldY < y && oldMove->dir == Direction::NORTH)
				|| (oldY > y && oldMove->dir == Direction::SOUTH))
				result = FALSE;
		}
	} else
		result = FALSE;
	if (result)
		cameraPos = pos;
}

void Player::RotateCamera(float deltaTime)
{
    float rotateCos, rotateSin;
    if (rotate.x)
    {
        rotateCos = cosf(rotate.x * (-rotateSpeed) * deltaTime);
        rotateSin = sinf(rotate.x * (-rotateSpeed) * deltaTime);
    }
    else
    {
        rotateCos = cosf(rotate.y * rotateSpeed * deltaTime);
        rotateSin = sinf(rotate.y * rotateSpeed * deltaTime);
    }

    FPOINT old = cameraVerDir;

    cameraVerDir.x = (cameraVerDir.x * rotateCos) - (cameraVerDir.y * rotateSin);
    cameraVerDir.y = (old.x * rotateSin) + (cameraVerDir.y * rotateCos);

    cameraHorDir.x = cameraVerDir.y;
	cameraHorDir.y = -cameraVerDir.x;

	UpdateFOV();
}

void Player::UpdateFOV()
{
	plane.x = cameraVerDir.y * this->fov;
	plane.y = -cameraVerDir.x * this->fov;
}

void Player::LossPlayerLife()
{
	if(playerLife > 0) playerLife -= 1;
}

void Player::Save()
{
    const LPCWCH filePath = L"Map/SavedMap.dat";

    CreateDirectory(L"Map", NULL);

    MapManager::GetInstance()->SaveMap(filePath);
}

void Player::Load()
{
    const LPCWCH filePath = L"Map/SavedMap.dat";

    MapManager::GetInstance()->LoadMap(filePath);
}