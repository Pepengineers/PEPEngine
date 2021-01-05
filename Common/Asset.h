#pragma once
#include <filesystem>
#include "nlohmann/json.hpp"

#define SCHEMA_VERSION 1.0f


namespace PEPEngine::Common
{
	using json = nlohmann::json;

	struct AssetType
	{
		enum Type : unsigned int
		{
			None,
			Image,
			Mesh,
			Material,
			Scene
		};
	};


	class Asset
	{
		friend class AssetDatabase;

		void CreateMetaFile();

	protected:
		std::wstring name;
		std::filesystem::path pathToFile;

		unsigned long long ID = UINT64_MAX;

		AssetType::Type type;

		void SerializeIDAndType(json& serializer);

		void DeserializeIDAndType(const json& json);

		Asset()
		{
		};


	public:

		Asset(unsigned long long ID, std::filesystem::path pathToFile, AssetType::Type type = AssetType::None);


		virtual ~Asset();

		//TODO: В идеале тут должен принматься сериализатор, чтобы написать код один раз и забыть на всю жизнь, но не в этот раз
		virtual void Serialize(std::filesystem::path pathToFile) = 0;

		//TODO: В идеале тут должен принматься сериализатор, чтобы написать код один раз и забыть на всю жизнь, но не в этот раз
		virtual void Deserialize(std::filesystem::path pathToFile) = 0;


		static void WriteToFile(const std::filesystem::path& pathToFile, const json& j);

		static void ReadFromFile(const std::filesystem::path& pathToFile, json& j);

		static std::filesystem::path FindNativeFile(std::filesystem::path pathToFile);

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
	};
}
