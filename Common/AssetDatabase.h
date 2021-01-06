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
			friend class Asset;

		

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

		public:

			static UINT64 GenerateID();
			
			inline const static std::wstring ASSET_EXTENSION_NAME = L".pepe";
			
			static std::shared_ptr<GTexture> LoadTextureFromFile(const std::filesystem::path& pathToFile);

			static std::shared_ptr<GModel> LoadModelFromFile(const std::filesystem::path& pathToFile);

			static void DeserializeAssetData(std::shared_ptr<Asset> asset, const std::filesystem::path& pathToFile);
			
			template <class T = Asset>
			static std::shared_ptr<T> LoadAssetFromFile(const std::filesystem::path& pathToFile)
			{
				auto asset = std::make_shared<T>();

				DeserializeAssetData(asset, pathToFile);

				return asset;
			};

			static std::shared_ptr<Asset> FindAssetByID(UINT64 id);


			template <class T = Asset>
			static std::shared_ptr<T> CreateAsset(const std::filesystem::path& pathToFile)
			{
				auto id = GenerateID();
				auto asset = std::make_shared<T>(id, pathToFile);
				asset->CreateMetaInfoFile();
				
				loadedAssets[id] = asset;

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
