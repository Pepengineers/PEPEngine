#include "AssimpModelLoader.h"



#include "AMaterial.h"
#include "AssetDatabase.h"
#include "ATexture.h"
#include "GeometryGenerator.h"
#include "GMesh.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"

#include "GModel.h"
#include "GTexture.h"
#include "Material.h"
#include "NativeModel.h"
#include "ShaderBuffersData.h"

namespace PEPEngine::Common
{
	using namespace Graphics;


	std::vector<Vertex> AssimpModelLoader::GetVertices(aiMesh* mesh)
	{
		std::vector<Vertex> vertices;

		//Get vertices
		for (UINT i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex{};

			vertex.Position.x = mesh->mVertices[i].x;
			vertex.Position.y = mesh->mVertices[i].y;
			vertex.Position.z = mesh->mVertices[i].z;

			vertex.Normal.x = mesh->mNormals[i].x;
			vertex.Normal.y = mesh->mNormals[i].y;
			vertex.Normal.z = mesh->mNormals[i].z;

			if (mesh->HasTextureCoords(0))
			{
				vertex.TexCord.x = mesh->mTextureCoords[0][i].x;
				vertex.TexCord.y = mesh->mTextureCoords[0][i].y;
			}

			if (mesh->HasTangentsAndBitangents())
			{
				vertex.TangentU.x = mesh->mTangents[i].x;
				vertex.TangentU.y = mesh->mTangents[i].y;
				vertex.TangentU.z = mesh->mTangents[i].z;
			}

			vertices.push_back(vertex);
		}
		return vertices;
	}

	std::vector<unsigned long> AssimpModelLoader::GetIndices(aiMesh* mesh)
	{
		std::vector<DWORD> data;

		for (UINT i = 0; i < mesh->mNumFaces; i++)
		{
			const aiFace face = mesh->mFaces[i];

			for (UINT j = 0; j < face.mNumIndices; j++)
			{
				data.push_back(face.mIndices[j]);
			}
		}
		return data;
	}	
		
	std::shared_ptr<NativeMesh> AssimpModelLoader::CreateSubMesh(aiMesh* mesh,
	                                                             const std::string& modelName)
	{
		auto vertices = GetVertices(mesh);
		auto indices = GetIndices(mesh);

		RecalculateTangent(indices.data(), indices.size(), vertices.data());

		auto nativeMesh = std::make_shared<NativeMesh>(vertices.data(), vertices.size(), indices.data(),
		                                               indices.size(),
		                                               D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		                                               modelName + " " + (mesh->mName.C_Str()));
		vertices.clear();
		indices.clear();

		return nativeMesh;
	}

	void AssimpModelLoader::RecursivlyLoadMeshes(std::shared_ptr<NativeModel> model, aiNode* node,
	                                             const aiScene* scene)
	{
		for (UINT i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* aMesh = scene->mMeshes[node->mMeshes[i]];
			const auto nativeMesh = CreateSubMesh(aMesh, model->GetName());
			model->AddMesh(std::move((nativeMesh)));
		}

		for (UINT i = 0; i < node->mNumChildren; i++)
		{
			RecursivlyLoadMeshes(model, node->mChildren[i], scene);
		}
	}

	std::shared_ptr<GModel> AssimpModelLoader::CreateModelFromFile(
		const std::shared_ptr<GCommandList>& cmdList, const std::filesystem::path& filePath)
	{
		Assimp::Importer importer;
		
		const aiScene* sceneModel = importer.ReadFile(filePath.string(),
		                                              aiProcess_Triangulate | aiProcess_GenNormals | 
		                                              aiProcess_ConvertToLeftHanded);

		assert(sceneModel != nullptr && "Model Path dosen't exist or wrong file");

		auto modelFromFile = std::make_shared<NativeModel>(filePath.stem().string());

		RecursivlyLoadMeshes(modelFromFile, sceneModel->mRootNode, sceneModel);

		auto renderModel = std::make_shared<GModel>(modelFromFile, cmdList);

		for (int i = 0; i < renderModel->GetMeshesCount(); ++i)
		{
			renderModel->SetMaterial(AMaterial::GetDefaultMaterial(), i);
		}		
		
		return renderModel;
	}

	void  RecursivlyFindMaterialPerMesh(aiNode* node, const aiScene* scene, std::unordered_map<aiMaterial*, std::vector<aiMesh*>>& materialPerMesh)
	{
		for (UINT i = 0; i < node->mNumMeshes; i++)
		{
			auto mesh = scene->mMeshes[node->mMeshes[i]];
			auto aiMaterial = scene->mMaterials[mesh->mMaterialIndex];			
			
			materialPerMesh[aiMaterial].push_back(mesh);
		}

		for (UINT i = 0; i < node->mNumChildren; i++)
		{
			RecursivlyFindMaterialPerMesh(node->mChildren[i], scene, materialPerMesh);
		}
	}

	bool HasTextures(aiMaterial* material, aiTextureType type)
	{
		auto textureCount = material->GetTextureCount(aiTextureType_DIFFUSE);
		return textureCount > 0;
	}

