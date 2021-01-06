#pragma once
#include "Component.h"
#include "d3dUtil.h"
#include "GCommandList.h"
#include "ShaderBuffersData.h"

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

			

			void Serialize(json& j) override
			{
				j["Type"] = ComponentID;

				auto jPos = json();
				jPos["Type"] = Type;				
				
				auto jType = json();  ;
				jType["x"] = Direction.x;
				jType["y"] = Direction.y;
				jType["z"] = Direction.z;
				
				jPos["Direction"] = jType;
				
				jType = json(); 
				jType["x"] = Color.x;
				jType["y"] = Color.y;
				jType["z"] = Color.z;
				jType["w"] = Color.w;
				jPos["Color"] = jType;
				
				jPos["SpotlightAngle"] = SpotlightAngle;
				jPos["Range"] = Range;
				jPos["Intensity"] = Intensity;
				jPos["Enabled"] = Enabled;

				j["LightData"] = jPos;
			};

			void Deserialize(json& j) override
			{
				auto jPos = j["LightData"];
				Type = jPos["Type"];
				
				float x, y, z, w;

				auto jType = jPos["Direction"];
				assert(TryReadVariable<float>(jType, "x", &x));
				assert(TryReadVariable<float>(jType, "y", &y));
				assert(TryReadVariable<float>(jType, "z", &z));
				Direction = Vector3(x, y, z);


				jType = jPos["Color"];
				assert(TryReadVariable<float>(jType, "x", &x));
				assert(TryReadVariable<float>(jType, "y", &y));
				assert(TryReadVariable<float>(jType, "z", &z));
				assert(TryReadVariable<float>(jType, "w", &w));
				Color = Vector4(x, y, z,w);
				
				
				assert(TryReadVariable<float>(jPos, "SpotlightAngle", &SpotlightAngle));
				assert(TryReadVariable<float>(jPos, "Range", &Range));
				assert(TryReadVariable<float>(jPos, "Intensity", &Intensity));
				assert(TryReadVariable<bool>(jPos, "Enabled", &Enabled));

			};

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

			Light()
				: Type(Spot)
				  , Direction(0, 0, -1)
				  , Color(1, 1, 1, 1)
				  , SpotlightAngle(45.0f)
				  , Range(5)
				  , Intensity(1.0f)
				  , Enabled(true)
				  , Selected(false)
			{
			}

			//, type(LightType::Point)

			Matrix& GetViewMatrix() const;


			LightData& GetData();
		};
	}
}
