#pragma once

#include <Windows.h>
#include "player_stats.h"

typedef struct ITEM_SET {
	int* base_pointer;
    char* player_name;
	player_stats* player_stat;
	ITEM_SET *next;
} item_set, *pitem_set;

bool insert(item_set** head, item_set* node);
void clear(item_set** head);
void print(item_set** head);