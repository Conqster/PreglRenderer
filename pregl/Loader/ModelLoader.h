#pragma once

#include "Core/NonCopyable.h"
#include "Renderer/GPUVertexData.h"

#include <string>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>




struct LoadedMesh
{
	std::string name;
	RenderableMesh mesh;
	glm::mat4 transform;
};

class ModelLoader : public NonCopyable
{
public:
	ModelLoader() = default;

	RenderableMesh LoadAsSingleMesh(const std::string& path, bool flip_uv = false);
	std::vector<LoadedMesh> LoadAsCollectionMeshes(const std::string& path, bool flip_uv = false, bool bake_transform = false);

private:
	std::string mCurrModelDir = "";

	glm::mat4 AiMatrixToGLM(const aiMatrix4x4& from) const;
	std::vector<LoadedMesh> ProcessNode(aiNode* node, const aiScene* scene, const aiMatrix4x4 parent_tranform, bool bake_transform);

	using MeshVerticesData = std::tuple<std::vector<Vertex>, std::vector<unsigned int>>;
	MeshVerticesData ProcessMesh(aiMesh* mesh, const aiScene* scene, const aiMatrix4x4 transform);
	MeshVerticesData ProcessNodeSingleMesh(aiNode* node, const aiScene* scene, const aiMatrix4x4 parent_tranform, bool bake_transform);

	//Helpers
	std::vector<Vertex> CalcAverageNormalsWcIndices(std::vector<Vertex>& vertices, std::vector<unsigned int> indices);

}; //# ModelLoader