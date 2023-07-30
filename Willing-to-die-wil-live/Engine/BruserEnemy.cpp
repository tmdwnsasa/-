#include "pch.h"
#include "BruserEnemy.h"
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


BruserEnemy::BruserEnemy() : Component(COMPONENT_TYPE::BRUSERENEMY)
{

}

BruserEnemy::~BruserEnemy()
{

}

void BruserEnemy::Update()
{

	//pos += GetTransform()->GetLook() * _speed * 10 * DELTA_TIME;
	//pos += GetTransform()->GetRight() * _speed * DELTA_TIME;
	Vec3 pos = GetTransform()->GetLocalPosition();

	shared_ptr<Scene> scene = GET_SINGLE(SceneManager)->GetActiveScene();
	Vec3 PlayerPos = scene->GetPlayerPosToEnemy();

	_distance = sqrt(pow(pos.x - PlayerPos.x, 2) + pow(pos.z - PlayerPos.z, 2));

	switch (_currentState)
	{
	case BruserENEMY_STATE::IDLE:
		break;
	case BruserENEMY_STATE::WALK:
		WalkAnimation();
		break;
	case BruserENEMY_STATE::RUN:
		RunAnimation();
		break;
	case BruserENEMY_STATE::BURSERKER:
		BurserkAnimation();
		break;
	case BruserENEMY_STATE::ATTACK:
		AttackAnimation();
		break;
	case BruserENEMY_STATE::DIE:
		DeathAnimation();
		break;
	case BruserENEMY_STATE::END:
		break;
	default:
		break;
	}

	//1 공격
	//2 광포고하
	//3 죽음
	//4 달리기
	//5 걷기

	//GetTransform()->SetLocalPosition(pos);

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
		if (_distance >= 300)
		{
			Moving += DELTA_TIME;
			if (Moving > 2.0)
			{
				if (Awake)
				{
					AstarMove(firstx, firsty, secondx, secondy);
				}
			}
		}
	}

	if (Dead == false)
	{
		if (_distance >= 300)
		{
			if (Attack == false)
			{
				AttackDelay += DELTA_TIME;
				if (AttackDelay > 2.0)
				{
					SetAttack(true);
					if (BurserkerMode == true)
					{
						SetState(BruserENEMY_STATE::WALK);
					}
					else
					{
						SetState(BruserENEMY_STATE::RUN);
					}
				}
			}
		}
	}

	if (Dead == false)
	{
		if (_distance < 300)
		{
			SetState(BruserENEMY_STATE::ATTACK);
		}
	}

	if (_hp <= 0)
	{
		SetState(BruserENEMY_STATE::DIE);
	}

}


void BruserEnemy::AstarCall()
{
	TileMap tileMap;
	PathFinder pathFinder(&tileMap);

	Vector2 startPos;
	Vector2 endPos;
	tileMap.CreateTile(startPos, endPos, 3);

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

void BruserEnemy::AstarMove(int x, int y, int z, int w)
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



void BruserEnemy::SetPlayerPos()
{
	for (int i = 0; i < 100; i++)
		for (int j = 0; j < 100; j++)
		{
			if (BrtileMap[i][j] == 2)
			{
				BrtileMap[i][j] = 0;
				/*EnemyCount++;
				if (EnemyCount == 10)
				{
					tileMap[i][j] = 0;
					EnemyCount = 0;
				}*/
			}
			else if (BrtileMap[i][j] == 3)
			{
				BrtileMap[i][j] = 0;
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
	BrtileMap[y][x] = 3;

	Vec3 EPos = GetEnemyPosition();

	if (EPos.z > 0)
		EPos.z = 0;

	double p = EPos.x;
	double q = -(EPos.z);


	p = (p / 300 * 4) - 1;
	q = (q / 300 * 4) + 1;


	x = (int)p;
	y = (int)q;
	BrtileMap[y][x] = 2;

	int w = x + y;
}

void BruserEnemy::WalkAnimation()
{
	if (Walk_State == true)
	{
		int32 count = GetAnimator()->GetAnimCount();
		int32 currentIndex = GetAnimator()->GetCurrentClipIndex();

		int32 index = 4 % count;
		GetAnimator()->Play(index);
		Walk_State = false;
	}
}

void BruserEnemy::DeathAnimation()
{
	if (Dead==false)
	{
		int32 count = GetAnimator()->GetAnimCount();
		
		int32 index = 2 % count;
		GetAnimator()->Play(index);
		Dead = true;
	}

	else if (Dead==true)
	{
		DieTime += DELTA_TIME;
		if (DieTime > 2.2)
		{
			GetAnimator()->Stop();
		}
	}
}

void BruserEnemy::RunAnimation()
{
	if (Run_State == false)
	{
		int32 count = GetAnimator()->GetAnimCount();
		int32 index = 3 % count;
		GetAnimator()->Play(index);
		Run_State = true;
	}
}

void BruserEnemy::BurserkAnimation()
{
	if (BurserkerMode==true)
	{
		int32 count = GetAnimator()->GetAnimCount();

		int32 index = 1 % count;
		GetAnimator()->Play(index);
		BurserkerMode = false;
		Awake = false;
	}

	else if (BurserkerMode == false)
	{
		BuffTime += DELTA_TIME;
		if (BuffTime > 2.5)
		{
			_speed = 300;
			_hp = 400;
			SetAtk(40);
			Awake = true;
			SetState(BruserENEMY_STATE::RUN);
		}
	}
}

void BruserEnemy::AttackAnimation()
{
	if (Attack == true)
	{
		AttackTime = 3.0f;
	}
	AttackTime += DELTA_TIME;
	if (AttackTime > 3)
	{
		int32 count = GetAnimator()->GetAnimCount();
		int32 index = 0 % count;
		GetAnimator()->Play(index);
		AttackTime = 0;
		Attack = false;
		Run_State = false;
		Walk_State = true;
		AttackDelay = 0;
		Moving = 0;
		SetAttack(false);
		LookPlayer();
	}
}

void BruserEnemy::LookPlayer()
{
	shared_ptr<Scene> scene = GET_SINGLE(SceneManager)->GetActiveScene();
	Vec3 PlayerPos = scene->GetPlayerPosToEnemy();
	Vec3 EPos = GetEnemyPosition();
	int x = round(EPos.x - PlayerPos.x);
	int z = round(EPos.z - PlayerPos.z);
	float dis = round(sqrt(pow(EPos.x - PlayerPos.x, 2) + pow(EPos.z - PlayerPos.z, 2)));
	float ros = std::acos(z / dis);
	if (x < 0)
	{
		GetTransform()->SetLocalRotation(Vec3(0, ros, 0));
	}
	else if (x > 0)
	{
		GetTransform()->SetLocalRotation(Vec3(0, -ros, 0));
	}
	//세타값은 동일 양 음 설정 필요
	//float radian = ros * 180 / py;
	//cout << "x :" << x << endl;
	//cout << " Z : " << z << endl;
	//cout << "dis : " << dis << endl;
	//cout << radian << endl;
}


int(*BruserEnemy::CreateMap())[BrHeight]
{
	return BrtileMap;
}

void BruserEnemy::LostHp(int damage)
{
	_hp -= damage;
	SetState(BruserENEMY_STATE::BURSERKER);
}


void BruserEnemy::Respone()
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

