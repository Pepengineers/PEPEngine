#pragma once
#include <filesystem>
#include <cstdint>
#include "nlohmann/json.hpp"

#define SCHEMA_VERSION 1.0f

using json = nlohmann::json;

namespace PEPEngine::Common
{
	

	struct AssetType
	{
		enum Type : unsigned int
		{
			None,
			Image,
			Model,
			Material,
			Level
		};
	};


	class Asset
	{
		friend class AssetDatabase;
	protected:
		std::filesystem::path pathToFile;

		uint64_t ID = UINT64_MAX;

		AssetType::Type type;

		void SerializeIDAndType(json& serializer);

		void DeserializeIDAndType(const json& json);
	public:

		virtual void Remove();

		std::wstring GetName() const;

		std::filesystem::path GetPepeFilePath() const;

		Asset(AssetType::Type type = AssetType::Type::None);

		Asset(uint64_t ID, std::filesystem::path pathToFile, AssetType::Type type = AssetType::None);


		virtual ~Asset();

		//TODO: � ������ ��� ������ ���������� ������������, ����� �������� ��� ���� ��� � ������ �� ��� �����, �� �� � ���� ���
		virtual void Serialize(json& json);

		//TODO: � ������ ��� ������ ���������� ������������, ����� �������� ��� ���� ��� � ������ �� ��� �����, �� �� � ���� ���
		virtual void Deserialize(json& json);


		static void WriteToFile(const std::filesystem::path& pathToFile, const json& j);

		static void ReadFromFile(const std::filesystem::path& pathToFile, json& j);

		void UpdatePepe();

		static std::filesystem::path FindNativeFile(std::filesystem::path pathToFile);

		uint64_t GetID() const;

		template <typename T>
		static bool TryReadVariable(const json& json, std::string varName, T* variable)
		{
			if (json.contains(varName))
			{
				*variable = json[varName].get<T>();
				return true;
			}
			assert(varName.c_str());
			return false;
		}

		static std::filesystem::path GetFilePath(const Asset& asset, std::wstring extension);
	};
}
