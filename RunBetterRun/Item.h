#pragma once
#include "structs.h"

#define ITEM_SIZE 0.2f

class Item
{
protected:
	Sprite sprite;
	virtual void Action(void);

public:
	 virtual HRESULT Init(FPOINT pos);
	 HRESULT Init(FPOINT pos, LPCWCH path, AnimationInfo ani);
	 BOOL Update(void);
};

