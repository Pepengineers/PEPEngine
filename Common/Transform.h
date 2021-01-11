#pragma once
#include "Component.h"
#include "SimpleMath.h"
#include "DirectXBuffers.h"


using namespace DirectX::SimpleMath;

namespace PEPEngine
{
	namespace Common
	{
		using namespace Allocator;
		using namespace Graphics;
		using namespace Utils;

		class Transform : public Component
		{
			friend class GameObject;
			
		public:
			Transform(Vector3 pos, Quaternion rot, Vector3 scale);

			Transform();

			void SetPosition(const Vector3& pos);

			void SetScale(const Vector3& s);

			void SetEulerRotate(const Vector3& eulerAngl);
			void SetRadianRotate(const Vector3& radianAngl);
			void SetMatrixRotate(const Matrix& rot);
			void SetQuaternionRotate(const Quaternion& quaternion);

			void AdjustPosition(const Vector3& pos);

			void AdjustPosition(float x, float y, float z);

			void AdjustEulerRotation(const Vector3& eulerAngl);

			void AdjustEulerRotation(float roll, float pitch, float yaw);

			[[nodiscard]] Vector3 GetWorldPosition() const;

			[[nodiscard]] Vector3 GetScale() const;

			[[nodiscard]] Quaternion GetQuaternionRotate() const;
			[[nodiscard]] Vector3 GetEulerAngels() const;;

			Matrix TextureTransform = Matrix::CreateScale(Vector3::One);
			Matrix worldTranspose;

			bool IsDirty() const;

			void Update() override;;

			void SetParent(Transform* transform);

			Vector3 GetForwardVector() const;

			Vector3 GetBackwardVector() const;

			Vector3 GetRightVector() const;

			Vector3 GetLeftVector() const;

			Vector3 GetUpVector() const;

			Vector3 GetDownVector() const;

			Matrix GetWorldMatrix() const;
			Matrix MakeLocalToParent() const;

			void SetWorldMatrix(const Matrix& mat);

			SERIALIZE_FROM_JSON(Transform)
		private:

			 void Serialize(json& j) override
			 {
				 j["Type"] = ComponentID;

				 auto jPos = json(); 
				 jPos["x"] = localPosition.x;
				 jPos["y"] = localPosition.y;
				 jPos["z"] = localPosition.z;
				 j["Position"] = jPos;

				 jPos = json();
				 jPos["x"] = localEulerAngles.x;
				 jPos["y"] = localEulerAngles.y;
				 jPos["z"] = localEulerAngles.z;
				 j["Rotation"] = jPos;
			 	
				 jPos = json();
				 jPos["x"] = localScale.x;
				 jPos["y"] = localScale.y;
				 jPos["z"] = localScale.z;

				 j["Scale"] = jPos;
			 };

			void Deserialize(json& j) override
			{
				float x, y, z;
				auto jPos = j["Position"];
				(TryReadVariable<float>(jPos, "x", &x));
				(TryReadVariable<float>(jPos, "y", &y));
				(TryReadVariable<float>(jPos, "z", &z));

				SetPosition(Vector3(x,y,z));

				jPos = j["Rotation"];
				(TryReadVariable<float>(jPos, "x", &x));
				(TryReadVariable<float>(jPos, "y", &y));
				(TryReadVariable<float>(jPos, "z", &z));

				SetEulerRotate( Vector3(x, y, z));

				jPos = j["Scale"];
				(TryReadVariable<float>(jPos, "x", &x));
				(TryReadVariable<float>(jPos, "y", &y));
				(TryReadVariable<float>(jPos, "z", &z));

				SetScale( Vector3(x, y, z));				
			};
			
			Matrix world = Matrix::Identity;


			Matrix MakeParentToLocal() const;
			Matrix CalculateWorldMatrix() const;

			Transform* Parent = nullptr;

			int NumFramesDirty = globalCountFrameResources;

			Vector3 localPosition = Vector3::Zero;

			Vector3 localEulerAngles = Vector3::Zero;

			Quaternion localRotate = Quaternion::Identity;
			Vector3 localScale = Vector3::One;
		};
	}
}
