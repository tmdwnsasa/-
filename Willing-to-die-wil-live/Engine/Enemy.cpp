#include "pch.h"
#include "Engine.h"
#include "Enemy.h"
#include "Input.h"
#include "Animator.h"
#include "Timer.h"
#include "Transform.h"
#include "Astar.h"
#include "TileNode.h"
#include "TileMap.h"
#include "PathFinder.h"
#include "Scene.h"
#include "SceneManager.h"



Enemy::Enemy()
{

}

Enemy::~Enemy()
{

}

void Enemy::Update()
{
	Vec3 pos = GetTransform()->GetLocalPosition();

	//pos += GetTransform()->GetLook() * _speed * 10 * DELTA_TIME;
	//pos += GetTransform()->GetRight() * _speed * DELTA_TIME;


	if (INPUT->GetButtonDown(KEY_TYPE::KEY_3))
	{
		int32 count = GetAnimator()->GetAnimCount();
		int32 currentIndex = GetAnimator()->GetCurrentClipIndex();

		int32 index = (currentIndex + 1) % count;
		GetAnimator()->Play(index);
	}

	//GetTransform()->SetLocalPosition(pos);

	//SetEnemyPosition(pos);

	//SetPlayerPos();

	AstarCall();
}


void Enemy::AstarCall()
{
	TileMap tileMap;
	PathFinder pathFinder(&tileMap);

	Vector2 startPos;
	Vector2 endPos;
	tileMap.CreateTile(startPos, endPos);

	tileMap.Display(std::list<TileNode*>());
	printf("\n\n");
	auto nodeList = pathFinder.DoFindPath(startPos, endPos);
	tileMap.Display(nodeList);

	int k = nodeList.front()->pos.x;
	int l = nodeList.front()->pos.y;
	
	list<TileNode*>::iterator iter = nodeList.begin();

	advance(iter, 1);
	int p = (*iter)->pos.x;
	int q = (*iter)->pos.y;

	AstarMove(k, l, p, q);
}

void Enemy::AstarMove(int x, int y, int z, int w)
{
	Vec3 pos = GetTransform()->GetLocalPosition();

	x = x - z;
	y = y - w;
	if (x == 1)
	{
		pos += GetTransform()->GetLook() * _speed * 10;
	}

	GetTransform()->SetLocalPosition(pos);

}



void Enemy::SetPlayerPos()
{
	shared_ptr<Scene> scene = GET_SINGLE(SceneManager)->GetActiveScene();
	Vec3 PlayerPos = scene->GetPlayerPosToEnemy();

	int x = PlayerPos.x;
	int y = PlayerPos.y;

	x = 1;
	y = 2;
	tileMap[y][x] = 3;

	Vec3 EPos = GetEnemyPosition();

	//x = EPos.x + 3;
	//y = EPos.y + 3;
	x = 3;
	y = 3;
	tileMap[y][x] = 2;
}

int(*Enemy::CreateMap())[Height]
{
	return tileMap;
}

