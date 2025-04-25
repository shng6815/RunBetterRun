#pragma once
#include "structs.h"

#define AMONSTER_SPEED	2.5f
#define AMONSTER_SIZE	0.8f

class AMonster
{
protected:
	Sprite sprite;
	float speed;
	FPOINT targetPosition;
	Direction direction;
	
	virtual void Action(void);
	virtual void Catch(void);
	BOOL CanMoveToPosition(FPOINT pos);

public:
	virtual ~AMonster(void) {};
	virtual HRESULT Init(FPOINT pos) = 0;
	virtual void Update(void);

	FPOINT GetPostion() { return sprite.pos; }
	void SetPosition(FPOINT pos) { sprite.pos = pos; }
	void SetTargetPosition(FPOINT pos) { targetPosition = pos; }
};

