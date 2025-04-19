#pragma once
#include "structs.h"

#define ITEM_SIZE 0.2f

class AItem
{
protected:
	Sprite sprite;

	virtual void Action(void) = 0;

public:
	virtual ~AItem(void) {};

	virtual HRESULT Init(FPOINT pos) = 0;
	BOOL Update(void);
};

