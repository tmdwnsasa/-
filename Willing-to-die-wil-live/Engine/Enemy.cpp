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
#include "Component.h"
#include <iostream>


//cout 출력용 코드
//#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")


Enemy::Enemy() : Component(COMPONENT_TYPE::ENEMY)
{

}

Enemy::~Enemy()
{

}

void Enemy::Update()
{

		Vec3 pos = GetTransform()->GetLocalPosition();

		shared_ptr<Scene> scene = GET_SINGLE(SceneManager)->GetActiveScene();
		Vec3 PlayerPos = scene->GetPlayerPosToEnemy();

		switch (_currentState)
		{
		case ENEMY_STATE::IDLE:
			break;
		case ENEMY_STATE::WALK:
			Animation();
			break;
		case ENEMY_STATE::Attack:
			AttackAnimation();
			break;
		case ENEMY_STATE::DIE:
			DeathAnimation();
			break;
		case ENEMY_STATE::END:
			break;
		default:
			break;
		}


		SetEnemyPosition(pos);
		SetPlayerPos();

		if (!ResponeCheck)
		{
			Respone();
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
		//AstarMove(firstx, firsty, secondx, secondy);


		if (_hp > 0)
		{
				Moving += DELTA_TIME;
				if (Moving > 1.0)
				{
					AstarMove(firstx, firsty, secondx, secondy);
				}
		}
		if (Dead == false)
		{
			if (_distance >= 200)
			{
				if (Attack == false)
				{
					AttackDelay += DELTA_TIME;
					if (AttackDelay > 2.0)
					{
						WalkState = true;
						SetAttack(true);
						SetState(ENEMY_STATE::WALK);
					}
				}
			}
		}

		_distance = sqrt(pow(pos.x - PlayerPos.x, 2) + pow(pos.z - PlayerPos.z, 2));
		if (Dead == false)
		{
			if (_distance < 200)
			{
				SetState(ENEMY_STATE::Attack);
			}
		}
		if (_hp <= 0)
		{
			SetState(ENEMY_STATE::DIE);
		}
}


void Enemy::AstarCall()
{
	TileMap tileMap;
	PathFinder pathFinder(&tileMap);

	Vector2 startPos;
	Vector2 endPos;
	tileMap.CreateTile(startPos, endPos,1);

	//tileMap.Display(std::list<TileNode*>());
	//printf("\n\n");
	auto nodeList = pathFinder.DoFindPath(startPos, endPos);
	//tileMap.Display(nodeList);


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

void Enemy::AstarMove(int x, int y, int z, int w)
{
	if (DELTA_TIME > 1)
		return;
	Vec3 pos = GetTransform()->GetLocalPosition();

	x = x - z;
	y = y - w;
	if (x == -1 && y == 0)
	{
		GetTransform()->SetLocalRotation(Vec3(0, py * 0.5, 0));
		
		pos += GetTransform()->GetLook() * _speed *DELTA_TIME; // 보는 방향기준 오른쪽
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
		
		pos += GetTransform()->GetLook() * _speed *DELTA_TIME; // 보는 방향기준 왼쪽
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

	GetTransform()->SetLocalPosition(pos);

}

void Enemy::LookPlayer()
{

	shared_ptr<Scene> scene = GET_SINGLE(SceneManager)->GetActiveScene();
	Vec3 PlayerPos = scene->GetPlayerPosToEnemy();
	Vec3 EPos = GetEnemyPosition();
	int x = round(EPos.x - PlayerPos.x);
	int z = round(EPos.z - PlayerPos.z);
	float dis = round(sqrt(pow(EPos.x - PlayerPos.x, 2) + pow(EPos.z - PlayerPos.z, 2)));
	float ros = std::acos(-z / dis);
	float radian = ros * 180 / 3.14;
	if (x < 0)
	{
		GetTransform()->SetLocalRotation(Vec3(0, ros, 0));
	}
	else if (x >= 0)
	{
		GetTransform()->SetLocalRotation(Vec3(0, -ros, 0));
	}
}

void Enemy::SetPlayerPos()
{
	for(int i=0; i<100; i++)
		for (int j = 0; j < 100; j++)
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

	k = (k / 300 * 4)-1;
	l = (l / 300*4)+1;

	int x = (int)k;
	int y = (int)l;
	tileMap[y][x] = 3;

	Vec3 EPos = GetEnemyPosition();

	

	if (EPos.z > 0)
		EPos.z = 0;

	double p = EPos.x;
	double q= -(EPos.z);


	p = (p / 300 * 4) - 1;
	q = (q / 300 * 4) + 1;
	

	x = (int)p;
	y = (int)q;
	tileMap[y][x] = 2;

	int w = x + y;
}

void Enemy::Animation()
{
	if (WalkState == true)
	{
		int32 count = GetAnimator()->GetAnimCount();
		//int32 currentIndex = GetAnimator()->GetCurrentClipIndex();

		int32 index = 1 % count;
		GetAnimator()->Play(index);
		WalkState = false;
		Attack = true;
	}
}

void Enemy::DeathAnimation()
{
	if (Dead == false)
	{
		int32 count = GetAnimator()->GetAnimCount();
		int32 currentIndex = GetAnimator()->GetCurrentClipIndex();

		int32 index = 3 % count;
		GetAnimator()->Play(index);
		Dead = true;
		SetDead(true);
	}

	else if (Dead==true)
	{
		DieTime += DELTA_TIME;
		if (DieTime > 2.5)
		{
			GetAnimator()->Stop();
		}
	}
}

void Enemy::AttackAnimation()
{
	if (Attack == true)
	{
		AttackTime = 3.0f;
	}
	AttackTime += DELTA_TIME;
	if (AttackTime > 3)
	{
		int32 count = GetAnimator()->GetAnimCount();
		int32 currentIndex = GetAnimator()->GetCurrentClipIndex();
		int32 index = 2 % count;
		GetAnimator()->Play(index);
		AttackTime = 0;
		Attack = false;
		AttackDelay = 0;
		Moving = 0;
		SetAttack(false);
		LookPlayer();
	}
}



int(*Enemy::CreateMap())[Height]
{
	return tileMap;
}

void Enemy::LostHp(int damage)
{
	_hp -= damage;
}

void Enemy::Respone()
{
	shared_ptr<Scene> scene = GET_SINGLE(SceneManager)->GetActiveScene();
	Vec3 PlayerPos = scene->GetPlayerPosToEnemy();

	srand((unsigned int)time(NULL));

	if (PlayerPos.x > 3500)
	{
		if (PlayerPos.z < -3500)
		{
			ResponeNumber = rand() % 6 + 4;
			
			CheckPoint = ResponeNumber;
			
		}
		else if (PlayerPos.z >= -3500)
		{
			ResponeNumber = rand() % 9 + 1;
			if (ResponeNumber != 4)
			{
				CheckPoint = ResponeNumber;
			}
			else if (ResponeNumber == 4)
			{
				CheckPoint = 5;
			}
		}
	}
	else if (PlayerPos.x <= 3500)
	{
		if (PlayerPos.z < -3500)
		{
			ResponeNumber = rand() % 9 + 1;
			if (ResponeNumber != 5)
			{
				CheckPoint = ResponeNumber;
			}
			else if (ResponeNumber == 5)
			{
				CheckPoint = 4;
			}
		}
		else if (PlayerPos.z >= -3500)
		{
			ResponeNumber = rand() % 5 + 1;
			
			CheckPoint = ResponeNumber;
			
		}
	}

	//체크용 

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
	case 5:
		GetTransform()->SetLocalPosition(ResponeArea5);
		break;
	case 6:
		GetTransform()->SetLocalPosition(ResponeArea6);
		break;
	case 7:
		GetTransform()->SetLocalPosition(ResponeArea7);
		break;
	case 8:
		GetTransform()->SetLocalPosition(ResponeArea8);
		break;
	case 9:
		GetTransform()->SetLocalPosition(ResponeArea9);
		break;
	default:
		break;
	}

	ResponeCheck = true;
}

