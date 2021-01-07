#pragma once
#include "AModel.h"
#include "AMaterial.h"
#include "AScene.h"
#include "Asset.h"
#include "ATexture.h"
#include "GeometryGenerator.h"
#include "MemoryAllocator.h"
#include "filesystem"
#include "GModel.h"
#include "GTexture.h"
#include "Material.h"
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

			inline static custom_unordered_map<std::wstring, std::shared_ptr<Asset>> actualPathToLoadedAssets =
				MemoryAllocator::CreateUnorderedMap<std::wstring, std::shared_ptr<Asset>>();

			inline static custom_unordered_map<std::wstring, std::shared_ptr<NativeModel>> loadedNativeModels =
				MemoryAllocator::CreateUnorderedMap<std::wstring, std::shared_ptr<NativeModel>>();

			inline static custom_unordered_map<std::wstring, std::shared_ptr<GTexture>> loadedTextures =
				MemoryAllocator::CreateUnorderedMap<std::wstring, std::shared_ptr<GTexture>>();

			inline static custom_unordered_map<std::wstring, std::shared_ptr<GModel>> loadedModels =
				MemoryAllocator::CreateUnorderedMap<std::wstring, std::shared_ptr<GModel>>();

			inline static custom_unordered_map<std::wstring, std::shared_ptr<AMaterial>> loadedModelMaterials =
				MemoryAllocator::CreateUnorderedMap<std::wstring, std::shared_ptr<AMaterial>>();

			inline static custom_unordered_map<UINT64, std::shared_ptr<Asset>> loadedAssets =
				MemoryAllocator::CreateUnorderedMap<UINT64, std::shared_ptr<Asset>>();

			inline static custom_unordered_map<AssetType::Type, std::set<UINT64>> typedAssets =
				MemoryAllocator::CreateUnorderedMap<AssetType::Type, std::set<UINT64>>();

			inline static custom_unordered_map<std::wstring, UINT64> typedAssetsByName =
				MemoryAllocator::CreateUnorderedMap<std::wstring, UINT64>();

			static std::filesystem::path GetOrCreateAssetFolderPath();
			static std::filesystem::path GetOrCreateDefaultAssetFolderPath();;

			static void Initialize();


			static void SaveToFile(std::shared_ptr<Asset> asset, const std::filesystem::path& saveAssetPath);
			static void LoadFromFile(std::shared_ptr<Asset> asset, const std::filesystem::path& saveAssetPath);
			static void AddAssetInCache(std::shared_ptr<Asset> asset);

			static std::vector<std::shared_ptr<AMaterial>> LoadMaterialsFromModelFile(
				const std::filesystem::path& saveAssetPath, std::shared_ptr<AModel> model);
			static void CheckExistFileAndCopyToAssetFolder(std::filesystem::path& loadAssetFile,
			                                               std::filesystem::path& savePathInAssetFolder);

			static std::shared_ptr<Asset> FindAssetByType(AssetType::Type type);

			static std::shared_ptr<Asset> FindAssetByName(std::wstring name);
		public:


			static UINT64 GenerateID();

			inline static const std::filesystem::path& AssetFolderPath = GetOrCreateAssetFolderPath();
			inline static const std::filesystem::path& DefaultAssetPath = GetOrCreateDefaultAssetFolderPath();

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

			template <class T = Asset>
			static std::shared_ptr<T> FindAssetByID(UINT64 id)
			{
				return std::static_pointer_cast<T>(FindAssetByID(id));
			}


			static std::shared_ptr<Asset> FindAssetByPath(std::filesystem::path& assetPath);

			template <class T = Asset>
			static std::shared_ptr<T> FindAssetByPath(std::filesystem::path& assetPath)
			{
				return std::static_pointer_cast<T>(FindAssetByPath(assetPath));
			}

			
			
			template <class T = Asset>
			static std::shared_ptr<T> Get(AssetType::Type type)
			{
				return std::static_pointer_cast<Asset, T>(FindAssetByType(type));
			}

		
			
			template <class T = Asset>
			static std::shared_ptr<T> Get(std::wstring name)
			{
				auto asset = FindAssetByName(name);
				return std::static_pointer_cast<T>(asset);
			}

			template <class T = Asset>
			static std::vector<std::shared_ptr<T>> GetAll(AssetType::Type type)
			{
				auto it = typedAssets.find(type);

				std::vector<std::shared_ptr<Asset>> assets;
				if (it != typedAssets.end())
				{
					if (it->second.size() > 0)
					{
						for (auto&& id : it->second)
						{
							assets.push_back(loadedAssets[id]);
						}
					}
					return assets;
				}
				return assets;
			}


			template <class T = Asset>
			static std::shared_ptr<T> CreateAsset(std::filesystem::path& saveAssetPath)
			{
				if (saveAssetPath.wstring().find(AssetFolderPath) == std::string::npos)
				{
					saveAssetPath = std::filesystem::path(AssetFolderPath.wstring()).concat(L"\\").concat(saveAssetPath.filename().wstring());
				}

				if (saveAssetPath.wstring().find(ASSET_EXTENSION_NAME) == std::wstring::npos)
				{
					saveAssetPath = saveAssetPath.concat(ASSET_EXTENSION_NAME);
				}

				auto asset = std::make_shared<T>();

				CreatePEPEFile(asset, saveAssetPath);

				return asset;
			};

			template <class T = Asset>
			static std::shared_ptr<T> CreateAsset(std::wstring createAssetName)
			{
				if (createAssetName.find(ASSET_EXTENSION_NAME) == std::wstring::npos)
				{
					createAssetName = createAssetName + T::DEFAULT_EXTENSION + (ASSET_EXTENSION_NAME);
				}

				auto path = std::filesystem::path(AssetFolderPath.wstring() + L"\\" + createAssetName);

				auto asset = std::make_shared<T>();

				CreatePEPEFile(asset, path);

				return asset;
			}
			

			static std::shared_ptr<ATexture> AddTexture(std::filesystem::path loadAssetFile,
			                                            std::filesystem::path savePathInAssetFolder = L"");

			static std::shared_ptr<AModel> AddModel(std::filesystem::path loadAssetFile,
			                                        std::filesystem::path savePathInAssetFolder = L"");

			static std::shared_ptr<AMaterial> AddMaterial(std::shared_ptr<Material> material,
			                                              std::filesystem::path savePathInAssetFolder = L"");

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
