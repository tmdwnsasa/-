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
#include <iostream>

//cout 출력용 코드
//#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")


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

	SetEnemyPosition(pos);

	SetPlayerPos();

	AstarCall();

	Animation();
}


void Enemy::AstarCall()
{
	TileMap tileMap;
	PathFinder pathFinder(&tileMap);

	Vector2 startPos;
	Vector2 endPos;
	tileMap.CreateTile(startPos, endPos);

	//tileMap.Display(std::list<TileNode*>());
	//printf("\n\n");
	auto nodeList = pathFinder.DoFindPath(startPos, endPos);
	//tileMap.Display(nodeList);


	Vec3 EPos = GetEnemyPosition();

	//std::cout << EPos.x << std::endl;

	if (nodeList.size() != 0)
	{
		int k = nodeList.front()->pos.x;
		int l = nodeList.front()->pos.y;


		list<TileNode*>::iterator iter = nodeList.begin();

		advance(iter, 1);
		int p = (*iter)->pos.x;
		int q = (*iter)->pos.y;

		AstarMove(k, l, p, q);
	}
}

void Enemy::AstarMove(int x, int y, int z, int w)
{
	Vec3 pos = GetTransform()->GetLocalPosition();

	x = x - z;
	y = y - w;
	if (x == -1)
	{
		pos += GetTransform()->GetRight() * _speed * 10;
	}

	if (x == 1)
	{
		pos -= GetTransform()->GetRight() * _speed * 10;
	}

	if (y == -1)
	{
		pos -= GetTransform()->GetLook() * _speed * 10;
	}

	if (y == 1)
	{
		pos += GetTransform()->GetLook() * _speed * 10;
	}


	GetTransform()->SetLocalPosition(pos);

}



void Enemy::SetPlayerPos()
{
	for(int i=0; i<25; i++)
		for (int j = 0; j < 25; j++)
		{
			if (tileMap[i][j] == 2)
			{
				tileMap[i][j] = 0;
			}
			else if (tileMap[i][j] == 3)
			{
				tileMap[i][j] = 0;
			}
		}
	shared_ptr<Scene> scene = GET_SINGLE(SceneManager)->GetActiveScene();
	Vec3 PlayerPos = scene->GetPlayerPosToEnemy();

	double k = PlayerPos.x;
	double l = -PlayerPos.z;

	k = (k / 300)-1;
	l = (l / 300)+1;

	int x = (int)k;
	int y = (int)l;
	tileMap[y][x] = 3;

	Vec3 EPos = GetEnemyPosition();

	k = EPos.x;
	l = -(EPos.z);


	k = (k / 300) - 1;
	l = (l / 300) + 1;
	

	x = (int)k;
	y = (int)l;
	tileMap[y][x] = 2;

	int w = x + y;
}

int(*Enemy::CreateMap())[Height]
{
	return tileMap;
}

void Enemy::Animation()
{
	if (INPUT->GetButtonDown(KEY_TYPE::KEY_1))
	{
		int32 count = GetAnimator()->GetAnimCount();
		int32 currentIndex = GetAnimator()->GetCurrentClipIndex();

		int32 index = (currentIndex + 1) % count;
		GetAnimator()->Play(index);
	}

	if (INPUT->GetButtonDown(KEY_TYPE::KEY_2))
	{
		int32 count = GetAnimator()->GetAnimCount();
		int32 currentIndex = GetAnimator()->GetCurrentClipIndex();

		int32 index = (currentIndex - 1 + count) % count;
		GetAnimator()->Play(index);
	}

	if (INPUT->GetButtonDown(KEY_TYPE::KEY_5))
	{
		GetAnimator()->Stop();
	}
}

