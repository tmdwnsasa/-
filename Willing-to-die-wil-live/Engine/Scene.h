#pragma once
#include "Fmod.hpp"

class GameObject;
class MeshData;

enum class SCENE_STATE : uint8
{
	BATTLE,
	SHOPPING,
	ESCAPE,
	
	
	END
};

class Scene
{
public:
	void Awake();
	void Start();
	void Update();
	void LateUpdate();
	void FinalUpdate();

	shared_ptr<class Camera> GetMainCamera();

	void GetPlayerPosToCam(wstring objectname);
	Vec3 GetPlayerPosToEnemy() { return PlayerObPos; };
	void SetPlayerPosToEnemy();
	Vec3 GetWallPosToEnemy() { return WallObPos; };
	void SetWallPosToEnemy();
	void SetCameraPosToPlayer();

	void Render();

	void ClearRTV();

	void RenderShadow();
	void RenderDeferred();
	void RenderLights();
	void RenderFinal();

	void RenderForward();

	void CursorClipping();
	void CollisionPlayerToWall();

	void EnemyAtk();
	void CheckWave();
	void MakeNormal(int Wave);
	void MakeStalker(int Wave);
	void MakeBruser(int Wave);

private:
	void PushLightData();
	int  DistanceWall = 70;
	int  EnemyHp = 0;
public:
	void AddGameObject(shared_ptr<GameObject> gameObject);
	void RemoveGameObject(shared_ptr<GameObject> gameObject);
	const vector<shared_ptr<GameObject>>& GetGameObjects() { return _gameObjects; }
	Vec3			PlayerObPos;
	Vec3			WallObPos;
	Vec3			PlayerPosition;
	Vec3			EnemyPosition;
	Vec3			BrEnemyPosition;
	Vec3			StEnemyPosition;
	int				CurrentWave = 1;
	int				DeathCount = 0;
	int				EnemyCount = 10;
	int				Wave1 = 5;
	int				Wave2 = 10;
	int				STZCount = 0;
	int				BrZCount = 0;
	int				ZCount = 0;
	int				Total = 0;
	int				FSTZCount = 0;
	int				FBrZCount = 0;
	int				FZCount = 0;

	float			RestTime = 0.0f;
	float			SponeTime = 0.0f;
	bool			IsRest = false;
	float			Distance = 0;

	int				PlayerHp = 0;
	float			Time = 0;
private:
	vector<shared_ptr<GameObject>>		_gameObjects;
	vector<shared_ptr<class Camera>>	_cameras;
	vector<shared_ptr<class Light>>		_lights;
	vector<shared_ptr<GameObject>>		_trashBin;
	shared_ptr<MeshData>				ZombieMesh;
	shared_ptr<MeshData>				StalkerZombieMesh;
	shared_ptr<MeshData>				BruserZombieMesh;

	bool			_shopOpened;
	int				_shopSelectedNum = 1000;

	bool			_clipcursor = false;
	const double	py = std::acos(-1);
	RECT			rc;

	bool			_mouseLock = false;
};

