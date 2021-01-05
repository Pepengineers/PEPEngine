#pragma once
#include "GModel.h"
#include "GTexture.h"
#include "NativeModel.h"
#include "ShaderBuffersData.h"
#include "assimp/scene.h"
#include "assimp/mesh.h"

namespace PEPEngine::Common
{
	class AssimpModelLoader
	{
		static std::vector<Vertex> GetVertices(aiMesh* mesh);

		static std::vector<unsigned long> GetIndices(aiMesh* mesh);


		static std::shared_ptr<GTexture> LoadTextureByAiMaterial(const aiMaterial* material,
		                                                         aiTextureType type,
		                                                         const std::wstring& directory,
		                                                         const std::shared_ptr<GCommandList>
		                                                         & cmdList);

		static std::shared_ptr<GTexture> LoadTextureByPath(const std::wstring& name,
		                                                   const std::wstring& fullPath,
		                                                   const std::shared_ptr<GCommandList>& cmdList);

		static void LoadTextureForModel(std::shared_ptr<GModel> model, const std::shared_ptr<GCommandList>& cmdList);


		static std::shared_ptr<NativeMesh> CreateSubMesh(aiMesh* mesh, const std::wstring& modelName);

		static void RecursivlyLoadMeshes(std::shared_ptr<NativeModel> model, aiNode* node, const aiScene* scene);

	public:
		static std::shared_ptr<GModel> CreateModelFromFile(const std::shared_ptr<GCommandList>& cmdList,
		                                                   const std::string& filePath);
	};
}
