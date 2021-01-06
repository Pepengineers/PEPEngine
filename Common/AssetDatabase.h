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

			inline static custom_unordered_map<std::wstring, std::shared_ptr<Asset>> actualPathToLoadedAssets = MemoryAllocator::CreateUnorderedMap<std::wstring, std::shared_ptr<Asset>>();

			inline static custom_unordered_map<std::wstring, std::shared_ptr<NativeModel>> loadedNativeModels =
				MemoryAllocator::CreateUnorderedMap<std::wstring, std::shared_ptr<NativeModel>>();

			inline static custom_unordered_map<std::wstring, std::shared_ptr<GTexture>> loadedTextures =
				MemoryAllocator::CreateUnorderedMap<std::wstring, std::shared_ptr<
					                                    GTexture>>();

			inline static custom_unordered_map<std::wstring, std::shared_ptr<GModel>> loadedModels =
				MemoryAllocator::CreateUnorderedMap<std::wstring, std::shared_ptr<GModel>>();

			inline static custom_unordered_map<UINT64, std::shared_ptr<Asset>> loadedAssets =
				MemoryAllocator::CreateUnorderedMap<UINT64, std::shared_ptr<Asset>>();

			inline static custom_unordered_map<AssetType::Type, std::vector<UINT64>> typedAssets =
				MemoryAllocator::CreateUnorderedMap<AssetType::Type, std::vector<UINT64>>();

			static std::filesystem::path GetOrCreateAssetFolderPath();

			static void Initialize();


			static void SaveToFile(std::shared_ptr<Asset> asset, const std::filesystem::path& saveAssetPath);
			static void LoadFromFile(std::shared_ptr<Asset> asset, const std::filesystem::path& saveAssetPath);

		public:


			static UINT64 GenerateID();

			inline static std::filesystem::path AssetFolderPath = GetOrCreateAssetFolderPath();

			inline const static std::wstring ASSET_EXTENSION_NAME = L".pepe";

			static std::shared_ptr<GTexture> LoadTextureFromFile(const std::filesystem::path& pathToFile);

			static std::shared_ptr<GModel> LoadModelFromFile(const std::filesystem::path& pathToFile);

			static void CreatePEPEFile(std::shared_ptr<Asset> asset, const std::filesystem::path& saveAssetPath);

			template <class T = Asset>
			static std::shared_ptr<T> LoadAssetFromFile(const std::filesystem::path& pathToFile)
			{
				if (pathToFile.wstring().find(AssetFolderPath) == std::string::npos)
				{
					return nullptr;
				}

				auto asset = std::make_shared<T>();

				CreatePEPEFile(asset, pathToFile);

				return asset;
			};

			static std::shared_ptr<Asset> FindAssetByID(UINT64 id);

			static std::shared_ptr<Asset> FindAssetByPath(std::filesystem::path& assetPath);

			
			static std::shared_ptr<Asset> Get(AssetType::Type type)
			{
				auto it = typedAssets.find(type);

				if (it != typedAssets.end())
				{
					if (it->second.size() > 0)
					{
						return (loadedAssets[it->second[0]]);
					}
					return nullptr;
				}
				return nullptr;
			}

			static std::vector<std::shared_ptr<Asset>> GetAll(AssetType::Type type)
			{
				auto it = typedAssets.find(type);

				if (it != typedAssets.end())
				{
					std::vector<std::shared_ptr<Asset>> assets;
					if (it->second.size() > 0)
					{
						for (auto&& id : it->second)
						{
							assets.push_back(loadedAssets[id]);
						}
					}
					return assets;
				}
			}


			template <class T = Asset>
			static std::shared_ptr<T> CreateAsset(std::filesystem::path& saveAssetPath)
			{
				if (saveAssetPath.wstring().find(AssetFolderPath) == std::string::npos)
				{
					saveAssetPath = AssetFolderPath.concat(L"\\").concat(saveAssetPath.filename().wstring());
				}

				if (saveAssetPath.wstring().find(ASSET_EXTENSION_NAME) == std::wstring::npos)
				{
					saveAssetPath = saveAssetPath.concat(ASSET_EXTENSION_NAME);
				}

				auto asset = std::make_shared<T>();

				CreatePEPEFile(asset, saveAssetPath);

				return asset;
			};


			static void UpdateAsset(std::shared_ptr<Asset> asset);

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
