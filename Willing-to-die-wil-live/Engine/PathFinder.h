#pragma once

class TileMap;
class TileNode;

class PathFinder
{
private:
	const TileMap* m_tileMap;

public:
	PathFinder(const TileMap* tileMap);
	~PathFinder();

	std::list<TileNode*> DoFindPath(Vector2 startPos, Vector2 endPos);

private:
	std::list<TileNode*> GetAstarNodeList(TileNode* endNode);
	TileNode* FindPath(Vector2 startPos, Vector2 endPos);
	std::list<TileNode*> GetNeighborList(TileNode* centerNode);
	static int GCost(TileNode* startNode, TileNode* endNode);
	static int GCost(Vector2 startPos, Vector2 endPos);
	static int HCost(TileNode* startNode, TileNode* endNode);
	static int HCost(Vector2 startPos, Vector2 endPos);
	static TileNode* GetBestFCost(std::list<TileNode*>& openTileNodeList);
};

