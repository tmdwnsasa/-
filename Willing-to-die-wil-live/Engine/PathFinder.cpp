#include "pch.h"
#include "Astar.h"
#include <algorithm>
#include "PathFinder.h"
#include "TileMap.h"

//#define _OPTIMIZED_

PathFinder::PathFinder(const TileMap* tileMap)
{
	m_tileMap = tileMap;
}

PathFinder::~PathFinder()
{
}

std::list<TileNode*> PathFinder::DoFindPath(Vector2 startPos, Vector2 endPos)
{
	TileNode* goalNode = FindPath(startPos, endPos);
	if (goalNode == NULL)
		return std::list<TileNode*>();

	return GetAstarNodeList(goalNode);
}

std::list<TileNode*> PathFinder::GetAstarNodeList(TileNode* endNode)
{
	std::list<TileNode*> tileNodeList;

	if (endNode == NULL)
		return tileNodeList;

	TileNode* curNode = endNode;
	while (curNode->parent != NULL)
	{
		tileNodeList.push_front(curNode);
		curNode = curNode->parent;

		if (curNode->parent == NULL)
		{
			tileNodeList.push_front(curNode);
			break;
		}
	}

	return tileNodeList;
}

TileNode* PathFinder::FindPath(Vector2 startPos, Vector2 endPos)
{
	std::list<TileNode*> openNodeList;
	std::hash_set<TileNode*> closeNodeSet;

	TileNode* startNode = m_tileMap->GetTileNode(startPos);
	if (startNode == NULL)
		return NULL;

	TileNode* endNode = m_tileMap->GetTileNode(endPos);
	if (endNode == NULL)
		return NULL;

	startNode->parent = NULL;
	startNode->gCost = PathFinder::GCost(startNode, startNode);
	startNode->hCost = PathFinder::HCost(startNode, endNode);
	openNodeList.push_back(startNode);

	while (openNodeList.size() > 0)
	{

#ifdef _OPTIMIZED_
		// ����� ���� �� ��带 ã��.
		// ���� �������� �����ؼ� �����Ƿ� ���� �տ� �ִ°��� ����� ���� �Դϴ�.
		TileNode* curNode = openNodeList.front();
		if (curNode == NULL)
			break;
#else //_OPTIMIZED_
		TileNode* curNode = GetBestFCost(openNodeList);
		if (curNode == NULL)
			break;
#endif //_OPTIMIZED_

		// ���� ��Ͽ� �ְ� ���� ��Ͽ��� ����.
		closeNodeSet.insert(curNode);
		openNodeList.remove(curNode);

		// ����.
		if (curNode == endNode)
		{
			return curNode;
		}

		// �̿��� ����Ʈ�� ������
		auto neighborList = GetNeighborList(curNode);

		auto startIter = neighborList.begin();
		auto endIter = neighborList.end();
		for (; startIter != endIter; ++startIter)
		{
			TileNode* neighborNode = *startIter;

			// ���� ���� ���̶�� ����
			if (neighborNode->tileAttribute == TileAttribute::NotMoveable)
				continue;

			// �̹� ���� ��忡 �ִٸ� ����
			if (closeNodeSet.find(neighborNode) != closeNodeSet.end())
				continue;

			// ���� ��忡�� �̿���� ���� ���� ���
			int curToNeighborGCost = PathFinder::GCost(curNode, neighborNode);

			// �̿���忡�� �����带 ���ؼ� ���۳����� ���� ���
			int neighborToStartGCost = curNode->gCost + curToNeighborGCost;

			// ������Ͽ� ���ٸ� �߰� �մϴ�.
			auto fi = std::find(openNodeList.begin(), openNodeList.end(), neighborNode);
			if (fi == openNodeList.end())
			{
				// gCost = ������� �� ��� + ���翡�� �̿������� ���� ���
				neighborNode->parent = curNode;
				neighborNode->gCost = neighborToStartGCost;
				neighborNode->hCost = PathFinder::HCost(neighborNode, endNode);

#ifdef _OPTIMIZED_
				// ������������ �����ؼ� �߰� �մϴ�.
				auto fi = upper_bound(openNodeList.begin(), openNodeList.end(), neighborNode->GetFCost(), TileNodeLess());
				openNodeList.insert(fi, neighborNode);
#else //_OPTIMIZED_
				openNodeList.push_back(neighborNode);
#endif //_OPTIMIZED_
			}
			// ���� ��Ͽ� �ִٸ� ���� ��带 ���ؼ� ���� ���� ���� ���� ���� ��
			else
			{
				// ���� start���� ���� ��뺸�� ���縦 ���ؼ� ���� ����� �� �δٸ� 
				// �θ� ������� ��ü�ϰ� gCost�� �ٽ� ����մϴ�.
				if (neighborToStartGCost < neighborNode->gCost)
				{
					neighborNode->parent = curNode;
					neighborNode->gCost = neighborToStartGCost;
				}
			}
		}
	}

	return NULL;
}

