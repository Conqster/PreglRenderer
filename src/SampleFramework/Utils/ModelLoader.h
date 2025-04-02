#pragma once

#include "../NonCopyable.h"
#include "Renderer/GPUVertexData.h"

#include <string>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


class ModelLoader : public NonCopyable
{
public:
	ModelLoader() = default;

	RenderableMesh LoadAsSingleMesh(const std::string& path, bool flip_uv = false);

private:
	std::string mCurrModelDir = "";

	std::vector<Vertex> mGroupedVertices;
	std::vector<unsigned> mGroupedIndices;
	RenderableMesh ProcessNodeForSingleMesh(aiNode* node, const aiScene* scene);
	void ProcessMeshAndBatchData(aiMesh* mesh, const aiScene* scene);



	//Helpers
	std::vector<Vertex> CalcAverageNormalsWcIndices(std::vector<Vertex>& vertices, std::vector<unsigned int> indices);

}; //# ModelLoader