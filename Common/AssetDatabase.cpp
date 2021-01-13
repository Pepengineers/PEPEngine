#include "AssetDatabase.h"

#include <fstream>

#include "Asset.h"
#include "AssimpModelLoader.h"
#include "GCommandQueue.h"
#include "GDeviceFactory.h"
#include "GeometryGenerator.h"
#include "GMesh.h"
#include "GModel.h"
#include "GTexture.h"
#include "IDGenerator.hpp"
#include "AScene.h"
#include "NativeModel.h"
#include "ATexture.h"
#include "AMaterial.h"
#include "AModel.h"


namespace PEPEngine::Common
{
	using namespace Allocator;
	using namespace Utils;
	using namespace Graphics;

	void CheckOrCreateFolder(const std::filesystem::path& fileRelativePath)
	{
		if (exists(fileRelativePath))
			return;

		CheckOrCreateFolder(fileRelativePath.parent_path());

		create_directories(fileRelativePath);
	}


	static inline std::shared_ptr<GModel> CreateModelFromGenerated(std::shared_ptr<GCommandList> cmdList,
	                                                               GeometryGenerator::MeshData generatedData,
	                                                               std::string name)
	{
		auto nativeMesh = std::make_shared<NativeMesh>(generatedData.Vertices.data(), generatedData.Vertices.size(),
		                                               generatedData.Indices32.data(),
		                                               generatedData.Indices32.size(),
		                                               D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		auto nativeModel = std::make_shared<NativeModel>(name);
		nativeModel->AddMesh(std::move(nativeMesh));

		return std::make_shared<GModel>(nativeModel, cmdList);
	}


	std::shared_ptr<GModel> AssetDatabase::GenerateSphere(std::shared_ptr<GCommandList> cmdList, float radius,
	                                                      UINT sliceCount,
	                                                      UINT stackCount)
	{
		const GeometryGenerator::MeshData sphere = geoGen.CreateSphere(radius, sliceCount, stackCount);

		return CreateModelFromGenerated(cmdList, sphere, "sphere");
	}

	std::shared_ptr<GModel> AssetDatabase::GenerateQuad(std::shared_ptr<GCommandList> cmdList, float x, float y,
	                                                    float w,
	                                                    float h, float depth)
	{
		const GeometryGenerator::MeshData genMesh = geoGen.CreateQuad(x, y, w, h, depth);

		return CreateModelFromGenerated(cmdList, genMesh, "quad");
	}


	std::shared_ptr<ATexture> AssetDatabase::AddTexture(std::filesystem::path loadAssetFile,
	                                                    std::filesystem::path savePathInAssetFolder)
	{
		CheckExistFileAndCopyToAssetFolder(loadAssetFile, savePathInAssetFolder);

		auto pathToTextureInAssetFolder = std::filesystem::path(savePathInAssetFolder);
		
		auto texture = CreateAsset<ATexture>(savePathInAssetFolder);

		texture->texture = AssetDatabase::LoadTextureFromFile(pathToTextureInAssetFolder);
		texture->texture->SetName(pathToTextureInAssetFolder.filename());
		
		AddAssetInCache(texture);		
		UpdateAsset(texture.get());
		return texture;
	}

	void AssetDatabase::CheckExistFileAndCopyToAssetFolder(std::filesystem::path& loadAssetFile, std::filesystem::path& savePathInAssetFolder)
	{
		assert(exists(loadAssetFile));

		if (savePathInAssetFolder != L"")
		{
			if(savePathInAssetFolder.wstring().find(AssetFolderPath) == std::wstring::npos)
			{
				savePathInAssetFolder = std::filesystem::path(AssetFolderPath.wstring()).concat("\\").concat(savePathInAssetFolder.wstring());
			}			
			
			CheckOrCreateFolder(savePathInAssetFolder.parent_path());
		}
		else
		{
			savePathInAssetFolder =  std::filesystem::path(AssetFolderPath.wstring()).concat("\\").concat(loadAssetFile.filename().wstring());

			UINT count = 0;
			while (exists(savePathInAssetFolder))
			{
				savePathInAssetFolder = std::filesystem::path(AssetFolderPath.wstring()).concat("\\").concat(loadAssetFile.stem().wstring()).concat(" " + std::to_string(count++)).concat(loadAssetFile.extension().wstring());
			}
		}

		if(exists(savePathInAssetFolder))
		{
			std::filesystem::remove(savePathInAssetFolder);
		}
		
		copy_file(loadAssetFile, savePathInAssetFolder);
		
	}

	std::shared_ptr<Asset> AssetDatabase::FindAssetByType(AssetType::Type type)
	{
		auto it = typedAssets.find(type);
		if (it != typedAssets.end())
		{
			if (it->second.size() > 0)
			{
				return (loadedAssets[*it->second.begin()]);
			}
			return nullptr;
		}
		return nullptr;
	}

	std::shared_ptr<Asset> AssetDatabase::FindAssetByName(std::wstring name)
	{
		auto it = typedAssetsByName.find(name);

		if (it != typedAssetsByName.end())
		{
			return loadedAssets[it->second];
		}
		return nullptr;
	}

	std::shared_ptr<AModel> AssetDatabase::AddModel(std::filesystem::path loadAssetFile,	std::filesystem::path savePathInAssetFolder)
	{
		CheckExistFileAndCopyToAssetFolder(loadAssetFile, savePathInAssetFolder);

		auto model = AssetDatabase::LoadModelFromFile(loadAssetFile);

		auto aModel = CreateAsset<AModel>(savePathInAssetFolder);
		aModel->gModel = model;
		aModel->pathToFile = savePathInAssetFolder.parent_path().concat("\\").concat(savePathInAssetFolder.stem().wstring()).concat(ASSET_EXTENSION_NAME);

		AddAssetInCache(aModel);
		
		LoadMaterialsFromModelFile(loadAssetFile, aModel);

		UpdateAsset(aModel.get());
		
		return aModel;		
	}

	std::shared_ptr<AMaterial> AssetDatabase::AddMaterial(std::shared_ptr<Material> material,
		std::filesystem::path savePathInAssetFolder)
	{
		CheckOrCreateFolder(savePathInAssetFolder.parent_path());

		auto amaterial = CreateAsset<AMaterial>(savePathInAssetFolder);
		amaterial->pathToFile = savePathInAssetFolder.parent_path().concat("\\").concat(savePathInAssetFolder.stem().wstring()).concat(ASSET_EXTENSION_NAME);
		amaterial->material = material;

		AddAssetInCache(amaterial);		
		UpdateAsset(amaterial.get());
		
		return amaterial;		
	}

	void AssetDatabase::UpdateAsset(Asset* asset)
	{
		assert(asset->ID != UINT64_MAX);

		const auto it = loadedAssets.find(asset->ID);

		assert(it != loadedAssets.end());

		SaveToFile(it->second, asset->pathToFile);

		AddAssetInCache(it->second);
	}

	void AssetDatabase::RemoveAsset(const UINT64 id)
	{
		auto asset = loadedAssets[id];
		
		typedAssets[asset->type].erase(id);
		typedAssetsByName.erase(asset->GetName());
		actualPathToLoadedAssets.erase(asset->pathToFile);
		loadedAssets.erase(id);
		IDGenerator::FreeID(id);
		asset->Remove();		
	}

	void AssetDatabase::RemoveAsset(Asset* asset)
	{
		RemoveAsset(asset->ID);
	}

	std::shared_ptr<GTexture> AssetDatabase::LoadTextureFromFile(const std::filesystem::path& pathToFile)
	{
		auto it = loadedTextures.find(pathToFile);

		if (it != loadedTextures.end()) return it->second;

		auto device = GDeviceFactory::GetDevice();

		auto queue = device->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE);
		auto cmdList = queue->GetCommandList();

		auto texture = GTexture::LoadTextureFromFile(pathToFile, cmdList);

		queue->WaitForFenceValue(queue->ExecuteCommandList(cmdList));

		loadedTextures[pathToFile] = std::move(texture);

		return loadedTextures[pathToFile];
	}

