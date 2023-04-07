#pragma once

class GameObject;


static const int Width = 10;
static const int Height = 10;

static int tileMap[Height][Width] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 1, 1, 1, 1, 0, 0, 0, 0,
	0, 0, 1, 0, 0, 1, 0, 0, 0, 0,
	0, 0, 1, 0, 0, 1, 0, 0, 0, 0,
	2, 0, 1, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 1, 0, 0, 1, 3, 0, 0, 0,
	0, 0, 1, 0, 0, 1, 1, 1, 1, 1,
	0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
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

	void Render();

	void ClearRTV();

	void RenderShadow();
	void RenderDeferred();
	void RenderLights();
	void RenderFinal();

	void RenderForward();

	void CursorClipping();
	int (*CreateMap())[Height];
private:
	void PushLightData();

public:
	void AddGameObject(shared_ptr<GameObject> gameObject);
	void RemoveGameObject(shared_ptr<GameObject> gameObject);

	const vector<shared_ptr<GameObject>>& GetGameObjects() { return _gameObjects; }

private:
	vector<shared_ptr<GameObject>>		_gameObjects;
	vector<shared_ptr<class Camera>>	_cameras;
	vector<shared_ptr<class Light>>		_lights;

	bool			_clipcursor = false;
	RECT			rc;
};

