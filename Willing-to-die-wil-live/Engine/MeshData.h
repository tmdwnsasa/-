#pragma once
#include "Object.h"
#include "FBXLoader.h"

class Mesh;
class Material;
class GameObject;

struct MeshRenderInfo
{
	shared_ptr<Mesh>				mesh;
	vector<shared_ptr<Material>>	materials;
};

class MeshData : public Object
{
public:
	MeshData();
	virtual ~MeshData();

public:
	static shared_ptr<MeshData> LoadFromFBX(const wstring& path);

	virtual void Load(const wstring& path);
	virtual void Save(const wstring& path);

	vector<shared_ptr<GameObject>> Instantiate();


	//For Load
	void Import(const wstring& path);
	void LoadBones(FbxNode* node) { LoadBones(node, 0, -1); }
	void LoadBones(FbxNode* node, int32 idx, int32 parentIdx);
	void LoadAnimationInfo();
	void CreateTextures();
	void CreateMaterials();
	vector<MeshRenderInfo> GetAnimClip() { return _meshRenders; };
	void SetAnimClip(vector<MeshRenderInfo> animclip) {
		int i = 0;
		for (auto& meshrender : _meshRenders)
		{
			_meshRenders[i].mesh = animclip[i].mesh;
			i++;
	} };

private:
	//For Load
	FbxManager*						_manager = nullptr;
	FbxScene*						_scene = nullptr;
	FbxImporter*					_importer = nullptr;
	vector<FbxMeshInfo>				_meshes;
	wstring							_resourceDirectory;
	vector<shared_ptr<FbxBoneInfo>>		_bones;
	vector<shared_ptr<FbxAnimClipInfo>>	_animClips;
	FbxArray<FbxString*>				_animNames;

	shared_ptr<Mesh>				_mesh;
	vector<shared_ptr<Material>>	_materials;

	vector<MeshRenderInfo>			_meshRenders;

	bool							_find = true;
};
