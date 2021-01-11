#pragma once
#include "Component.h"
#include "d3dUtil.h"
#include "GCommandList.h"
#include "ShaderBuffersData.h"

#include "nlohmann/json.hpp"

using json = nlohmann::json;

namespace PEPEngine
{
	namespace Common
	{
		using namespace Allocator;
		using namespace Utils;


		class Light : public Component
		{
		private:

			int NumFramesDirty = globalCountFrameResources;
			LightData lightData{};
			void Update() override;;
			Matrix view = Matrix::Identity;
			Matrix projection = Matrix::Identity;


			void Serialize(json& j) override;;

			void Deserialize(json& j) override;;

		public:
			SERIALIZE_FROM_JSON(Light)
			
			LightType Type;
			Vector3 Direction;
			Vector4 Color;
			float SpotlightAngle;
			float Range;
			float Intensity;
			bool Enabled;
			bool Selected;


			void Render(std::shared_ptr<Graphics::GCommandList> cmdList);

			Light();

			//, type(LightType::Point)

			Matrix GetViewMatrix() const;


			LightData& GetData();
		};
	}
}
