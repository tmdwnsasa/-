#include "pch.h"
#include "Engine.h"
#include "Enemy.h"
#include "Input.h"
#include "Animator.h"
#include "Timer.h"
#include "Transform.h"
#include "Astar.h"
#include "TileMap.h"
#include "PathFinder.h"




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

	GetTransform()->SetLocalPosition(pos);

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
}

