#include "pch.h"
#include "ParticleSystem.h"
#include "StructuredBuffer.h"
#include "Mesh.h"
#include "Resources.h"
#include "Transform.h"
#include "Timer.h"
#include <iostream>

ParticleSystem::ParticleSystem() : Component(COMPONENT_TYPE::PARTICLE_SYSTEM)
{
}
ParticleSystem::ParticleSystem(string name) : Component(COMPONENT_TYPE::PARTICLE_SYSTEM)
{
	_particleBuffer = make_shared<StructuredBuffer>();
	_particleBuffer->Init(sizeof(ParticleInfo), _maxParticle);

	_computeSharedBuffer = make_shared<StructuredBuffer>();
	_computeSharedBuffer->Init(sizeof(ComputeSharedInfo), 1);

	_mesh = GET_SINGLE(Resources)->LoadPointMesh();

	
	if (name == "Blood")
	{
		_material = GET_SINGLE(Resources)->Get<Material>(L"Particle");
		shared_ptr<Texture> tex = GET_SINGLE(Resources)->Load<Texture>(
			L"Blood", L"..\\Resources\\Texture\\blood effect.png");
		_material->SetTexture(0, tex);
		particleName = name;
	}

	if (name == "Smoke")
	{
		_material = GET_SINGLE(Resources)->Get<Material>(L"Particle2");
		shared_ptr<Texture> tex = GET_SINGLE(Resources)->Load<Texture>(
			L"Smoke", L"..\\Resources\\Texture\\Smoke.png");
		_material->SetTexture(0, tex);
		particleName = name;
	}

	_computeMaterial = GET_SINGLE(Resources)->Get<Material>(L"ComputeParticle");
}

ParticleSystem::~ParticleSystem()
{
}

void ParticleSystem::Awake()
{
}

void ParticleSystem::Start()
{
	Parameter();
}

void ParticleSystem::Update()
{
	if (particleName == "Blood")
	{
		if (_accTime >= 0.2 && _accTime < 5)
		{
			OnOff(false);
		}
		if (_accTime >= 5)
		{
			_erase = true;
		}
	}
}

void ParticleSystem::LateUpdate()
{
}

void ParticleSystem::FinalUpdate()
{
	_accTime += DELTA_TIME;
	int32 add = 0;
	if (_createInterval < _accTime)
	{
		_accTime = _accTime - _createInterval;
		add = 1;
	}

	_particleBuffer->PushComputeUAVData(UAV_REGISTER::u0);
	_computeSharedBuffer->PushComputeUAVData(UAV_REGISTER::u1);

	_computeMaterial->SetInt(0, _maxParticle);
	_computeMaterial->SetInt(1, add);
	if (particleName == "Blood")
	{
		_computeMaterial->SetInt(3, 1);
	}
	if (particleName == "Smoke")
	{
		_computeMaterial->SetInt(3, 0);
	}

	_computeMaterial->SetVec2(1, Vec2(DELTA_TIME, _accTime));
	_computeMaterial->SetVec4(0, Vec4(_minLifeTime, _maxLifeTime, _minSpeed, _maxSpeed));

	_computeMaterial->Dispatch(1, 1, 1);
}

void ParticleSystem::Render()
{
	GetTransform()->PushData();

	_particleBuffer->PushGraphicsData(SRV_REGISTER::t9);
	_material->SetFloat(0, _startScale);
	_material->SetFloat(1, _endScale);
	_material->PushGraphicsData();

	_mesh->Render(_maxParticle);
}

void ParticleSystem::Parameter()
{
	if (particleName == "Blood")
	{
		_createInterval = 0.05f;
		_minLifeTime = 0.5f;
		_maxLifeTime = 2.f;
		_minSpeed = 60;
		_maxSpeed = 30;
		_startScale = 20.f;
		_endScale = 3.f;
	}
	if (particleName == "Smoke")
	{
		_createInterval = 0.05f;
		_minLifeTime = 100.f;
		_maxLifeTime = 100.f;
		_minSpeed = 0;
		_maxSpeed = 0;
		_startScale = 1000.f;
		_endScale = 1000.f;
	}
}

void ParticleSystem::OnOff(bool state)
{
	if (state == true)
	{
		_state = true;
		if (particleName == "Blood")
		{
			_createInterval = 10.f;

		}
		if (particleName == "Smoke")
		{
			_createInterval = 0.3f;

		}
	}
	if (state == false)
	{
		_state = false;
		_createInterval = 10000.f;
	}
}