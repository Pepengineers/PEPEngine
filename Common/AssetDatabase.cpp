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
#include "Level.h"
#include "NativeModel.h"
#include "Texture.h"


namespace PEPEngine::Common
{
	using namespace Allocator;
	using namespace Utils;
	using namespace Graphics;
	
	void CheckOrCreateFolder(const std::filesystem::path fileRelativePath)
	{
		if (exists(fileRelativePath.parent_path()))
			return;

		CheckOrCreateFolder(fileRelativePath.parent_path());
		
		create_directories(fileRelativePath.parent_path());
	}

	
	static inline std::shared_ptr<GModel> CreateModelFromGenerated(std::shared_ptr<GCommandList> cmdList,
	                                                               GeometryGenerator::MeshData generatedData,
	                                                               std::wstring name)
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

		return CreateModelFromGenerated(cmdList, sphere, L"sphere");
	}

	std::shared_ptr<GModel> AssetDatabase::GenerateQuad(std::shared_ptr<GCommandList> cmdList, float x, float y,
	                                                    float w,
	                                                    float h, float depth)
	{
		const GeometryGenerator::MeshData genMesh = geoGen.CreateQuad(x, y, w, h, depth);

		return CreateModelFromGenerated(cmdList, genMesh, L"quad");
	}


	void AssetDatabase::UpdateAsset(std::shared_ptr<Asset> asset)
	{
		assert(asset->ID != UINT64_MAX);

		const auto it = loadedAssets.find(asset->ID);

		assert(it != loadedAssets.end());

		SaveToFile(asset, asset->pathToFile);

		//actualPathToLoadedAssets[asset->pathToFile] = asset;
	}

	void AssetDatabase::RemoveAsset(const UINT64 id)
	{
		loadedAssets.erase(id);
		IDGenerator::FreeID(id);
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

		auto queue = device->GetCommandQueue();
		auto cmdList = queue->GetCommandList();

		auto texture = GTexture::LoadTextureFromFile(pathToFile, cmdList);

		queue->WaitForFenceValue(queue->ExecuteCommandList(cmdList));

		loadedTextures[pathToFile] = std::move(texture);

		return loadedTextures[pathToFile];
	}


	std::shared_ptr<GModel> AssetDatabase::LoadModelFromFile(const std::filesystem::path& pathToFile)
	{
		auto it = loadedModels.find(pathToFile);
		if (it != loadedModels.end()) return it->second;

		auto device = GDeviceFactory::GetDevice();

		auto queue = device->GetCommandQueue();
		const auto cmdList = queue->GetCommandList();

		auto model = AssimpModelLoader::CreateModelFromFile(cmdList, pathToFile.string());

		queue->WaitForFenceValue(queue->ExecuteCommandList(cmdList));

		loadedModels[pathToFile] = std::move(model);

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

	void AssetDatabase::Initialize()
	{
		CheckOrCreateFolder(AssetFolderPath);

		for (auto&& file : std::filesystem::recursive_directory_iterator(AssetFolderPath))
		{
			if(file.is_regular_file())
			{
				if(file.path().filename().extension() == ASSET_EXTENSION_NAME)
				{
					auto path = file.path();

					json j;
					Asset::ReadFromFile(path, j);

					AssetType::Type type;
					assert(Asset::TryReadVariable< AssetType::Type>(j, "Type", &type));

					std::shared_ptr<Asset> asset;
					
					switch (type)
					{
					case AssetType::Image: asset = std::make_shared<Texture>(); break;
					case AssetType::Mesh: break;
					case AssetType::Material: break;
					case AssetType::Level: asset = std::make_shared<Level>(); break;
					default: asset = std::make_shared<Asset>(); ;
					}					

					CreatePEPEFile(asset, path);
				}
			}
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
			assetPath = AssetFolderPath.concat(L"\\").concat(assetPath.filename().wstring());
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
		Asset::WriteToFile( saveAssetPath, json);
	}

	void AssetDatabase::LoadFromFile(std::shared_ptr<Asset> asset, const std::filesystem::path& saveAssetPath)
	{
		json json;
		Asset::ReadFromFile(saveAssetPath, json);		
		asset->Deserialize(json);
	}

	void AssetDatabase::CreatePEPEFile(std::shared_ptr<Asset> asset, const std::filesystem::path& saveAssetPath)
	{
		CheckOrCreateFolder(saveAssetPath);
		
		asset->pathToFile = saveAssetPath;
		
		
		if(std::filesystem::exists(saveAssetPath))
		{
			LoadFromFile(asset, saveAssetPath);
			
			IDGenerator::AddLoadedID(asset->ID);			
		}
		else
		{
			asset->ID = GenerateID();

			SaveToFile(asset, saveAssetPath);
		}

		actualPathToLoadedAssets[saveAssetPath] = asset;
		loadedAssets[asset->ID] = asset;
		
		typedAssets[asset->type].push_back(asset->ID);
		
	}
}
