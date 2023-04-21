#include "pch.h"
#include "Astar.h"
#include "TileNode.h"



bool TileNodeLess::operator() (TileNode* a0, TileNode* a1) const
{
	if (a0->GetFCost() == a1->GetFCost())
		return a0->hCost < a1->hCost;

	return a0->GetFCost() < a1->GetFCost();
}

bool TileNodeLess::operator() (TileNode* a0, int fCost) const
{
	return a0->GetFCost() < fCost;
}

bool TileNodeLess::operator() (int fCost, TileNode* a0) const
{
	return fCost < a0->GetFCost();
}

TileNode::TileNode()
{
	parent = NULL;
	index = 0;
	gCost = 0;
	hCost = 0;
}

TileNode::~TileNode()
{
}