#include "pch.h"
#include "MeshData.h"
#include "FBXLoader.h"
#include "Mesh.h"
#include "Material.h"
#include "Resources.h"
#include "Transform.h"
#include "MeshRenderer.h"
#include "Animator.h"

MeshData::MeshData() : Object(OBJECT_TYPE::MESH_DATA)
{
}

MeshData::~MeshData()
{
}

shared_ptr<MeshData> MeshData::LoadFromFBX(const wstring& path)
{
	FBXLoader loader;

	shared_ptr<MeshData> meshData = make_shared<MeshData>();


	meshData->Load(path);
	if (meshData->_find == true)
	{
		return meshData;
	}
	else
		loader.LoadFbx(path);


	for (int32 i = 0; i < loader.GetMeshCount(); i++)
	{
		shared_ptr<Mesh> mesh = Mesh::CreateFromFBX(&loader.GetMesh(i), loader);

		GET_SINGLE(Resources)->Add<Mesh>(mesh->GetName(), mesh);

		// Material 찾아서 연동
		vector<shared_ptr<Material>> materials;
 		for (size_t j = 0; j < loader.GetMesh(i).materials.size(); j++)
		{
			shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(loader.GetMesh(i).materials[j].name);
			materials.push_back(material);
		}

		MeshRenderInfo info = {};
		info.mesh = mesh;
		info.materials = materials;
		meshData->_meshRenders.push_back(info);
	}
	const wstring pathSave = L"";
	return meshData;
}

