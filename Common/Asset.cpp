#include "Asset.h"


#include <cassert>
#include <fstream>

namespace PEPEngine::Common
{
	void Asset::CreateMetaInfoFile()
	{
		json j;		
		Serialize(j);
		WriteToFile(pathToFile, j);
	}

	void Asset::SerializeIDAndType(json& serializer)
	{
		serializer["Version"] = SCHEMA_VERSION;
		serializer["GUID"] = ID;
		serializer["Type"] = type;
	}

	void Asset::DeserializeIDAndType(const json& json)
	{
		const float version = json["Version"];

		assert(version == SCHEMA_VERSION);
		assert(TryReadVariable<unsigned long long>(json, "GUID", &ID));
		assert(TryReadVariable<AssetType::Type>(json, "Type", &type));
	}

	Asset::Asset(unsigned long long ID, std::filesystem::path pathToFile,
	             AssetType::Type type) : pathToFile(pathToFile), ID(ID), type(type)
	{
	}

	Asset::~Asset() = default;

	void Asset::WriteToFile(const std::filesystem::path& pathToFile, const json& j)
	{
		std::ofstream file(pathToFile);
		file << std::setw(4) << j << std::endl;
	}

	void Asset::ReadFromFile(const std::filesystem::path& pathToFile, json& j)
	{
		std::ifstream file(pathToFile);
		file >> j;
	}

	std::filesystem::path Asset::FindNativeFile(std::filesystem::path pathToFile)
	{
		const auto folder = pathToFile.parent_path();

		const auto fileName = pathToFile.filename().wstring();

		const auto fileNameWithoutExtension = fileName.substr(0, fileName.find(pathToFile.extension()));

		std::filesystem::directory_iterator it(folder);

		std::filesystem::path nativeDataFile;

		for (auto&& file : it)
		{
			auto folderFileName = file.path().filename().wstring();
			if (folderFileName.find(fileNameWithoutExtension) != std::wstring::npos && file.path().extension() !=
				pathToFile.extension())
			{
				nativeDataFile = file.path();
				break;
			}
		}

		return nativeDataFile;
	}
}
