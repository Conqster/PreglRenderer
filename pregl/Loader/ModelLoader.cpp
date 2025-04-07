#include "ModelLoader.h"
#include "Core/Util.h"
#include "Core/Log.h"

RenderableMesh ModelLoader::LoadAsSingleMesh(const std::string& path, bool flip_uv)
{
    Assimp::Importer importer; 
	int process_step = aiProcess_Triangulate;
	if (flip_uv)
		process_step |= aiProcess_FlipUVs;

	const aiScene* scene = importer.ReadFile(path, process_step);

	if (!scene || scene->mFlags && AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		DEBUG_LOG_STATUS("[ASSIMP ERROR]: ", importer.GetErrorString());
		return {};
	}

	mCurrModelDir = path.substr(0, path.find_last_of('/'));

	DEBUG_LOG_STATUS("[ASSIMP - Load as Single Mesh]: Loading Model, path: ", mCurrModelDir);
    return SingleMeshProcessing(scene->mRootNode, scene);
}

RenderableMesh ModelLoader::SingleMeshProcessing(aiNode* node, const aiScene* scene)
{
	DEBUG_LOG_STATUS("[PROCESS NODE]: For Single Mesh");
	BatchProcessNodeForSingleMesh(node, scene);
	if (mGroupedVertices.size() > 0 && mGroupedIndices.size())
	{
		RenderableMesh _mesh;
		_mesh.Create(mGroupedVertices, mGroupedIndices);
		DEBUG_LOG_STATUS("[ASSIMP - SINGLE MESH]: Single Mesh generated from Batches");
		mGroupedVertices.clear();
		mGroupedIndices.clear();
		return _mesh;
	}
	DEBUG_LOG_WARNING("[ASSIMP - SINGLE MESH]: No Mesh was generated");
	return {};
}

void ModelLoader::BatchProcessNodeForSingleMesh(aiNode* node, const aiScene* scene)
{
	DEBUG_LOG_STATUS("[PROCESS NODE] Mesh count: ", node->mNumMeshes, " from ", mCurrModelDir);
	DEBUG_LOG_STATUS("[PROCESS NODE] Mesh children count: ", node->mNumChildren, " from ", mCurrModelDir);


	//process all node in current node
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		ProcessMeshAndBatchData(mesh, scene);
		//temp_mesh.push_back(ProcessMesh(mesh, scene));
	}

	//transverse through current nodes children (recursively)
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		//std::vector<ModelMesh> temp = ProcessNodes(node->mChildren[i], scene);
		//temp_mesh.insert(temp_mesh.end(), temp.begin(), temp.end());
		BatchProcessNodeForSingleMesh(node->mChildren[i], scene);
	}

	DEBUG_LOG_STATUS("[PROCESS NODE]: node process complete.");
}

void ModelLoader::ProcessMeshAndBatchData(aiMesh* mesh, const aiScene* scene)
{
	//data from current mesh
	std::vector<Vertex> temp_vertices;
	std::vector<unsigned> temp_indices;


	aiVector3D vp;     //position
	aiVector3D vn;	   //normal
	aiVector2D st;     //texture coord

	//Later
	//glm::vec3 tn;   //tangent 
	//glm::vec3 bt;	  //bitangent

	////////////////////////////
	// VERTICES
	////////////////////////////
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		vp = mesh->mVertices[i];
		vn = aiVector3D();

		if (mesh->HasNormals())
			vn = mesh->mNormals[i];

		if (mesh->mTextureCoords[0])
		{
			st.x = mesh->mTextureCoords[0][i].x;
			st.y = mesh->mTextureCoords[0][i].y;
		}
		else
		{
			st = aiVector2D();
			st.x = vp.x;
			st.y = vp.z;

			st.y = glm::atan(vp.y, vp.z) / Util::PI * 0.5 + 0.5;
			st.x = vp.x;
		}


		//{ SET UP
		//	float position[4];
		//	float colour[4];
		//	float texCoord[2];
		//	float normals[3];
		//}

		Vertex vertex
		{
			{vp.x, vp.y, vp.z},    //layout 0 => pos
			{vn.x, vn.y, vn.z},       //layout 3 => nor
			{st.x, st.y},			   //layout 2 => uv
			{1.0f, 0.0f, 1.0f},	//layout 1 => col  //magenta for debugging
			//{0.0f, 0.0f, 0.0f},       //layout 4 => tan
			//{0.0f, 0.0f, 0.0f},       //layout 5 => bi tan
		};
		temp_vertices.push_back(vertex);
	}


	////////////////////////////
	// INDICES
	////////////////////////////
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
		{
			temp_indices.push_back(face.mIndices[j]);
		}
	}



	////////////////////////
	// (Quick Hack): if model doesnt have normal generate from face & its associate vertices
	////////////////////////
	if (!mesh->HasNormals())
	{
		bool generateWcFace = false;

		if (!generateWcFace)
			temp_vertices = CalcAverageNormalsWcIndices(temp_vertices, temp_indices);
		//else
		//	temp_vertices = CalcNormalsWcMeshFace(temp_vertices, mesh);

	}


	mGroupedVertices.insert(mGroupedVertices.end(), temp_vertices.begin(), temp_vertices.end());
	mGroupedIndices.insert(mGroupedIndices.end(), temp_indices.begin(), temp_indices.end());
	DEBUG_LOG_STATUS("[PROCESS MESH]: Batched vertices & indices to Single Mesh groups.");
}

std::vector<Vertex> ModelLoader::CalcAverageNormalsWcIndices(std::vector<Vertex>& vertices, std::vector<unsigned int> indices)
{
	std::vector<Vertex> temp = vertices;

	for (size_t i = 0; i < indices.size(); i += 3)
	{
		unsigned int in0 = indices[i];
		unsigned int in1 = indices[i + 1];
		unsigned int in2 = indices[i + 2];

		glm::vec3 v1;
		glm::vec3 v2;

		v1 = glm::vec3(temp[in1].position[0] - temp[in0].position[0],
			temp[in1].position[1] - temp[in0].position[1],
			temp[in1].position[2] - temp[in0].position[2]);

		v2 = glm::vec3(temp[in2].position[0] - temp[in0].position[0],
			temp[in2].position[1] - temp[in0].position[1],
			temp[in2].position[2] - temp[in0].position[2]);

		glm::vec3 nor = glm::cross(v1, v2);
		nor = glm::normalize(nor);

		//(accumulate) add current normally to all current vertices
		temp[in0].normal[0] += nor.x;
		temp[in0].normal[1] += nor.y;
		temp[in0].normal[2] += nor.z;

		temp[in1].normal[0] += nor.x;
		temp[in1].normal[1] += nor.y;
		temp[in1].normal[2] += nor.z;

		temp[in2].normal[0] += nor.x;
		temp[in2].normal[1] += nor.y;
		temp[in2].normal[2] += nor.z;
	}

	//re-normalizing
	for (size_t i = 0; i < temp.size(); i++)
	{
		glm::vec3 vec(temp[i].normal[0], temp[i].normal[1], temp[i].normal[2]);
		vec = glm::normalize(vec);

		if (glm::length(vec) > 0.0f)
			vec = glm::normalize(vec);

		temp[i].normal[0] = vec.x;
		temp[i].normal[1] = vec.y;
		temp[i].normal[2] = vec.z;
	}

	DEBUG_LOG_STATUS("[ASSIMP]: Calculated Average Normals with indices.");
	return temp;
}

