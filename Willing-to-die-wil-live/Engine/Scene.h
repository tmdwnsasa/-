#pragma once
#include "Fmod.hpp"

class GameObject;

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


private:
	vector<shared_ptr<GameObject>>		_gameObjects;
	vector<shared_ptr<class Camera>>	_cameras;
	vector<shared_ptr<class Light>>		_lights;
	vector<shared_ptr<GameObject>>		_trashBin;

	bool			_shopOpened;

	bool			_clipcursor = false;
	const double	py = std::acos(-1);
	RECT			rc;

	bool			_mouseLock = false;
};