void MeshData::Load(const wstring& _strFilePath)
{
	wstring path = fs::path(_strFilePath).parent_path().wstring() + L"\\" + fs::path(_strFilePath).filename().stem().wstring() + L".fbx";
	Import(path);
	LoadBones(_scene->GetRootNode());
	LoadAnimationInfo();

	if (fs::path(_strFilePath).extension() != ".bin")
	{
		_find = false;
		return;
	}

#pragma region ParseNode Parts
	{
		DWORD dwBytes;
		HANDLE file;

		file = CreateFile(_strFilePath.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (file == INVALID_HANDLE_VALUE)
		{
			_find = false;
			return;
		}

		int32 meshcount = 0;
		ReadFile(file, &meshcount, sizeof(int32), &dwBytes, NULL);

		for (int k = 0; k < meshcount; k++)
		{
			FbxMeshInfo meshinfo;

			size_t nameSize;
			ReadFile(file, &nameSize, sizeof(size_t), &dwBytes, NULL);
			wchar_t* NameString = new wchar_t[nameSize + 1];
			ReadFile(file, NameString, sizeof(wchar_t) * nameSize, &dwBytes, NULL);
			meshinfo.name = wstring{ NameString,nameSize };

			size_t vertexSize;
			ReadFile(file, &vertexSize, sizeof(size_t), &dwBytes, NULL);
			for (int i = 0; i < vertexSize; i++)
			{
				Vertex vertex;
				ReadFile(file, &vertex, sizeof(Vertex), &dwBytes, NULL);
				meshinfo.vertices.push_back(vertex);
			}

			size_t indexSize;
			ReadFile(file, &indexSize, sizeof(size_t), &dwBytes, NULL);
			for (int i = 0; i < indexSize; i++)
			{
				meshinfo.indices.resize(indexSize);
				size_t indexSize2;
				ReadFile(file, &indexSize2, sizeof(size_t), &dwBytes, NULL);
				for (int j = 0; j < indexSize2; j++)
				{
					uint32 index;
					ReadFile(file, &index, sizeof(uint32), &dwBytes, NULL);
					meshinfo.indices[i].push_back(index);
				}
			}

			size_t materialSize;
			ReadFile(file, &materialSize, sizeof(size_t), &dwBytes, NULL);
			meshinfo.materials.resize(materialSize);
			for (int i = 0; i < materialSize; i++)
			{
				ReadFile(file, &meshinfo.materials[i].diffuse, sizeof(Vec4), &dwBytes, NULL);
				ReadFile(file, &meshinfo.materials[i].ambient, sizeof(Vec4), &dwBytes, NULL);
				ReadFile(file, &meshinfo.materials[i].specular, sizeof(Vec4), &dwBytes, NULL);

				size_t NameSize;
				ReadFile(file, &NameSize, sizeof(size_t), &dwBytes, NULL);
				wchar_t* nameString = new wchar_t[NameSize];
				ReadFile(file, nameString, sizeof(wchar_t) * NameSize, &dwBytes, NULL);
				meshinfo.materials[i].name = wstring{ nameString,NameSize };

				size_t diffuseNameSize;
				ReadFile(file, &diffuseNameSize, sizeof(size_t), &dwBytes, NULL);
				wchar_t* diffuseNameString = new wchar_t[diffuseNameSize];
				ReadFile(file, diffuseNameString, sizeof(wchar_t) * diffuseNameSize, &dwBytes, NULL);
				meshinfo.materials[i].diffuseTexName = wstring{ diffuseNameString,diffuseNameSize };

				size_t normalNameSize;
				ReadFile(file, &normalNameSize, sizeof(size_t), &dwBytes, NULL);
				wchar_t* normalNameString = new wchar_t[normalNameSize];
				ReadFile(file, normalNameString, sizeof(wchar_t) * normalNameSize, &dwBytes, NULL);
				meshinfo.materials[i].normalTexName = wstring{ normalNameString,normalNameSize };

				size_t specularNameSize;
				ReadFile(file, &specularNameSize, sizeof(size_t), &dwBytes, NULL);
				wchar_t* specularNameString = new wchar_t[specularNameSize];
				ReadFile(file, specularNameString, sizeof(wchar_t) * specularNameSize, &dwBytes, NULL);
				meshinfo.materials[i].specularTexName = wstring{ specularNameString,specularNameSize };
			}

			size_t boneWeightSize;
			ReadFile(file, &boneWeightSize, sizeof(size_t), &dwBytes, NULL);
			meshinfo.boneWeights.resize(boneWeightSize);
			for (int i = 0; i < boneWeightSize; i++)
			{
				size_t boneWeightSize2;
				ReadFile(file, &boneWeightSize2, sizeof(size_t), &dwBytes, NULL);
				meshinfo.boneWeights[i].boneWeights.resize(boneWeightSize2);
				for (int j = 0; j < boneWeightSize2; j++)
				{
					pair<int32, double> boneWeight;
					ReadFile(file, &boneWeight, sizeof(int32) + sizeof(double), &dwBytes, NULL);
					meshinfo.boneWeights[i].boneWeights.push_back(boneWeight);
				}
			}
			ReadFile(file, &meshinfo.hasAnimation, sizeof(bool), &dwBytes, NULL);

			_meshes.push_back(meshinfo);
		}

		size_t animClipCount = 0;
		ReadFile(file, &animClipCount, sizeof(size_t), &dwBytes, NULL);

		for (int k = 0; k < animClipCount; k++)
		{
			size_t nameSize = 0;
			ReadFile(file, &nameSize, sizeof(size_t), &dwBytes, NULL);
			wchar_t* NameString = new wchar_t[nameSize + 1];
			ReadFile(file, NameString, sizeof(wchar_t) * nameSize, &dwBytes, NULL);
			_animClips[k]->name = wstring{ NameString,nameSize };

			ReadFile(file, &_animClips[k]->startTime, sizeof(FbxTime), &dwBytes, NULL);
			ReadFile(file, &_animClips[k]->endTime, sizeof(FbxTime), &dwBytes, NULL);
			ReadFile(file, &_animClips[k]->mode, sizeof(FbxTime::EMode), &dwBytes, NULL);

			size_t animCount = 0;
			ReadFile(file, &animCount, sizeof(size_t), &dwBytes, NULL);
			for (int i = 0; i < animCount; i++)
			{
				_animClips[k]->keyFrames.resize(animCount);
				size_t animCount2 = 0;
				ReadFile(file, &animCount2, sizeof(size_t), &dwBytes, NULL);
				for (int j = 0; j < animCount2; j++)
				{
					FbxKeyFrameInfo keyframe;
					ReadFile(file, &keyframe, sizeof(FbxKeyFrameInfo), &dwBytes, NULL);
					_animClips[k]->keyFrames[i].push_back(keyframe);
				}
			}
		}

		size_t boneCount = 0;
		ReadFile(file, &boneCount, sizeof(size_t), &dwBytes, NULL);
		for (int k = 0; k < boneCount; k++)
		{
			size_t nameSize = 0;
			ReadFile(file, &nameSize, sizeof(size_t), &dwBytes, NULL);
			wchar_t* NameString = new wchar_t[nameSize + 1];
			ReadFile(file, NameString, sizeof(wchar_t) * nameSize, &dwBytes, NULL);
			_bones[k]->boneName = wstring{ NameString,nameSize };

			ReadFile(file, &_bones[k]->parentIndex, sizeof(int32), &dwBytes, NULL);
			ReadFile(file, &_bones[k]->matOffset, sizeof(FbxAMatrix), &dwBytes, NULL);
		}
		CloseHandle(file);
	}
#pragma endregion

	CreateTextures();
	CreateMaterials();

	for (int32 i = 0; i < _meshes.size(); i++)
	{
		shared_ptr<Mesh> mesh = Mesh::CreateFromFBX(&_meshes[i], _animClips, _bones);

		GET_SINGLE(Resources)->Add<Mesh>(mesh->GetName(), mesh);

		// Material 찾아서 연동
		vector<shared_ptr<Material>> materials;
		for (size_t j = 0; j < _meshes[i].materials.size(); j++)
		{
			shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(_meshes[i].materials[j].name);
			materials.push_back(material);
		}

		MeshRenderInfo info = {};
		info.mesh = mesh;
		info.materials = materials;
		_meshRenders.push_back(info);
	}


	const wstring pathSave = L"";
}

void MeshData::Save(const wstring& _strFilePath)
{
	FBXLoader loader;
	loader.LoadFbx(_strFilePath);
	DWORD dwBytes;
	HANDLE file;

	file = CreateFile(L"..\\Resources\\FBX\\test.bin", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	int32 meshcount = loader.GetMeshCount();
	WriteFile(file, &meshcount, sizeof(int32), &dwBytes, NULL);
	for (int i = 0; i < loader.GetMeshCount(); i++)
	{
		wstring wstr = loader.GetMesh(i).name;
		size_t nameSize =  wstr.size();
		WriteFile(file, &nameSize, sizeof(size_t), &dwBytes, NULL);
		WriteFile(file, wstr.c_str(), sizeof(wchar_t) * nameSize, &dwBytes, NULL);


		size_t vertexSize = loader.GetMesh(i).vertices.size();
		WriteFile(file, &vertexSize, sizeof(size_t), &dwBytes, NULL);
		for (auto& vertex : loader.GetMesh(i).vertices)
		{
			WriteFile(file, &vertex, sizeof(Vertex), &dwBytes, NULL);
		}

		size_t indexSize = loader.GetMesh(i).indices.size();
		WriteFile(file, &indexSize, sizeof(size_t), &dwBytes, NULL);
		for (auto& index : loader.GetMesh(i).indices)
		{
			size_t indexSize2 = index.size();
			WriteFile(file, &indexSize2, sizeof(size_t), &dwBytes, NULL);
			for (auto& index2 : index)
			{
				WriteFile(file, &index2, sizeof(uint32), &dwBytes, NULL);
			}
		}

		size_t materialSize = loader.GetMesh(i).materials.size();
		WriteFile(file, &materialSize, sizeof(size_t), &dwBytes, NULL);
		for (auto& material : loader.GetMesh(i).materials)
		{
			Vec4 diffuse = material.diffuse;
			WriteFile(file, &diffuse, sizeof(Vec4), &dwBytes, NULL);
			Vec4 ambient = material.ambient;
			WriteFile(file, &ambient, sizeof(Vec4), &dwBytes, NULL);
			Vec4 specular = material.specular;
			WriteFile(file, &specular, sizeof(Vec4), &dwBytes, NULL);

			wstring wstr = material.name;
			size_t NameSize = wstr.size();
			WriteFile(file, &NameSize, sizeof(size_t), &dwBytes, NULL);
			WriteFile(file, wstr.c_str(), sizeof(wchar_t) * NameSize, &dwBytes, NULL);

			wstr = material.diffuseTexName;
			NameSize = wstr.size();
			WriteFile(file, &NameSize, sizeof(size_t), &dwBytes, NULL);
			WriteFile(file, wstr.c_str(), sizeof(wchar_t) * NameSize, &dwBytes, NULL);

			wstr = material.normalTexName;
			NameSize = wstr.size();
			WriteFile(file, &NameSize, sizeof(size_t), &dwBytes, NULL);
			WriteFile(file, wstr.c_str(), sizeof(wchar_t) * NameSize, &dwBytes, NULL);

			wstr = material.specularTexName;
			NameSize = wstr.size();
			WriteFile(file, &NameSize, sizeof(size_t), &dwBytes, NULL);
			WriteFile(file, wstr.c_str(), sizeof(wchar_t) * NameSize, &dwBytes, NULL);
		}

		size_t boneWeightSize = loader.GetMesh(i).boneWeights.size();
		WriteFile(file, &boneWeightSize, sizeof(size_t), &dwBytes, NULL);
		for (auto& boneWeight : loader.GetMesh(i).boneWeights)
		{
			size_t boneWeightSize2 = boneWeight.boneWeights.size();
			WriteFile(file, &boneWeightSize2, sizeof(size_t), &dwBytes, NULL);
			for (auto& boneWeight2 : boneWeight.boneWeights)
			{
				WriteFile(file, &boneWeight2, sizeof(int32) + sizeof(double), &dwBytes, NULL);
			}
		}
		WriteFile(file, &loader.GetMesh(i).hasAnimation, sizeof(bool), &dwBytes, NULL);
	}

	size_t animCount = loader.GetAnimClip().size();
	WriteFile(file, &animCount, sizeof(size_t), &dwBytes, NULL);
	for (auto& animclip : loader.GetAnimClip())
	{
		wstring wstr = animclip->name;
		size_t nameSize = wstr.size();
		WriteFile(file, &nameSize, sizeof(size_t), &dwBytes, NULL);
		WriteFile(file, wstr.c_str(), sizeof(wchar_t) * nameSize, &dwBytes, NULL);
		
		WriteFile(file, &animclip->startTime, sizeof(FbxTime), &dwBytes, NULL);
		WriteFile(file, &animclip->endTime, sizeof(FbxTime), &dwBytes, NULL);
		WriteFile(file, &animclip->mode, sizeof(FbxTime::EMode), &dwBytes, NULL);

		size_t animCount = animclip->keyFrames.size();
		WriteFile(file, &animCount, sizeof(animCount), &dwBytes, NULL);
		for (auto& keyFrameCount : animclip->keyFrames)
		{
			size_t animCount2 = keyFrameCount.size();
			WriteFile(file, &animCount2, sizeof(animCount2), &dwBytes, NULL);
			for (auto& keyFrameCount2 : keyFrameCount)
			{
				WriteFile(file, &keyFrameCount2, sizeof(FbxKeyFrameInfo), &dwBytes, NULL);
			}
		}
	}

	size_t boneCount = loader.GetBones().size();
	WriteFile(file, &boneCount, sizeof(size_t), &dwBytes, NULL);
	for (auto& boneInfo : loader.GetBones())
	{
		wstring wstr = boneInfo->boneName;
		size_t nameSize = wstr.size();
		WriteFile(file, &nameSize, sizeof(size_t), &dwBytes, NULL);
		WriteFile(file, wstr.c_str(), sizeof(wchar_t) * nameSize, &dwBytes, NULL);

		WriteFile(file, &boneInfo->parentIndex, sizeof(int32), &dwBytes, NULL);
		WriteFile(file, &boneInfo->matOffset, sizeof(FbxAMatrix), &dwBytes, NULL);
	}

	CloseHandle(file);
}

vector<shared_ptr<GameObject>> MeshData::Instantiate()
{
	vector<shared_ptr<GameObject>> v;

	for (MeshRenderInfo& info : _meshRenders)
	{
		shared_ptr<GameObject> gameObject = make_shared<GameObject>();
		gameObject->AddComponent(make_shared<Transform>());
		gameObject->AddComponent(make_shared<MeshRenderer>());
		gameObject->GetMeshRenderer()->SetMesh(info.mesh);

		for (uint32 i = 0; i < info.materials.size(); i++)
			gameObject->GetMeshRenderer()->SetMaterial(info.materials[i]->Clone(), i);

		if (info.mesh->IsAnimMesh())
		{
			shared_ptr<Animator> animator = make_shared<Animator>();
			gameObject->AddComponent(animator);
			animator->SetBones(info.mesh->GetBones());
			animator->SetAnimClip(info.mesh->GetAnimClip());
		}
		v.push_back(gameObject);
	}
	return v;
}

void MeshData::Import(const wstring& path)
{
	// FBX SDK 관리자 객체 생성
	_manager = FbxManager::Create();

	// IOSettings 객체 생성 및 설정
	FbxIOSettings* settings = FbxIOSettings::Create(_manager, IOSROOT);
	_manager->SetIOSettings(settings);

	// FbxImporter 객체 생성
	_scene = FbxScene::Create(_manager, "");

	// 나중에 Texture 경로 계산할 때 쓸 것
	_resourceDirectory = fs::path(path).parent_path().wstring() + L"\\" + fs::path(path).filename().stem().wstring() + L".fbm";

	_importer = FbxImporter::Create(_manager, "");

	string strPath = ws2s(path);
	_importer->Initialize(strPath.c_str(), -1, _manager->GetIOSettings());

	_importer->Import(_scene);

	_scene->GetGlobalSettings().SetAxisSystem(FbxAxisSystem::DirectX);

	// 씬 내에서 삼각형화 할 수 있는 모든 노드를 삼각형화 시킨다.
	FbxGeometryConverter geometryConverter(_manager);
	geometryConverter.Triangulate(_scene, true);

	_importer->Destroy();
}

void MeshData::LoadBones(FbxNode* node, int32 idx, int32 parentIdx)
{
	FbxNodeAttribute* attribute = node->GetNodeAttribute();

	if (attribute && attribute->GetAttributeType() == FbxNodeAttribute::eSkeleton)
	{
		shared_ptr<FbxBoneInfo> bone = make_shared<FbxBoneInfo>();
		bone->boneName = s2ws(node->GetName());
		bone->parentIndex = parentIdx;
		_bones.push_back(bone);
	}

	const int32 childCount = node->GetChildCount();
	for (int32 i = 0; i < childCount; i++)
		LoadBones(node->GetChild(i), static_cast<int32>(_bones.size()), idx);
}

void MeshData::LoadAnimationInfo()
{
	_scene->FillAnimStackNameArray(OUT _animNames);

	const int32 animCount = _animNames.GetCount();
	for (int32 i = 0; i < animCount; i++)
	{
		FbxAnimStack* animStack = _scene->FindMember<FbxAnimStack>(_animNames[i]->Buffer());
		if (animStack == nullptr)
			continue;

		shared_ptr<FbxAnimClipInfo> animClip = make_shared<FbxAnimClipInfo>();
		animClip->name = s2ws(animStack->GetName());
		animClip->keyFrames.resize(_bones.size()); // 키프레임은 본의 개수만큼

		FbxTakeInfo* takeInfo = _scene->GetTakeInfo(animStack->GetName());
		animClip->startTime = takeInfo->mLocalTimeSpan.GetStart();
		animClip->endTime = takeInfo->mLocalTimeSpan.GetStop();
		animClip->mode = _scene->GetGlobalSettings().GetTimeMode();

		_animClips.push_back(animClip);
	}
}

void MeshData::CreateTextures()
{
	for (size_t i = 0; i < _meshes.size(); i++)
	{
		for (size_t j = 0; j < _meshes[i].materials.size(); j++)
		{
			// DiffuseTexture
			{
				wstring relativePath = _meshes[i].materials[j].diffuseTexName.c_str();
				wstring filename = fs::path(relativePath).filename();
				wstring fullPath = _resourceDirectory + L"\\" + filename;
				if (filename.empty() == false)
					GET_SINGLE(Resources)->Load<Texture>(filename, fullPath);
			}

			// NormalTexture
			{
				wstring relativePath = _meshes[i].materials[j].normalTexName.c_str();
				wstring filename = fs::path(relativePath).filename();
				wstring fullPath = _resourceDirectory + L"\\" + filename;
				if (filename.empty() == false)
					GET_SINGLE(Resources)->Load<Texture>(filename, fullPath);
			}

			// SpecularTexture
			{
 				wstring relativePath = _meshes[i].materials[j].specularTexName.c_str();
				wstring filename = fs::path(relativePath).filename();
				wstring fullPath = _resourceDirectory + L"\\" + filename;
				if (filename.empty() == false)
					GET_SINGLE(Resources)->Load<Texture>(filename, fullPath);
			}
		}
	}
}

void MeshData::CreateMaterials()
{
	for (size_t i = 0; i < _meshes.size(); i++)
	{
		for (size_t j = 0; j < _meshes[i].materials.size(); j++)
		{
			shared_ptr<Material> material = make_shared<Material>();
			wstring key = _meshes[i].materials[j].name;
			material->SetName(key);
			material->SetShader(GET_SINGLE(Resources)->Get<Shader>(L"Deferred"));

			{
				wstring diffuseName = _meshes[i].materials[j].diffuseTexName.c_str();
				wstring filename = fs::path(diffuseName).filename();
				wstring key = filename;
				shared_ptr<Texture> diffuseTexture = GET_SINGLE(Resources)->Get<Texture>(key);
				if (diffuseTexture)
					material->SetTexture(0, diffuseTexture);
			}

			{
				wstring normalName = _meshes[i].materials[j].normalTexName.c_str();
				wstring filename = fs::path(normalName).filename();
				wstring key = filename;
				shared_ptr<Texture> normalTexture = GET_SINGLE(Resources)->Get<Texture>(key);
				if (normalTexture)
					material->SetTexture(1, normalTexture);
			}

			{
				wstring specularName = _meshes[i].materials[j].specularTexName.c_str();
				wstring filename = fs::path(specularName).filename();
				wstring key = filename;
				shared_ptr<Texture> specularTexture = GET_SINGLE(Resources)->Get<Texture>(key);
				if (specularTexture)
					material->SetTexture(2, specularTexture);
			}

			GET_SINGLE(Resources)->Add<Material>(material->GetName(), material);
		}
	}
}