	std::shared_ptr<ATexture> AssimpModelLoader::LoadTextureFromAiMaterial(const aiMaterial* material, const aiTextureType type,
		const std::filesystem::path& directory, std::shared_ptr<AModel> model)
	{
		aiString str;
		material->GetTexture(type, 0, &str);

		auto dirPath = directory;
		
		std::wstring modelTexturePath(AnsiToWString(str.C_Str()));

		if (modelTexturePath.find(L"\\") != std::wstring::npos)
		{
			auto fileName = modelTexturePath.substr(modelTexturePath.find_last_of('\\'),
				modelTexturePath.size() - modelTexturePath.find_last_of('\\'));

			modelTexturePath = fileName.replace(fileName.find(L"\\"), 1, L"");
		}

		if(modelTexturePath.empty())
		{
			return nullptr;
		}
		

		std::filesystem::path texturePath;
		for (auto && iterator : std::filesystem::directory_iterator(directory))
		{
			if(iterator.is_regular_file())
			{
				if(iterator.path().wstring().find(modelTexturePath) != std::wstring::npos)
				{
					texturePath = iterator.path();
					break;
				}
			}
		}

		if(!texturePath.has_filename())
		{
			for (auto&& iterator : std::filesystem::recursive_directory_iterator(directory))
			{
				if (iterator.is_regular_file())
				{
					if (iterator.path().wstring().find(modelTexturePath) != std::wstring::npos)
					{
						texturePath = iterator.path();
						break;
					}
				}
			}
		}

		if (!texturePath.has_filename()) return nullptr;

		OutputDebugStringW((texturePath.wstring() + L"\n").c_str());

		auto texture = AssetDatabase::AddTexture(texturePath, model->GetPepeFilePath().parent_path().concat("\\").concat(texturePath.filename().wstring()));
		return texture;
	}
	
	std::vector<std::shared_ptr<AMaterial>> AssimpModelLoader::FindAndCreateMaterialFromModelFile(
		const std::shared_ptr<GCommandList>& cmdList, const std::string& filePath, std::shared_ptr<AModel> model)
	{
		Assimp::Importer importer;

		const aiScene* sceneModel = importer.ReadFile(filePath, 0);

		assert(sceneModel != nullptr && "Model Path dosen't exist or wrong file");

		std::vector<std::shared_ptr<AMaterial>> materials;

		std::unordered_map<aiMaterial*, std::vector<aiMesh*>> findedMaterialsPerMesh;

		RecursivlyFindMaterialPerMesh(sceneModel->mRootNode, sceneModel, findedMaterialsPerMesh);

		std::unordered_map<aiMaterial*, std::shared_ptr<AMaterial>> createdMaterials;


		auto gmodel = model->GetGModel();
		
		for (auto && pair : findedMaterialsPerMesh)
		{
			auto aiMaterial = pair.first;

			aiString name;
			aiMaterial->Get(AI_MATKEY_NAME, name);

			auto gmaterial = std::make_shared<Material>(gmodel->GetName() + " " + (name.C_Str()));

			auto modelDir = std::filesystem::path(filePath).parent_path();

			std::shared_ptr<ATexture> texture = nullptr;
			
			if(HasTextures(aiMaterial, aiTextureType_DIFFUSE))
			{
				texture = LoadTextureFromAiMaterial(aiMaterial, aiTextureType_DIFFUSE, modelDir, model);
				
			}
			else
			{
				if(HasTextures(aiMaterial, aiTextureType_BASE_COLOR))
				{
					texture = LoadTextureFromAiMaterial(aiMaterial, aiTextureType_BASE_COLOR, modelDir, model);
				}				
			}

			if(texture == nullptr)
			{
				texture = ATexture::GetDefaultAlbedo();
			}
			
			gmaterial->SetMaterialMap(Material::BaseColor, texture);


			texture = nullptr;

			if(HasTextures(aiMaterial, aiTextureType_HEIGHT))
			{
				texture = LoadTextureFromAiMaterial(aiMaterial, aiTextureType_HEIGHT, modelDir, model);
			}
			else
			{
				if(HasTextures(aiMaterial, aiTextureType_NORMALS))
				{
					texture = LoadTextureFromAiMaterial(aiMaterial, aiTextureType_NORMALS, modelDir, model);
				}
			}

			if (texture == nullptr)
			{
				texture = ATexture::GetDefaultNormal();
			}
			
			gmaterial->SetMaterialMap(Material::NormalMap, texture);


			auto aMaterial = AssetDatabase::AddMaterial(gmaterial, model->GetPepeFilePath().parent_path().concat("\\").concat(gmaterial->GetName()).concat(AMaterial::DEFAULT_EXTENSION));
			

			materials.push_back(aMaterial);			
			createdMaterials[aiMaterial] = aMaterial;

		}		

		auto gMeshes = gmodel->GetMeshes();
		
		for (auto && pair : findedMaterialsPerMesh)
		{
			for (auto&& mesh : pair.second)
			{
				for (int i = 0; i < gmodel->GetMeshesCount(); ++i)
				{
					const auto gmesh = gMeshes[i];
					if(gmesh->GetName().find((mesh->mName.C_Str())) != std::string::npos)
					{
						gmodel->SetMaterial(createdMaterials[pair.first], i);
					}
				}				
			}
		}
		
		

		return materials;
	}
}
