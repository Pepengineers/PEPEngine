#include "pch.h"
#include "AssetsLoader.h"
#include "ComputePSO.h"
#include "GCommandList.h"
#include "GCommandQueue.h"
#include "GeometryGenerator.h"
#include "GraphicPSO.h"
#include "GTexture.h"
#include "Material.h"
#include "GMesh.h"
#include "GModel.h"
#include "assimp/Importer.hpp"
#include "assimp/mesh.h"
#include "assimp/postprocess.h"

using namespace DXLib;

static Assimp::Importer importer;

std::shared_ptr<GModel> AssetsLoader::GenerateSphere(std::shared_ptr<GCommandList> cmdList, float radius, UINT sliceCount, UINT stackCount)
{
	auto it = models.find(L"sphere");
	if (it != models.end()) return  it->second;


	const auto meshDataIt = meshesData.find(L"sphere");
	if (meshDataIt == meshesData.end())
	{
		GeometryGenerator::MeshData sphere = geoGen.CreateSphere(radius, sliceCount, stackCount);
		auto meshData = std::make_shared<MeshData>(sphere.Vertices.data(), sphere.Vertices.size(), sphere.Indices32.data(), sphere.Indices32.size(), L"sphere");
		meshesData[meshData->GetName()] = meshData;
	}
	
	const auto sphereMesh = std::make_shared<GMesh>(*meshesData[L"sphere"].get(), cmdList, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	auto sphereModel = std::make_shared<GModel>(L"sphere");
	sphereModel->AddMesh(sphereMesh);
	models[sphereModel->GetName()] = std::move(sphereModel);	
	return models[L"sphere"];
}

std::shared_ptr<GModel> AssetsLoader::GenerateQuad(std::shared_ptr<GCommandList> cmdList, float x, float y, float w,
	float h, float depth)
{
	auto it = models.find(L"quad");
	if (it != models.end()) return  it->second;


	const auto meshDataIt = meshesData.find(L"quad");
	if (meshDataIt == meshesData.end())
	{
		GeometryGenerator::MeshData genMesh = geoGen.CreateQuad(x, y, w, h, depth);
		auto meshData = std::make_shared<MeshData>(genMesh.Vertices.data(), genMesh.Vertices.size(), genMesh.Indices32.data(), genMesh.Indices32.size(), L"quad");
		meshesData[meshData->GetName()] = meshData;
	}

	const auto mesh = std::make_shared<GMesh>(*meshesData[L"quad"].get(), cmdList, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);	
	auto model = std::make_shared<GModel>(L"quad");
	model->AddMesh(mesh);
	models[model->GetName()] = std::move(model);
	return models[L"quad"];
}


std::shared_ptr<GMesh> AssetsLoader::CreateSubMesh(aiMesh* mesh, std::wstring name, std::shared_ptr<GCommandList> cmdList)
{
	// Data to fill
	std::vector<Vertex> vertices;
	std::vector<DWORD> indices;

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

	//Get indices
	for (UINT i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];

		for (UINT j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	RecalculateTangent(indices.data(), indices.size(), vertices.data());

	auto meshName = name + L" " + AnsiToWString(mesh->mName.C_Str());
	
	auto meshDataIT = meshesData.find(meshName);
	if(meshDataIT == meshesData.end())
	{
		const auto meshData = std::make_shared<MeshData>(vertices.data(), vertices.size(), indices.data(), indices.size(), name + L" " + AnsiToWString(mesh->mName.C_Str()));
		meshesData[meshName] = std::move(meshData);
	}
	
	return std::make_shared<GMesh>(*meshesData[meshName].get(), cmdList, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

std::shared_ptr<GTexture> AssetsLoader::LoadOrGetTexture(const aiMaterial* material, const aiTextureType type,
                                                         const std::wstring directory,
                                                         std::shared_ptr<GCommandList> cmdList)
{
		
	aiString str;
	auto result = material->GetTexture(type, 0, &str);	

	std::wstring modelTexturePath(AnsiToWString(str.C_Str()));

	if(modelTexturePath.find(L"\\") != std::wstring::npos)
	{
		auto fileName = modelTexturePath.substr(modelTexturePath.find_last_of('\\'), modelTexturePath.size() - modelTexturePath.find_last_of('\\'));

		modelTexturePath = fileName.replace(fileName.find(L"\\"), 1, L"");
	}
	
	std::wstring textureName = modelTexturePath;
	std::wstring texturePath = directory + L"\\" + textureName;

	const auto it = texturesMap.find(textureName);
	if (it != texturesMap.end()) return textures[ it->second];
	
	OutputDebugStringW((texturePath + L"\n").c_str());

	auto texture = GTexture::LoadTextureFromFile(texturePath, cmdList, type == aiTextureType_DIFFUSE ? TextureUsage::Albedo : TextureUsage::Normalmap);
	texture->SetName(textureName);

	textures.push_back(std::move(texture));
	
	texturesMap[textureName] = textures.size() - 1;
	return textures[texturesMap[textureName]];
}

void  AssetsLoader::CreateMaterialForMesh(std::shared_ptr<GMesh> mesh, const aiMaterial* material, const std::shared_ptr<GCommandList> cmdList)
{
	aiString name;
	material->Get(AI_MATKEY_NAME, name);
	auto materialName = mesh->GetName() + L" " + AnsiToWString(name.C_Str());

	const auto it = materialsMap.find(materialName);
	if (it != materialsMap.end())
	{
		defaultMaterialForMeshFromFile[mesh->GetName()] = materials[it->second];		
		return;
	}


	
	auto directory = mesh->GetName().substr(0, mesh->GetName().find_last_of('\\'));

	auto count = material->GetTextureCount(aiTextureType_DIFFUSE);

	std::shared_ptr<GTexture> diffuse;

	if(count > 0)
	{
		diffuse = LoadOrGetTexture(material, aiTextureType_DIFFUSE, directory, cmdList);
	}
	else
	{

		const auto textureIt = texturesMap.find(L"seamless");
		if (textureIt != texturesMap.end())
		{
			diffuse = textures[textureIt->second];
		}
		else
		{
			auto defauldAlbedo = GTexture::LoadTextureFromFile(L"Data\\Textures\\seamless_grass.jpg", cmdList,
				TextureUsage::Diffuse);
			defauldAlbedo->SetName(L"seamless");

			textures.push_back((defauldAlbedo));			
			texturesMap[defauldAlbedo->GetName()] = textures.size() - 1;

			diffuse = textures.back();
		}		
	}

	count = material->GetTextureCount(aiTextureType_HEIGHT);

	std::shared_ptr<GTexture> normal;
	
	if (count > 0)
	{
		normal = LoadOrGetTexture(material, aiTextureType_HEIGHT, directory, cmdList);
	}
	else
	{
		count = material->GetTextureCount(aiTextureType_NORMALS);

		if(count > 0)
		{
			normal = LoadOrGetTexture(material, aiTextureType_NORMALS, directory, cmdList);
		}
		else {

			const auto textureIt = texturesMap.find(L"defaultNormalMap");
			if (textureIt != texturesMap.end())
			{
				normal = textures[textureIt->second];
			}
			else
			{
				auto defaultNormal = GTexture::LoadTextureFromFile(L"Data\\Textures\\default_nmap.dds", cmdList,
					TextureUsage::Normalmap);
				defaultNormal->SetName(L"defaultNormalMap");

				textures.push_back((defaultNormal));
				
				texturesMap[defaultNormal->GetName()] = textures.size() - 1;

				normal = textures.back();
			}
		}
	}	 


	
	auto mat = std::make_shared<Material>(materialName);
	mat->SetDiffuseTexture(diffuse, texturesMap[diffuse->GetName()]);
	mat->SetNormalMap(normal, texturesMap[normal->GetName()]);
	mat->FresnelR0 = Vector3::One * 0.05;
	mat->Roughness = 0.95;

	AddMaterial(mat);
	defaultMaterialForMeshFromFile[mesh->GetName()] = mat;
}


void  AssetsLoader::RecursivlyLoadMeshes(std::shared_ptr<GModel> model, aiNode* node, const aiScene* scene, const std::shared_ptr<GCommandList> cmdList)
{
	for (UINT i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* aMesh = scene->mMeshes[node->mMeshes[i]];				
		
		auto mesh = CreateSubMesh(aMesh, model->GetName(), cmdList);
		CreateMaterialForMesh(mesh, scene->mMaterials[aMesh->mMaterialIndex], cmdList);
		meshes.push_back(std::move(mesh));
		model->AddMesh(meshes.back());
		
	}

	for (UINT i = 0; i < node->mNumChildren; i++)
	{
		RecursivlyLoadMeshes(model, node->mChildren[i], scene, cmdList);
	}
}

AssetsLoader::AssetsLoader(const std::shared_ptr<GDevice> device): device(device)
{
}

void AssetsLoader::SetModelDefaultMaterial(std::shared_ptr<GMesh> mesh, std::shared_ptr<Material> material)
{
	defaultMaterialForMeshFromFile[mesh->GetName()] = material;
}

std::shared_ptr<GMesh> AssetsLoader::GetMesh(std::wstring name)
{
	for (auto&& mesh : meshes)
	{
		if (mesh->GetName()._Equal(name))
		{
			return mesh;
		}
		
	}
}

UINT AssetsLoader::GetTextureIndex(std::wstring name)
{
	auto it = texturesMap.find(name);
	if (it == texturesMap.end())
	{
		return -1;
	}
	return it->second;
}

UINT AssetsLoader::GetMaterialIndex(std::wstring name)
{
	auto it = materialsMap.find(name);
	if (it == materialsMap.end())
	{
		return -1;
	}
	return it->second;
}

size_t AssetsLoader::GetLoadTexturesCount() const
{
	return texturesMap.size();
}

void AssetsLoader::AddMaterial(std::shared_ptr<Material> material)
{	
	materialsMap[material->GetName()] = materials.size();
	materials.push_back((material));
}

void AssetsLoader::AddTexture(std::shared_ptr<GTexture> texture)
{
	
	texturesMap[texture->GetName()] = textures.size();
	textures.push_back((texture));
}

void AssetsLoader::AddModel(std::shared_ptr<GModel> model)
{
	models[model->GetName()] = model;
}

custom_vector<std::shared_ptr<Material>>& AssetsLoader::GetMaterials()
{
	return materials;
}


custom_vector<std::shared_ptr<GTexture>>& AssetsLoader::GetTextures()
{
	return textures;
}


std::shared_ptr<GTexture> AssetsLoader::GetTexture(UINT index)
{
	return textures[index];
}

std::shared_ptr<Material> AssetsLoader::GetMaterials(UINT index)
{
	return materials[index];
}

std::shared_ptr<Material> AssetsLoader::GetDefaultMaterial(std::shared_ptr<GMesh> mesh)
{
	return defaultMaterialForMeshFromFile[mesh->GetName()];
}

std::shared_ptr<GModel> AssetsLoader::GetModelByName(const std::wstring name)
{
	auto it = models.find((name));
	if (it != models.end())
	{
		return it->second;
	}

	return nullptr;
}


std::shared_ptr<GModel> AssetsLoader::GetOrCreateModelFromFile(std::shared_ptr<GCommandQueue> queue,
                                                              const std::string filePath)
{
	auto it = models.find(AnsiToWString(filePath));
	if (it != models.end()) return it->second;


	const aiScene* sceneModel = importer.ReadFile(filePath,
		 aiProcess_Triangulate   | aiProcess_GenNormals |
	                                          aiProcess_ConvertToLeftHanded);

	assert(sceneModel != nullptr && "Model Path dosen't exist or wrong file");	

	auto model = std::make_shared<GModel>(AnsiToWString(filePath));

	const auto cmdList = queue->GetCommandList();
		
	RecursivlyLoadMeshes(model, sceneModel->mRootNode, sceneModel, cmdList);

	queue->WaitForFenceValue(queue->ExecuteCommandList(cmdList));

	models[model->GetName()] = std::move(model);

	importer.FreeScene();
	
	return models[AnsiToWString(filePath)];	
}



void AssetsLoader::ClearTrackedObjects()
{
	trackGeneratedData.clear();	
}


