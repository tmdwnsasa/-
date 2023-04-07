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
		// 비용이 제일 싼 노드를 찾음.
		// 내림 차순으로 정렬해서 넣으므로 가장 앞에 있는것이 비용이 제일 쌉니다.
		TileNode* curNode = openNodeList.front();
		if (curNode == NULL)
			break;
#else //_OPTIMIZED_
		TileNode* curNode = GetBestFCost(openNodeList);
		if (curNode == NULL)
			break;
#endif //_OPTIMIZED_

		// 닫힌 목록에 넣고 오픈 목록에서 삭제.
		closeNodeSet.insert(curNode);
		openNodeList.remove(curNode);

		// 종료.
		if (curNode == endNode)
		{
			return curNode;
		}

		// 이웃의 리스트를 가져옴
		auto neighborList = GetNeighborList(curNode);

		auto startIter = neighborList.begin();
		auto endIter = neighborList.end();
		for (; startIter != endIter; ++startIter)
		{
			TileNode* neighborNode = *startIter;

			// 갈수 없는 곳이라면 무시
			if (neighborNode->tileAttribute == TileAttribute::NotMoveable)
				continue;

			// 이미 닫힌 노드에 있다면 무시
			if (closeNodeSet.find(neighborNode) != closeNodeSet.end())
				continue;

			// 현재 노드에서 이웃노드 까지 가는 비용
			int curToNeighborGCost = PathFinder::GCost(curNode, neighborNode);

			// 이웃노드에서 현재노드를 통해서 시작노드까지 가는 비용
			int neighborToStartGCost = curNode->gCost + curToNeighborGCost;

			// 열린목록에 없다면 추가 합니다.
			auto fi = std::find(openNodeList.begin(), openNodeList.end(), neighborNode);
			if (fi == openNodeList.end())
			{
				// gCost = 현재까지 든 비용 + 현재에서 이웃노드까지 가는 비용
				neighborNode->parent = curNode;
				neighborNode->gCost = neighborToStartGCost;
				neighborNode->hCost = PathFinder::HCost(neighborNode, endNode);

#ifdef _OPTIMIZED_
				// 내림차순으로 정렬해서 추가 합니다.
				auto fi = upper_bound(openNodeList.begin(), openNodeList.end(), neighborNode->GetFCost(), TileNodeLess());
				openNodeList.insert(fi, neighborNode);
#else //_OPTIMIZED_
				openNodeList.push_back(neighborNode);
#endif //_OPTIMIZED_
			}
			// 열린 목록에 있다면 현재 노드를 통해서 가는 비용과 원래 가는 비용과 비교
			else
			{
				// 원래 start까지 가는 비용보다 현재를 통해서 가는 비용이 더 싸다면 
				// 부모를 현재노드로 교체하고 gCost를 다시 계산합니다.
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

			// 이동할 수 없는 타일이라면 무시.
			if (neighborNode->tileAttribute == TileAttribute::NotMoveable)
				continue;

			// 둘중 한개라도 0이아니면 대각선
			if (y != 0 && x != 0)
			{
				int xAxisX = neighborX + -x;
				int xAxisY = neighborY;

				// x축 검사해서 벽이 막혀있다면 대각선 이동 불가능이므로 대상에서 제외.
				TileNode* xAxisNode = m_tileMap->GetTileNode(xAxisX, xAxisY);
				if (xAxisNode != NULL &&
					xAxisNode->tileAttribute == TileAttribute::NotMoveable)
					continue;

				int yAxisX = neighborX;
				int yAxisY = neighborY + -y;

				// y축 검사해서 벽이 막혀있다면 대각선 이동 불가능이므로 대상에서 제외.
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
