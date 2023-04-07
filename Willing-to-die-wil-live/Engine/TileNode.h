#pragma once

struct TileAttribute
{
	enum T
	{
		Moveable = 0,
		NotMoveable = 1,
		Start = 2,
		End = 3,
	};
};

class TileNode;
struct TileNodeLess
{
	bool operator() (TileNode* a0, TileNode* a1) const;
	bool operator() (TileNode* a0, int fCost) const;
	bool operator() (int fCost, TileNode* a0) const;
};

class TileNode
{
public:
	TileNode* parent;
	TileAttribute::T tileAttribute;
	int index;
	Vector2 pos;
	int gCost;
	int hCost;

public:
	TileNode();
	~TileNode();

	int GetFCost() const
	{
		return gCost + hCost;
	}
};