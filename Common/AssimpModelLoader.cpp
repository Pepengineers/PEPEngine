#include "AssimpModelLoader.h"


#include "AssetDatabase.h"
#include "GeometryGenerator.h"
#include "GMesh.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"

#include "GModel.h"
#include "GTexture.h"
#include "NativeModel.h"
#include "ShaderBuffersData.h"

namespace PEPEngine::Common
{
	using namespace Graphics;
	static Assimp::Importer importer;


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

	std::shared_ptr<GTexture> AssimpModelLoader::LoadTextureByAiMaterial(
		const aiMaterial* material, const aiTextureType type, const std::wstring
		& directory,
		const std::shared_ptr<GCommandList>& cmdList)
	{
		aiString str;
		material->GetTexture(type, 0, &str);

		std::wstring modelTexturePath(AnsiToWString(str.C_Str()));

		if (modelTexturePath.find(L"\\") != std::wstring::npos)
		{
			auto fileName = modelTexturePath.substr(modelTexturePath.find_last_of('\\'),
			                                        modelTexturePath.size() - modelTexturePath.find_last_of('\\'));

			modelTexturePath = fileName.replace(fileName.find(L"\\"), 1, L"");
		}

		std::wstring textureName = modelTexturePath;
		std::wstring texturePath = directory + L"\\" + textureName;

		return LoadTextureByPath(textureName, texturePath, cmdList);
	}

	std::shared_ptr<GTexture> AssimpModelLoader::LoadTextureByPath(
		const std::wstring& name, const std::wstring& fullPath, const std::shared_ptr<GCommandList>& cmdList)
	{
		OutputDebugStringW((name + L"\n").c_str());

		auto texture = GTexture::LoadTextureFromFile(fullPath, cmdList);
		texture->SetName(name);

		return texture;
	}

	void AssimpModelLoader::LoadTextureForModel(std::shared_ptr<GModel> model, const std::shared_ptr<GCommandList>&
	                                            cmdList)
	{
		/*for (int i = 0; i < model->GetMeshesCount(); ++i)
		{
			auto nativeMesh = model->GetMesh(i)->GetMeshData();

			auto aiMaterial = loadedAiMaterialForMesh[nativeMesh];

			assert(aiMaterial != nullptr);

			aiString name;
			aiMaterial->Get(AI_MATKEY_NAME, name);

			auto materialName = model->GetName() + L" " + AnsiToWString(name.C_Str());

			auto it = materialsMap.find(materialName);

			std::shared_ptr<Material> material;

			if (it == materialsMap.end())
			{
				material = std::make_shared<Material>(materialName);

				const auto modelDirectory = model->GetName().substr(0, model->GetName().find_last_of('\\'));

				auto textureCount = aiMaterial->GetTextureCount(aiTextureType_DIFFUSE);

				std::shared_ptr<GTexture> texture;

				if (textureCount > 0)
				{
					texture = LoadTextureByAiMaterial(aiMaterial.get(), aiTextureType_DIFFUSE, modelDirectory,
						cmdList);
				}
				else
				{
					texture = LoadTextureByPath(L"seamless", L"Data\\Textures\\seamless_grass.jpg", cmdList);
				}

				loadedTexturesForMesh[nativeMesh].push_back(texturesMap[texture->GetName()]);

				material->SetMaterialMap(Material::BaseColor, texture);

				textureCount = aiMaterial->GetTextureCount(aiTextureType_HEIGHT);

				if (textureCount > 0)
				{
					texture = LoadTextureByAiMaterial(aiMaterial.get(), aiTextureType_HEIGHT, modelDirectory,
						cmdList);
				}
				else
				{
					textureCount = aiMaterial->GetTextureCount(aiTextureType_NORMALS);

					if (textureCount > 0)
					{
						texture = LoadTextureByAiMaterial(aiMaterial.get(), aiTextureType_NORMALS, modelDirectory,
							cmdList);
					}
					else
					{
						texture = LoadTextureByPath(L"defaultNormalMap", L"Data\\Textures\\default_nmap.dds",
						                            cmdList);
					}
				}

				loadedTexturesForMesh[nativeMesh].push_back(texturesMap[texture->GetName()]);
				material->SetMaterialMap(Material::NormalMap, texture);
				AddMaterial(material);
			}
			else
			{
				material = materials[it->second];
			}

			model->SetMeshMaterial(i, material);
		}*/
	}

	std::shared_ptr<NativeMesh> AssimpModelLoader::CreateSubMesh(aiMesh* mesh,
	                                                             const std::wstring& modelName)
	{
		auto vertices = GetVertices(mesh);
		auto indices = GetIndices(mesh);

		RecalculateTangent(indices.data(), indices.size(), vertices.data());

		auto nativeMesh = std::make_shared<NativeMesh>(vertices.data(), vertices.size(), indices.data(),
		                                               indices.size(),
		                                               D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		                                               modelName + L" " + AnsiToWString(mesh->mName.C_Str()));
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
		const std::shared_ptr<GCommandList>& cmdList, const std::string& filePath)
	{
		const aiScene* sceneModel = importer.ReadFile(filePath,
		                                              aiProcess_Triangulate | aiProcess_GenNormals |
		                                              aiProcess_ConvertToLeftHanded);

		assert(sceneModel != nullptr && "Model Path dosen't exist or wrong file");

		auto modelFromFile = std::make_shared<NativeModel>(AnsiToWString(filePath));

		RecursivlyLoadMeshes(modelFromFile, sceneModel->mRootNode, sceneModel);

		auto renderModel = std::make_shared<GModel>(modelFromFile, cmdList);

		return renderModel;
	}
}
