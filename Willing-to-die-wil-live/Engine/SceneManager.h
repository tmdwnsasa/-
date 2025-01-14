#pragma once

class Scene;

enum
{
	MAX_LAYER = 32
};

class SceneManager
{
	DECLARE_SINGLE(SceneManager);

public:
	void Update();
	void Render();
	void LoadScene(wstring sceneName);

	void SetLayerName(uint8 index, const wstring& name);
	const wstring& IndexToLayerName(uint8 index) { return _layerNames[index]; }
	uint8 LayerNameToIndex(const wstring& name);

	shared_ptr<class GameObject> Pick(int32 screenX, int32 screenY);

	const double py = std::acos(-1);
	const float baseX = 3000;
	const float baseZ = -150;
	const float posScale = 300;
	const float scale = 349;

public:
	shared_ptr<Scene> GetActiveScene() { return _activeScene; }
private:
	void MakeWall(bool horizontal, int value, float xchange, float ychange, shared_ptr<Scene>& scene);
	shared_ptr<Scene> LoadMainScene();
	shared_ptr<Scene> LoadGameScene();
	wstring SceneName;

private:
	shared_ptr<Scene> _activeScene;

	array<wstring, MAX_LAYER> _layerNames;
	map<wstring, uint8> _layerIndex;

};