std::list<TileNode*> PathFinder::GetNeighborList(TileNode* centerNode)
{
	std::list<TileNode*> neighborList;

	if (centerNode == NULL)
		return neighborList;

	for (int y = -1; y <= 1; ++y)
	{
		for (int x = -1; x <= 1; ++x)
		{
			int neighborX = centerNode->pos.x + x;
			int neighborY = centerNode->pos.y + y;

			if (neighborX < 0 || neighborX >= kWidth)
				continue;

			if (neighborY < 0 || neighborX >= kHeight)
				continue;

			TileNode* neighborNode = m_tileMap->GetTileNode(neighborX, neighborY);
			if (neighborNode == NULL)
				continue;

			// �̵��� �� ���� Ÿ���̶�� ����.
			if (neighborNode->tileAttribute == TileAttribute::NotMoveable)
				continue;

			// ���� �Ѱ��� 0�̾ƴϸ� �밢��
			if (y != 0 && x != 0)
			{
				int xAxisX = neighborX + -x;
				int xAxisY = neighborY;

				// x�� �˻��ؼ� ���� �����ִٸ� �밢�� �̵� �Ұ����̹Ƿ� ��󿡼� ����.
				TileNode* xAxisNode = m_tileMap->GetTileNode(xAxisX, xAxisY);
				if (xAxisNode != NULL &&
					xAxisNode->tileAttribute == TileAttribute::NotMoveable)
					continue;

				int yAxisX = neighborX;
				int yAxisY = neighborY + -y;

				// y�� �˻��ؼ� ���� �����ִٸ� �밢�� �̵� �Ұ����̹Ƿ� ��󿡼� ����.
				TileNode* yAxisNode = m_tileMap->GetTileNode(yAxisX, yAxisY);
				if (yAxisNode != NULL &&
					yAxisNode->tileAttribute == TileAttribute::NotMoveable)
					continue;
			}

			if (neighborNode == centerNode)
				continue;

			neighborList.push_back(neighborNode);
		}
	}

	return neighborList;
}

int PathFinder::GCost(TileNode* startNode, TileNode* endNode)
{
	return GCost(startNode->pos, endNode->pos);
}

int PathFinder::GCost(Vector2 startPos, Vector2 endPos)
{
	int xDiff = abs(endPos.x - startPos.x);
	int yDiff = abs(endPos.y - startPos.y);

	if (xDiff > yDiff)
		return (yDiff * 14) + (abs(xDiff - yDiff) * 10);
	else
		return (xDiff * 14) + (abs(yDiff - xDiff) * 10);
}

int PathFinder::HCost(TileNode* startNode, TileNode* endNode)
{
	return HCost(startNode->pos, endNode->pos);
}

int PathFinder::HCost(Vector2 startPos, Vector2 endPos)
{
	int xDiff = abs(endPos.x - startPos.x);
	int yDiff = abs(endPos.y - startPos.y);

	return (xDiff + yDiff) * 10;
}

TileNode* PathFinder::GetBestFCost(std::list<TileNode*>& openTileNodeList)
{
	if (openTileNodeList.size() == 0)
		return NULL;

	TileNode* curNode = openTileNodeList.back();

	auto startIter = openTileNodeList.rbegin();
	auto endIter = openTileNodeList.rend();

	for (; startIter != endIter; ++startIter)
	{
		TileNode* findNode = *startIter;

		if (findNode->GetFCost() < curNode->GetFCost())
		{
			curNode = findNode;
			continue;
		}
		else if (findNode->GetFCost() == curNode->GetFCost() &&
			findNode->hCost < curNode->hCost)
		{
			curNode = findNode;
		}
	}

	return curNode;
}
