#pragma once
#include "Asset.h"
#include "GeometryGenerator.h"
#include "MemoryAllocator.h"
#include "filesystem"
#include "GModel.h"
#include "GTexture.h"
#include "NativeModel.h"

namespace PEPEngine
{
	namespace Common
	{
		using namespace Allocator;


		class AssetDatabase
		{
			friend class AssimpModelLoader;	
			friend class D3DApp;
			
			inline static GeometryGenerator geoGen;

			inline static custom_unordered_map<std::wstring, std::shared_ptr<NativeModel>> loadedNativeModels =
				MemoryAllocator::CreateUnorderedMap<std::wstring, std::shared_ptr<NativeModel>>();

			inline static custom_unordered_map<std::wstring, std::shared_ptr<GTexture>> loadedTextures =
				MemoryAllocator::CreateUnorderedMap<std::wstring, std::shared_ptr<
					                                    GTexture>>();

			inline static custom_unordered_map<std::wstring, std::shared_ptr<GModel>> loadedModels =
				MemoryAllocator::CreateUnorderedMap<std::wstring, std::shared_ptr<GModel>>();

			inline static custom_unordered_map<UINT64, std::shared_ptr<Asset>> loadedAssets =
				MemoryAllocator::CreateUnorderedMap<UINT64, std::shared_ptr<Asset>>();

			static std::filesystem::path GetOrCreateAssetFolderPath();

			static void Initialize();
			
		public:

			
			
			static UINT64 GenerateID();
			
			inline static std::filesystem::path AssetFolderPath = GetOrCreateAssetFolderPath();
			
			inline const static std::wstring ASSET_EXTENSION_NAME = L".pepe";
			
			static std::shared_ptr<GTexture> LoadTextureFromFile(const std::filesystem::path& pathToFile);

			static std::shared_ptr<GModel> LoadModelFromFile(const std::filesystem::path& pathToFile);

			static void DeserializeAssetData(std::shared_ptr<Asset> asset, const std::filesystem::path& pathToFile);

			static void CreatePEPEFile(std::shared_ptr<Asset> asset, const std::filesystem::path& saveAssetPath);
			
			template <class T = Asset>
			static std::shared_ptr<T> LoadAssetFromFile(const std::filesystem::path& pathToFile)
			{
				if (pathToFile.wstring().find(AssetFolderPath) == std::string::npos)
				{
					return  nullptr;
				}
				
				auto asset = std::make_shared<T>();

				CreatePEPEFile(asset, pathToFile);
				
				return asset;
			};

			static std::shared_ptr<Asset> FindAssetByID(UINT64 id);


			template <class T = Asset>
			static std::shared_ptr<T> CreateAsset(std::filesystem::path& saveAssetPath)
			{
				if(saveAssetPath.wstring().find(AssetFolderPath) == std::string::npos)
				{
					saveAssetPath = AssetFolderPath.concat(L"\\").concat(saveAssetPath.filename().wstring());
				}
				
				if(saveAssetPath.wstring().find(ASSET_EXTENSION_NAME) == std::wstring::npos)
				{
					saveAssetPath = saveAssetPath.concat(ASSET_EXTENSION_NAME);
				}
				
				auto asset = std::make_shared<T>();

				CreatePEPEFile(asset, saveAssetPath);				

				return asset;
			};

			static void RemoveAsset(UINT64 id);
			static void RemoveAsset(Asset* asset);


			static std::shared_ptr<GModel> GenerateSphere(std::shared_ptr<GCommandList> cmdList, float radius = 1.0f,
			                                              UINT sliceCount = 20, UINT stackCount = 20);

			static std::shared_ptr<GModel> GenerateQuad(std::shared_ptr<GCommandList> cmdList, float x = 1.0f,
			                                            float y = 1.0f,
			                                            float w = 1.0f, float h = 1.0f, float depth = 0.0);
		};
	}
}
