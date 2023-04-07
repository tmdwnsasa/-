#pragma once

#include <list>
#include "TileNode.h"

static const int kWidth = 10;
static const int kHeight = 10;

static int tileMapArray[kHeight][kWidth] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 1, 1, 1, 1, 0, 0, 0, 0,
	0, 0, 1, 0, 0, 1, 0, 0, 0, 0,
	0, 0, 1, 0, 0, 1, 0, 0, 0, 0,
	2, 0, 1, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 1, 0, 0, 1, 3, 0, 0, 0,
	0, 0, 1, 0, 0, 1, 1, 1, 1, 1,
	0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
};

class TileMap
{
private:
	mutable TileNode m_tileNode[kHeight][kWidth];

public:
	TileMap();
	~TileMap();

	void CreateTile(Vector2& start, Vector2& end);
	void Display(std::list<TileNode*> moveNodeList);
	void CreateMap();
	TileNode* GetTileNode(int x, int y) const;
	TileNode* GetTileNode(Vector2 pos) const;
};