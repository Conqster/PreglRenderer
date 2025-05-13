#pragma once
#include <vector>



struct Vertex
{
	float position[3];
	float normal[3];   
	float uv[2];
	float colour[3];   //<-----for debug/visuals
};

class RenderableMesh
{
private:
	unsigned int mVAO = 0;
	unsigned int mVBO = 0;
	unsigned int mIBO = 0;
public:
	RenderableMesh() = default;

	RenderableMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
	void Create(std::vector<Vertex> vertices, std::vector<unsigned int> indices);

	void Bind();
	void UnBind();

	void Draw() const;
	void DrawOutline();

	void Clear();
	~RenderableMesh() = default;

private:
	unsigned int m_IndiceCount = 0;
};



#include <GLM/glm/glm.hpp>
//since multiple object could share Mesh 
struct RenderableEntity
{
	RenderableMesh* renderableMesh = nullptr;
	glm::mat4 transform = glm::mat4(1.0f);
	bool solidRender = true;
	bool canCastShadow = true;

	glm::vec4 colour = glm::vec4(1.0f);
	bool plainTexture = false;
};



