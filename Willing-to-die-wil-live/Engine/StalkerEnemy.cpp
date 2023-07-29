#include "pch.h"
#include "StalkerEnemy.h"
#include "Engine.h"
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
#include "Component.h"
#include <iostream>

//cout 출력용 코드
//#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")


StalkerEnemy::StalkerEnemy() : Component(COMPONENT_TYPE::STALKERENEMY)
{

}

StalkerEnemy::~StalkerEnemy()
{

}

void StalkerEnemy::Update()
{
	Vec3 pos = GetTransform()->GetLocalPosition();

	//pos += GetTransform()->GetLook() * _speed * 10 * DELTA_TIME;
	//pos += GetTransform()->GetRight() * _speed * DELTA_TIME;


	//GetTransform()->SetLocalPosition(pos);
	// 1 공격 2 죽음 3 걷기

	SetEnemyPosition(pos);

	SetPlayerPos();
	if (!ResponeCheck)
	{
		//Respone();
	}
	Time += DELTA_TIME;
	if (Time > 0.1)
	{
		if (_hp > 0)
		{
			AstarCall();
		}
		Time = 0;
	}
	if (_hp > 0)
	{
		AstarMove(firstx, firsty, secondx, secondy);
	}

	Animation();

	if (_hp <= 0)
	{
		AnimationCount();
	}

}



void StalkerEnemy::AstarCall()
{
	TileMap tileMap;
	PathFinder pathFinder(&tileMap);

	Vector2 startPos;
	Vector2 endPos;
	tileMap.CreateTile(startPos, endPos,2);

	//tileMap.Display(std::list<TileNode*>());
	//printf("\n\n");
	auto nodeList = pathFinder.DoFindPath(startPos, endPos);
	//tileMap.Display(nodeList);


	Vec3 EPos = GetEnemyPosition();

	//std::cout <<EPos.x <<"Z좌표 :    "<< EPos.z << std::endl;

	if (nodeList.size() != 0)
	{
		firstx = nodeList.front()->pos.x;
		firsty = nodeList.front()->pos.y;


		list<TileNode*>::iterator iter = nodeList.begin();

		advance(iter, 1);
		secondx = (*iter)->pos.x;
		secondy = (*iter)->pos.y;

	}

}

void StalkerEnemy::AstarMove(int x, int y, int z, int w)
{
	if (DELTA_TIME > 1)
		return;
	Vec3 pos = GetTransform()->GetLocalPosition();

	x = x - z;
	y = y - w;
	if (x == -1 && y == 0)
	{
		GetTransform()->SetLocalRotation(Vec3(0, py * 0.5, 0));

		pos += GetTransform()->GetLook() * _speed * DELTA_TIME; // 보는 방향기준 오른쪽
		//float a = DELTA_TIME;
		//pos += GetTransform()->GetRight() * _speed * 10; // 보는 방향기준 오른쪽
	}

	if (x == -1 && y == 1)
	{
		GetTransform()->SetLocalRotation(Vec3(0, py * 0.25, 0));

		pos += GetTransform()->GetLook() * _speed * DELTA_TIME; // 보는 방향기준 오른쪽
		//float a = DELTA_TIME;
		//pos += GetTransform()->GetRight() * _speed * 10; // 보는 방향기준 오른쪽
	}
	if (x == -1 && y == -1)
	{
		GetTransform()->SetLocalRotation(Vec3(0, py * 0.75, 0));

		pos += GetTransform()->GetLook() * _speed * DELTA_TIME; // 보는 방향기준 오른쪽
		//float a = DELTA_TIME;
		//pos += GetTransform()->GetRight() * _speed * 10; // 보는 방향기준 오른쪽
	}


	if (x == 1 && y == 0)
	{
		GetTransform()->SetLocalRotation(Vec3(0, -py * 0.5, 0));

		pos += GetTransform()->GetLook() * _speed * DELTA_TIME; // 보는 방향기준 왼쪽
		//pos -= GetTransform()->GetRight() * _speed * 10; // 보는 방향기준 왼쪽
	}


	if (x == 1 && y == 1)
	{
		GetTransform()->SetLocalRotation(Vec3(0, -py * 0.25, 0));

		pos += GetTransform()->GetLook() * _speed * DELTA_TIME; // 보는 방향기준 왼쪽
		//pos -= GetTransform()->GetRight() * _speed * 10; // 보는 방향기준 왼쪽
	}


	if (x == 1 && y == -1)
	{
		GetTransform()->SetLocalRotation(Vec3(0, -py * 0.75, 0));

		pos += GetTransform()->GetLook() * _speed * DELTA_TIME; // 보는 방향기준 왼쪽
		//pos -= GetTransform()->GetRight() * _speed * 10; // 보는 방향기준 왼쪽
	}
	if (x == 0 && y == -1)
	{
		GetTransform()->SetLocalRotation(Vec3(0, -py, 0));

		pos += GetTransform()->GetLook() * _speed * DELTA_TIME;
	}

	if (x == 0 && y == 1)
	{
		GetTransform()->SetLocalRotation(Vec3(0, 0, 0));

		pos += GetTransform()->GetLook() * _speed * DELTA_TIME;
	}
	double rotpos = GetTransform()->GetLocalRotation().y;
	GetTransform()->SetLocalPosition(pos);

}



