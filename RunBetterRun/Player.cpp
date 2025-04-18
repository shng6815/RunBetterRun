#include "Player.h"
#include "MapManager.h"

HRESULT Player::Init(function<void(float, float,bool)> shakeFunc)
{
	fov = 0.66f;

	cameraPos = { 22, 12 };

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

	// 카메라 흔들기
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

    if (km->IsStayKeyDown('T'))
        shakeFunc(10,0.2f,true);
    
    if (km->IsOnceKeyDown(VK_SHIFT))
        moveSpeed = runSpeed;

    if (km->IsOnceKeyUp(VK_SHIFT))
        moveSpeed = defaultSpeed;

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
	if (deltaX < 0)
	{
		rotate.x = deltaX;
		rotate.y = 0;
	}
	else if (deltaX > 0)
	{
		rotate.x = 0;
		rotate.y = -deltaX;
	}
	else
	{
		rotate.x = 0;
		rotate.y = 0;
	}
	SetCursorPos(WINSIZE_X / 2, WINSIZE_Y / 2);
}

void Player::MoveCamera(float deltaTime)
{
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

    // 카메라가 이동할 수 있는지 확인
    int x = INT(pos.x);
    int y = INT(pos.y);
    MapData* md = MapManager::GetInstance()->GetMapData();
    if ((0 <= x && x < md->width && 0 <= y && y < md->height)
        && md->tiles[y * md->width + x].roomType == RoomType::FLOOR)
    {
        cameraPos = pos;
    }
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
    plane.x = cameraHorDir.x * fov;
    plane.y = cameraHorDir.y * fov;
}

void Player::Save()
{
    const LPCWCH filePath = L"Map/SavedMap.dat";

    CreateDirectory(L"Map", NULL);

    if (MapManager::GetInstance()->SaveMap(filePath))
    {
        MessageBox(g_hWnd, TEXT("맵이 성공적으로 저장되었습니다."), TEXT("알림"), MB_OK);
    }
    else
    {
        MessageBox(g_hWnd, TEXT("맵 저장에 실패했습니다."), TEXT("경고"), MB_OK);
    }
}

void Player::Load()
{
    const LPCWCH filePath = L"Map/SavedMap.dat";

    if (MapManager::GetInstance()->LoadMap(filePath))
    {
        MessageBox(g_hWnd, TEXT("맵이 성공적으로 로드되었습니다."), TEXT("알림"), MB_OK);
    }
    else
    {
        MessageBox(g_hWnd, TEXT("맵 로드에 실패했습니다."), TEXT("경고"), MB_OK);
    }
}