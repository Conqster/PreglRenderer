#include "GPUVertexData.h"

#include "GLErrorAssertion.h"
#include "Core/Log.h"

RenderableMesh::RenderableMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
{
	Create(vertices, indices);
}

void RenderableMesh::Create(std::vector<Vertex> vertices, std::vector<unsigned int> indices)
{
	GLCall(glGenVertexArrays(1, &mVAO));
	GLCall(glBindVertexArray(mVAO)); 

	//vertex buffer
	GLCall(glGenBuffers(1, &mVBO));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, mVBO));
	GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), vertices.data(), GL_STATIC_DRAW));


	//index buffer
	GLCall(glGenBuffers(1, &mIBO));
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO));
	m_IndiceCount = (unsigned int)indices.size();
	GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * m_IndiceCount, indices.data(), GL_STATIC_DRAW));

	GLCall(glBindVertexArray(mVAO));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, mVBO));


	//BUFFER LAYOUT
	unsigned int offset_ptr = 0;
	GLsizei stride = 3 * sizeof(float) + //POS
					 3 * sizeof(float) +  //NOR
					 2 * sizeof(float) + //UV
					 3 * sizeof(float);  //COL

	//pos
	GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (const void*)offset_ptr));
	GLCall(glEnableVertexAttribArray(0));
	offset_ptr += 3 * sizeof(float);//pos

	//nor
	GLCall(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)offset_ptr));
	GLCall(glEnableVertexAttribArray(1));
	offset_ptr += 3 * sizeof(float);//nor

	//uv
	GLCall(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)offset_ptr));
	GLCall(glEnableVertexAttribArray(2));
	offset_ptr += 2 * sizeof(float);//uv

	//col
	GLCall(glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)offset_ptr));
	GLCall(glEnableVertexAttribArray(3));


	//unbind so other new buffer would link it this
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	DEBUG_LOG_STATUS("Created New Renderable Mesh, for GPU Vertex Data");
}

void RenderableMesh::Bind()
{
	GLCall(glBindVertexArray(mVAO));
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO));
}

void RenderableMesh::UnBind()
{
	GLCall(glBindVertexArray(0));
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

void RenderableMesh::Draw()
{
	GLCall(glBindVertexArray(mVAO));
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO));
	GLCall(glDrawElements(GL_TRIANGLES, m_IndiceCount, GL_UNSIGNED_INT, (void*)0));
}

void RenderableMesh::DrawOutline()
{
	GLCall(glBindVertexArray(mVAO));
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO));
	GLCall(glDrawElements(GL_LINE_LOOP, m_IndiceCount, GL_UNSIGNED_INT, (void*)0));
}

void RenderableMesh::Clear()
{
	glDeleteBuffers(1, &mVBO);
	glDeleteBuffers(1, &mIBO);
	glDeleteVertexArrays(1, &mVAO);
	DEBUG_LOG_STATUS("Destroyied Renderable Mesh");
}

