#include "GPUVertexData.h"

#include "ErrorAssertion.h"

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
}





ShadowMap::ShadowMap(unsigned int in_size) : mSize(in_size)
{
	Generate();
}

void ShadowMap::Generate()
{
	GLCall(glGenFramebuffers(1, &mID));

	GLCall(glGenTextures(1, &mColourAttachment));
	GLCall(glBindTexture(GL_TEXTURE_2D, mColourAttachment));
	GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, mSize, mSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));
	float borderColour[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLCall(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColour));

	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, mID));
	GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mColourAttachment, 0));
	GLCall(glDrawBuffer(GL_NONE));
	GLCall(glReadBuffer(GL_NONE));


	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("[FRAMEBUFFER ERROR]: Shadow map FBO did not complete!!!!\n");
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
		return;
	}

	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void ShadowMap::Write()
{
	glViewport(0, 0, mSize, mSize);
	glBindFramebuffer(GL_FRAMEBUFFER, mID);
}

void ShadowMap::Read(unsigned int slot)
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, mColourAttachment);
}

void ShadowMap::Destroy()
{
	GLCall(glDeleteFramebuffers(1, &mID));
	GLCall(glDeleteTextures(1, &mColourAttachment));
	mID = 0;
	mColourAttachment = 0;
}
