#pragma once
#include "Singleton.h"
#include "structs.h"
#include <list>

class AItem;
class ItemManager: public Singleton<ItemManager>
{
	list<AItem*>	items;
	DWORD		keyCount;

	HRESULT LoadFile(LPCWCH path);

public:
	HRESULT Init(void);
	HRESULT Init(LPCWCH path);
	void	Release(void);
	void	Update(void);

	DWORD	Key(void) {	return keyCount; }
	void	PutItem(AItem* item);
	void	PushKey(void);
	void	PopKey(void);
};

