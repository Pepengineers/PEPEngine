#pragma once
#include <Component.h>

#include "AModel.h"
#include "d3d12.h"
#include "SimpleMath.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;
using namespace PEPEngine::Common;

class SpawnController : public PEPEngine::Common::Component
{
	Vector3 leftPoint;
	Vector3 rightPoint;
	Vector3 spawnModelScale;
	std::shared_ptr<AModel> spawnObjectModel;

	UINT count = 0;
	UINT spawnModelPerSecondCount = 1;
	float tickTime = 1.0f;
	float currentTime = 0;
	
public:
	SERIALIZE_FROM_JSON(SpawnController)

	SpawnController(const std::shared_ptr<AModel> spawnModel, const Vector3 lp, const Vector3 rp,
	                const Vector3 modelScale = Vector3::One);

	void OnGUI() override;;
	
	void Update() override;
	void Serialize(json& j) override;
	void Deserialize(json& j) override;
};

