#pragma once

#include "Singleton.h"
#include "config.h"
#include <vector>

using namespace std;
class Item;
class Sprite;
class ItemManager : public Singleton<ItemManager>
{
private:
	int itemCount;
	vector<Item*> vecitem;
	map<LPCWCH, Sprite>	itemMap;
public:
	// TODO: item 생성로직?
	// TODO: item 스프라이트 적용
	// TODO: item 먹을때 카운트 증가
	// TODO:
};

