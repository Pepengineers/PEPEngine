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


	class Asset : std::enable_shared_from_this<Asset>
	{
		friend class AssetDatabase;
	protected:
		std::wstring name;
		std::filesystem::path pathToFile;

		uint64_t ID = UINT64_MAX;

		AssetType::Type type;

		void SerializeIDAndType(json& serializer);

		void DeserializeIDAndType(const json& json);
	public:

		Asset(AssetType::Type type = AssetType::Type::None) : type(type)
		{
		}

		Asset(uint64_t ID, std::filesystem::path pathToFile, AssetType::Type type = AssetType::None);


		virtual ~Asset();

		//TODO: В идеале тут должен принматься сериализатор, чтобы написать код один раз и забыть на всю жизнь, но не в этот раз
		virtual void Serialize(json& json) {};

		//TODO: В идеале тут должен принматься сериализатор, чтобы написать код один раз и забыть на всю жизнь, но не в этот раз
		virtual void Deserialize(json& json) {};


		static void WriteToFile(const std::filesystem::path& pathToFile, const json& j);

		static void ReadFromFile(const std::filesystem::path& pathToFile, json& j);

		void UpdatePepe();

		static std::filesystem::path FindNativeFile(std::filesystem::path pathToFile);

		inline uint64_t GetID() const {
			return ID;
		}

		template <typename T>
		static bool TryReadVariable(const json& json, std::string varName, T* variable)
		{
			if (json.contains(varName))
			{
				*variable = json[varName].get<T>();
				return true;
			}

			return false;
		}

		static std::filesystem::path GetFilePath(const Asset& asset, std::wstring extension);
	};
}