	std::vector<std::shared_ptr<AMaterial>> AssetDatabase::LoadMaterialsFromModelFile(const std::filesystem::path& saveAssetPath, std::shared_ptr<AModel> model)
	{
		assert(exists(saveAssetPath));
		
		auto device = GDeviceFactory::GetDevice();
		auto queue = device->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE);
		auto cmdList = queue->GetCommandList();

		auto materials = AssimpModelLoader::FindAndCreateMaterialFromModelFile(cmdList, saveAssetPath.string(), model);
		
		queue->WaitForFenceValue(queue->ExecuteCommandList(cmdList));

		return materials;
	}
	

	std::shared_ptr<GModel> AssetDatabase::LoadModelFromFile(const std::filesystem::path& pathToFile)
	{
		auto it = loadedModels.find(pathToFile);
		if (it != loadedModels.end()) return it->second;

		auto device = GDeviceFactory::GetDevice();

		auto queue = device->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE);
		const auto cmdList = queue->GetCommandList();

		auto model = AssimpModelLoader::CreateModelFromFile(cmdList, pathToFile.string());
		
		queue->WaitForFenceValue(queue->ExecuteCommandList(cmdList));

		loadedModels[pathToFile] = (model);

		
		return loadedModels[pathToFile];
	}

	std::filesystem::path AssetDatabase::GetOrCreateAssetFolderPath()
	{
		auto path = std::filesystem::current_path().concat("\\Assets");

		CheckOrCreateFolder(path);

		if (!exists(path))
		{
			create_directory(path);
		}

		return path;
	}

	std::filesystem::path AssetDatabase::GetOrCreateDefaultAssetFolderPath()
	{
		return GetOrCreateAssetFolderPath().parent_path().concat("\\Default\\");
	}

	void AssetDatabase::Initialize()
	{
		CheckOrCreateFolder(AssetFolderPath);
		CheckOrCreateFolder(DefaultAssetPath);

		static std::unordered_map<AssetType::Type, std::vector<std::filesystem::path>> sortedLoadedAssets;

		for (auto& file : std::filesystem::recursive_directory_iterator(DefaultAssetPath))
		{
			if (file.is_regular_file())
			{
				if (file.path().filename().extension() == ASSET_EXTENSION_NAME)
				{
					auto path = file.path();

					json j;
					Asset::ReadFromFile(path, j);

					AssetType::Type type;
					(Asset::TryReadVariable< AssetType::Type>(j, "Type", &type));

					sortedLoadedAssets[type].push_back(path);
				}
			}
		}

		for (auto& loadAssets : sortedLoadedAssets[AssetType::Type::Image])
		{
			std::shared_ptr<ATexture> asset = std::make_shared<ATexture>();
			CreatePEPEFile(asset, loadAssets);

			if (loadAssets.filename().wstring() == L"defaultAlbedo.pepe")
			{
				ATexture::defaultAlbedo = asset;
			}
			else
			{
				ATexture::defaultNormal = asset;
			}
		}

		for (auto& loadAssets : sortedLoadedAssets[AssetType::Type::Material])
		{
			std::shared_ptr<AMaterial> asset = std::make_shared<AMaterial>();
			CreatePEPEFile(asset, loadAssets);

			AMaterial::defaultMaterial = asset;
		}

		sortedLoadedAssets.clear();


		for (auto&& file : std::filesystem::recursive_directory_iterator(AssetFolderPath))
		{
			if (file.is_regular_file())
			{
				if (file.path().filename().extension() == ASSET_EXTENSION_NAME)
				{
					auto path = file.path();

					json j;
					Asset::ReadFromFile(path, j);

					AssetType::Type type;
					(Asset::TryReadVariable< AssetType::Type>(j, "Type", &type));

					sortedLoadedAssets[type].push_back(path);
				}
			}
		}

		for (auto& loadAssets : sortedLoadedAssets[AssetType::Type::Image])
		{
			std::shared_ptr<ATexture> asset = std::make_shared<ATexture>();
			CreatePEPEFile(asset, loadAssets);
		}

		for (auto& loadAssets : sortedLoadedAssets[AssetType::Type::Material])
		{
			std::shared_ptr<AMaterial> asset = std::make_shared<AMaterial>();
			CreatePEPEFile(asset, loadAssets);
		}

		for (auto& loadAssets : sortedLoadedAssets[AssetType::Type::Model])
		{
			std::shared_ptr<AModel> asset = std::make_shared<AModel>();
			CreatePEPEFile(asset, loadAssets);
		}

		for (auto& loadAssets : sortedLoadedAssets[AssetType::Type::Level])
		{
			std::shared_ptr<AScene> asset = std::make_shared<AScene>();
			CreatePEPEFile(asset, loadAssets);
		}

		for (auto& loadAssets : sortedLoadedAssets[AssetType::Type::None])
		{
			std::shared_ptr<Asset> asset = std::make_shared<Asset>();
			CreatePEPEFile(asset, loadAssets);
		}
	}


	UINT64 AssetDatabase::GenerateID()
	{
		return IDGenerator::Generate();
	}

	std::shared_ptr<Asset> AssetDatabase::FindAssetByID(const UINT64 id)
	{
		const auto it = loadedAssets.find(id);

		if (it != loadedAssets.end()) return it->second;

		return nullptr;
	}

	std::shared_ptr<Asset> AssetDatabase::FindAssetByPath(std::filesystem::path& assetPath)
	{
		if (assetPath.wstring().find(AssetFolderPath) == std::string::npos)
		{
			assetPath = std::filesystem::path(AssetFolderPath.wstring()).concat(L"\\").concat(assetPath.filename().wstring());
		}

		if (assetPath.wstring().find(ASSET_EXTENSION_NAME) == std::wstring::npos)
		{
			assetPath = assetPath.concat(ASSET_EXTENSION_NAME);
		}

		auto it = actualPathToLoadedAssets.find(assetPath);
		if (it != actualPathToLoadedAssets.end()) return it->second;
		return nullptr;
	}


	void AssetDatabase::SaveToFile(std::shared_ptr<Asset> asset, const std::filesystem::path& saveAssetPath)
	{
		json json;
		asset->Serialize(json);
		Asset::WriteToFile(saveAssetPath, json);
	}

	void AssetDatabase::LoadFromFile(std::shared_ptr<Asset> asset, const std::filesystem::path& saveAssetPath)
	{
		json json;
		Asset::ReadFromFile(saveAssetPath, json);
		asset->Deserialize(json);
	}


	void AssetDatabase::AddAssetInCache(std::shared_ptr<Asset> asset)
	{
		actualPathToLoadedAssets[asset->pathToFile] = asset;
		loadedAssets[asset->ID] = asset;
		typedAssets[asset->type].insert(asset->ID);
		typedAssetsByName[asset->GetName()] = asset->ID;
	}

	void AssetDatabase::CreatePEPEFile(std::shared_ptr<Asset> asset, const std::filesystem::path& saveAssetPath)
	{
		CheckOrCreateFolder(saveAssetPath.parent_path());

		asset->pathToFile = saveAssetPath;


		if (exists(saveAssetPath))
		{
			LoadFromFile(asset, saveAssetPath);

			IDGenerator::AddLoadedID(asset->ID);
		}
		else
		{
			asset->ID = GenerateID();

			SaveToFile(asset, saveAssetPath);
		}

		AddAssetInCache(asset);
	}
}
