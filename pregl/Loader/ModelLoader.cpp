#include "ModelLoader.h"
#include "Core/Util.h"
#include "Core/Log.h"

RenderableMesh ModelLoader::LoadAsSingleMesh(const std::string& path, bool flip_uv)
{
    Assimp::Importer importer; 
	int process_step = aiProcess_Triangulate;
	if (flip_uv)
		process_step |= aiProcess_FlipUVs;

	//aiProcess_PreTransformVertices transform to a single object
	process_step |= aiProcess_PreTransformVertices;

	const aiScene* scene = importer.ReadFile(path, process_step);

	if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode)
	{
		DEBUG_LOG_STATUS("[ASSIMP ERROR]: ", importer.GetErrorString());
		return {};
	}

	mCurrModelDir = path.substr(0, path.find_last_of('/'));

	DEBUG_LOG_STATUS("[ASSIMP - Load as Single Mesh]: Loading Model, path: ", mCurrModelDir);
	auto [vertices, indices] = ProcessNodeSingleMesh(scene->mRootNode, scene, aiMatrix4x4(), false);
	return RenderableMesh(vertices, indices);
}

std::vector<LoadedMesh> ModelLoader::LoadAsCollectionMeshes(const std::string& path, bool flip_uv, bool bake_transform)
{
	Assimp::Importer importer;
	int process_step = aiProcess_Triangulate;
	if (flip_uv)
		process_step |= aiProcess_FlipUVs;


	//aiProcess_PreTransformVertices transform to a single object
	//process_step |= aiProcess_PreTransformVertices;

	const aiScene* scene = importer.ReadFile(path, process_step);

	if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode)
	{
		DEBUG_LOG_STATUS("[ASSIMP ERROR]: ", importer.GetErrorString());
		return {};
	}

	mCurrModelDir = path.substr(0, path.find_last_of('/'));

	DEBUG_LOG_STATUS("[ASSIMP - Load as Collection Meshe]: Loading Model, path: ", mCurrModelDir);
	return ProcessNode(scene->mRootNode, scene, aiMatrix4x4(), bake_transform);
}


/// <summary>
/// Convertion to GLM has a column-major 
/// </summary>
/// <param name="from"></param>
/// <returns></returns>
glm::mat4 ModelLoader::AiMatrixToGLM(const aiMatrix4x4& m) const
{
	return glm::mat4(m.a1, m.b1, m.c1, m.d1,
					 m.a2, m.b2, m.c2, m.d2,
					 m.a3, m.b3, m.c3, m.d3,
					 m.a4, m.b4, m.c4, m.d4);
}

std::vector<LoadedMesh> ModelLoader::ProcessNode(aiNode* node, const aiScene* scene, const aiMatrix4x4 parent_transform, bool bake_transform)
{

	DEBUG_LOG_STATUS("[PROCESS NODE] Mesh count: ", node->mNumMeshes, " from ", mCurrModelDir);
	DEBUG_LOG_STATUS("[PROCESS NODE] Mesh children count: ", node->mNumChildren, " from ", mCurrModelDir);

	std::vector<LoadedMesh> meshes;
	aiMatrix4x4 local_trans = parent_transform;
	//if (node->mNumMeshes > 0)
	local_trans *= node->mTransformation;

	//local_trans = node->mTransformation;
	//process all node in current node
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];


		//load as mesh {vertices} offsets from rootnode location
		//i.e vertices are transformed, but not its mesh itself 
		//else loads mesh vertices to be determined with origin 0,0,0
		//but the mesh global tranform would be the nodes transform.
		// 
		//load as mesh {vertices} offsets from rootnode location
		//i.e vertices are transformed, but not its mesh itself 
		aiMatrix4x4 smaple_local_transform = (bake_transform) ? local_trans : aiMatrix4x4();
		auto [vertices, indices] = ProcessMesh(mesh, scene, smaple_local_transform);
		LoadedMesh new_mesh
		{
			node->mName.C_Str(),
			RenderableMesh(vertices, indices),
			(bake_transform) ? glm::mat4(1.0f) : AiMatrixToGLM(local_trans)
		};

		meshes.push_back(new_mesh);
	}

	//transverse through current nodes children (recursively)
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		std::vector<LoadedMesh> children_meshes = ProcessNode(node->mChildren[i], scene, local_trans, bake_transform);
		meshes.insert(meshes.end(), children_meshes.begin(), children_meshes.end());
	}

	DEBUG_LOG_STATUS("[PROCESS NODE]: node process complete.");
	return meshes;
}



ModelLoader::MeshVerticesData ModelLoader::ProcessNodeSingleMesh(aiNode* node, const aiScene* scene, const aiMatrix4x4 parent_transform, bool bake_transform)
{

	//cache vertices & indices
	std::vector<Vertex> cache_vertices;
	std::vector<unsigned int> cache_indices;



	aiMatrix4x4 local_trans = parent_transform * node->mTransformation;

	//local_trans = node->mTransformation;
	//process all node in current node
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

		//load as mesh {vertices} offsets from rootnode location
		//i.e vertices are transformed, but not its mesh itself 
		//else loads mesh vertices to be determined with origin 0,0,0
		//but the mesh global tranform would be the nodes transform.
		// 
		//load as mesh {vertices} offsets from rootnode location
		//i.e vertices are transformed, but not its mesh itself 
		aiMatrix4x4 sample_local_transform = (bake_transform) ? local_trans : aiMatrix4x4();
		auto [vertices, indices] = ProcessMesh(mesh, scene, sample_local_transform);
		int offset = cache_vertices.size();
		for (auto& i : indices)
			i += offset;
		cache_vertices.insert(cache_vertices.end(), vertices.begin(), vertices.end());
		cache_indices.insert(cache_indices.end(), indices.begin(), indices.end());
	}

	//transverse through current nodes children (recursively)
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		auto [vertices, indices] = ProcessNodeSingleMesh(node->mChildren[i], scene, local_trans, bake_transform);
		int offset = cache_vertices.size();
		for (auto& i : indices)
			i += offset;
		cache_vertices.insert(cache_vertices.end(), vertices.begin(), vertices.end());
		cache_indices.insert(cache_indices.end(), indices.begin(), indices.end());
	}
	return MeshVerticesData{ cache_vertices, cache_indices };
}


ModelLoader::MeshVerticesData ModelLoader::ProcessMesh(aiMesh* mesh, const aiScene* scene, const aiMatrix4x4 transform)
{
	//data from current mesh
	std::vector<Vertex> temp_vertices;
	std::vector<unsigned int> temp_indices;


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
		vn = (mesh->HasNormals()) ? mesh->mNormals[i] : aiVector3D();

		//transform pos and nor
		vp = transform * vp;
		if (mesh->HasNormals())
		{
			aiMatrix3x3 nor_mat = aiMatrix3x3(transform);
			nor_mat = nor_mat.Inverse().Transpose();
			vn = nor_mat * mesh->mNormals[i];
		}

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
			temp_indices.push_back(face.mIndices[j]);
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


	DEBUG_LOG_STATUS("[PROCESS MESH]: Batched vertices & indices to Single Mesh groups.");
	return MeshVerticesData{ temp_vertices, temp_indices };
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

