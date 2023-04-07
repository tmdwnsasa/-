#pragma once
#include "Monobehaviour.h"
#include "pch.h"
#include <stack>
#include <set>

enum class ENEMY_STATE : uint8
{
	IDLE,
	WALK,


	DIE,
	END
};


struct Cell {
	int parent_x, parent_y;
	double f, g, h;
};

class Enemy : public MonoBehaviour
{
public:
	Enemy();
	virtual ~Enemy();

public:
	int									MAX = 101;
	double								INF = 1e9 + 7;

	// 직선
	const int dx1[4] = { 0, 0, 1, -1 };
	const int dy1[4] = { -1, 1, 0, 0 };

	// 대각선
	const int dx2[4] = { 1, -1, -1, 1 };
	const int dy2[4] = { -1, 1, -1, 1 };

	using Pair = std::pair<int, int>;
	using pPair = std::pair<double, Pair>;
	std::stack<Pair> s;
	char zmap[101][101];
	int Bmap[9][9] = { 0, };
	int ROW = 9;
	int COL = 9;


public:
	virtual void Update() override;
	void AstarCall();
	bool isDestination(int row, int col, Pair dst);
	bool isRanger(int row, int col);
	bool isUnBlock(int row, int col);
	float GetValue(int row, int col, Pair dst);
	void AstarMove();
	void Path(Cell cellDetails[101][101], Pair dst);
	void Astar(Pair src, Pair dst);
	void Checking();
	void TracePath(int x, int y, int z);
	void MakingMap();



private:
	float		_speed = 100.0f;
};