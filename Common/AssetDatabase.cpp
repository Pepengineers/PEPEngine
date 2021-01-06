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
#include "NativeModel.h"


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
					LoadAssetFromFile(file.path());
				}
			}
		}		
	}

	UINT64 AssetDatabase::GenerateID()
	{
		return IDGenerator::Generate();
	}

	void AssetDatabase::DeserializeAssetData(std::shared_ptr<Asset> asset, const std::filesystem::path& pathToFile)
	{
		json j;
		Asset::ReadFromFile(pathToFile, j);
		
		asset->Deserialize(j);
	}

	std::shared_ptr<Asset> AssetDatabase::FindAssetByID(const UINT64 id)
	{
		const auto it = loadedAssets.find(id);

		if (it != loadedAssets.end()) return it->second;

		return nullptr;
	}

	
	
	void AssetDatabase::CreatePEPEFile(std::shared_ptr<Asset> asset, const std::filesystem::path& saveAssetPath)
	{
		CheckOrCreateFolder(saveAssetPath);

		json json;
		if(std::filesystem::exists(saveAssetPath))
		{
			Asset::ReadFromFile(saveAssetPath, json);		
			asset->Deserialize(json);
			
			asset->pathToFile = saveAssetPath;
			IDGenerator::AddLoadedID(asset->ID);			
		}
		else
		{
			asset->ID = GenerateID();
			asset->pathToFile = saveAssetPath;
			
			asset->Serialize(json);
			Asset::WriteToFile( saveAssetPath, json);
		}

		loadedAssets[asset->ID] = asset;
	}
}
