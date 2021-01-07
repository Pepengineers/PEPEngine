#include "Asset.h"
#include "AssetDatabase.h"

#include <cassert>
#include <fstream>

namespace PEPEngine::Common
{
	
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

	void Asset::Remove()
	{
		std::filesystem::remove(pathToFile);
	}

	std::wstring Asset::GetName() const
	{
		return pathToFile.stem().stem();
	}

	std::filesystem::path Asset::GetPepeFilePath() const
	{
		return pathToFile;
	}

	Asset::Asset(AssetType::Type type): type(type)
	{
	}

	void Asset::Serialize(json& json)
	{
	}

	void Asset::Deserialize(json& json)
	{
	}

	uint64_t Asset::GetID() const
	{
		return ID;
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
	
	bool replace(std::wstring& str, const std::wstring& from, const std::wstring& to) {
		size_t start_pos = str.find(from);
		if (start_pos == std::wstring::npos)
			return false;
		str.replace(start_pos, from.length(), to);
		return true;
	}

	

	std::filesystem::path Asset::FindNativeFile(std::filesystem::path pathToFile)
	{
		auto path = pathToFile.wstring();
		replace(path, AssetDatabase::ASSET_EXTENSION_NAME, L"");
		return path;
	}

	
	std::filesystem::path Asset::GetFilePath(const Asset& asset, std::wstring extension)
	{
		auto file = asset.pathToFile.filename().wstring();

		replace(file, asset.pathToFile.extension().wstring(), extension);

		return std::filesystem::path(asset.pathToFile.parent_path().concat("\\").concat(file));
	}

	void Asset::UpdatePepe()
	{
		AssetDatabase::UpdateAsset(shared_from_this());
	}

}