void StalkerEnemy::SetPlayerPos()
{
	for (int i = 0; i < 100; i++)
		for (int j = 0; j < 100; j++)
		{
			if (STtileMap[i][j] == 2)
			{
				STtileMap[i][j] = 0;
				/*EnemyCount++;
				if (EnemyCount == 10)
				{
					tileMap[i][j] = 0;
					EnemyCount = 0;
				}*/
			}
			else if (STtileMap[i][j] == 3)
			{
				STtileMap[i][j] = 0;
			}
		}
	shared_ptr<Scene> scene = GET_SINGLE(SceneManager)->GetActiveScene();
	Vec3 PlayerPos = scene->GetPlayerPosToEnemy();

	double k = PlayerPos.x;
	double l = -PlayerPos.z;

	k = (k / 300 * 4) - 1;
	l = (l / 300 * 4) + 1;

	int x = (int)k;
	int y = (int)l;
	STtileMap[y][x] = 3;

	Vec3 EPos = GetEnemyPosition();

	if (EPos.z > 0)
		EPos.z = 0;

	double p = EPos.x;
	double q = -(EPos.z);


	p = (p / 300 * 4) - 1;
	q = (q / 300 * 4) + 1;


	x = (int)p;
	y = (int)q;
	STtileMap[y][x] = 2;

	int w = x + y;
}

void StalkerEnemy::Animation()
{
	if (AnimeCount == 0)
	{
		int32 count = GetAnimator()->GetAnimCount();
		
		int32 index = 2 % count;
		GetAnimator()->Play(index);
		AnimeCount++;
	}
}

void StalkerEnemy::AnimationCount()
{
	if (AnimeCount == 1)
	{
		int32 count = GetAnimator()->GetAnimCount();
		
		int32 index =1 % count;
		GetAnimator()->Play(index);
		AnimeCount++;
	}

	else if (AnimeCount == 2)
	{
		DieTime += DELTA_TIME;
		if (DieTime > 2.2)
		{
			GetAnimator()->Stop();
		}
	}
}



int(*StalkerEnemy::CreateMap())[STHeight]
{
	return STtileMap;
}

void StalkerEnemy::LostHp()
{
	_hp -= 30;
}

void StalkerEnemy::Respone()
{
	shared_ptr<Scene> scene = GET_SINGLE(SceneManager)->GetActiveScene();
	Vec3 PlayerPos = scene->GetPlayerPosToEnemy();

	if (PlayerPos.x > 3500)
		if (PlayerPos.z < -3500)
			CheckPoint = 1;
		else if (PlayerPos.z >= -3500)
			CheckPoint = 2;
	if (PlayerPos.x <= 3500)
		if (PlayerPos.z < -3500)
			CheckPoint = 3;
		else if (PlayerPos.z >= -3500)
			CheckPoint = 4;

	switch (CheckPoint)
	{
	case 1:
		GetTransform()->SetLocalPosition(ResponeArea1);
		break;
	case 2:
		GetTransform()->SetLocalPosition(ResponeArea2);
		break;
	case 3:
		GetTransform()->SetLocalPosition(ResponeArea3);
		break;
	case 4:
		GetTransform()->SetLocalPosition(ResponeArea4);
		break;
	default:
		break;
	}

	ResponeCheck = true;
}

