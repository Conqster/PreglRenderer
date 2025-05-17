#include "DebugGizmosRenderer.h"
#include "Core/EditorCamera.h"
#include "Core/Log.h"


bool DebugGizmosRenderer::Initialise()
{
	if (!this)
	{
		DEBUG_LOG_ERROR("Failed to initialise debug gizmos renderer, memory error");
		return false;
	}


	//quick hack 
	if (mLineSegVertex.VAO)
	{
		DEBUG_LOG_STATUS("Initilaising Debug Gizmos Renderer, Clearing Old GPU Data.");
		mShader.Clear();

		glDeleteBuffers(1, &mLineSegVertex.VBO);
		glDeleteVertexArrays(1, &mLineSegVertex.VAO);

	}

    bool success = false;
    success = mShader.Create("debug-shader",
								PGL_ASSETS_PATH"/shaders/debug/batchLines.vert", //vertex shader
								PGL_ASSETS_PATH"/shaders/debug/batchLines.frag"); //fragment shader

	//New Line Segment
	glGenVertexArrays(1, &mLineSegVertex.VAO);
	//vertex buffer
	glGenBuffers(1, &mLineSegVertex.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, mLineSegVertex.VBO);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(Line), nullptr, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Line) * mLineSegVertex.vertexCount, nullptr, GL_DYNAMIC_DRAW);
	glBindVertexArray(mLineSegVertex.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, mLineSegVertex.VBO);

	GLsizei layout_stride = sizeof(Line) / 2;  // <-- size of a vertex(since there are two do divided by 2)
	//GLsizei layout_stride = sizeof(NewLine::p0);
	//unsigned int layout_offset = 0;
	size_t layout_offset = 0;

	//start point
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, layout_stride, (const void*)layout_offset);
	layout_offset += 3 * sizeof(float); //start point
	//layout_offset += sizeof(Line::start);

	//start colour
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, layout_stride, (const void*)layout_offset);
	layout_offset += 4 * sizeof(float); // start colour



	mLineBatches.reserve(20);

    return success;
}

void DebugGizmosRenderer::DrawLine(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& colour)
{
	if (!this)
	{
		DEBUG_LOG_ERROR("Failed to use debug gizmos renderer, memory error");
		return;
	}
	//struct Line
	//{
	//	float start[3]; float startColour[4];  // <--- vertex0
	//	float end[3]; float endColour[4];	   // <--- vertex1
	//};
	Line line = {
		v0.x, v0.y, v0.z, colour.x, colour.y, colour.z, colour.z,
		v1.x, v1.y, v1.z, colour.x, colour.y, colour.z, colour.z,
	};
	mLineBatches.push_back(line);
}

void DebugGizmosRenderer::DrawWireTriangle(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, const glm::vec3& colour)
{
	if (!this)
	{
		DEBUG_LOG_ERROR("Failed to use debug gizmos renderer, memory error");
		return;
	}
	DrawLine(v1, v2, colour);
	DrawLine(v1, v3, colour);
	DrawLine(v2, v3, colour);
}


void DebugGizmosRenderer::SendBatchesToGPU(EditorCamera& cam_ref, float aspect_ratio)
{
	if (!this)
	{
		DEBUG_LOG_ERROR("Failed to use debug gizmos renderer, memory error");
		return;
	}

	if (mLineBatches.size() <= 0)
		return;

	mShader.Bind();
	mShader.SetUniformMat4("uProj", cam_ref.ProjMat(aspect_ratio));
	mShader.SetUniformMat4("uView", cam_ref.ViewMat());
	glBindVertexArray(mLineSegVertex.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, mLineSegVertex.VBO);

	unsigned int req_buffer_data_count = unsigned int(mLineBatches.size());
	if (mLineSegVertex.vertexCount < req_buffer_data_count)
	{
		mLineSegVertex.vertexCount = req_buffer_data_count;
		//reallocate buffer to accomodate buffer, based on if triangle size if larger than line or vice versa
		GLsizeiptr buffer_data_size = sizeof(Line) * mLineBatches.size();
		glBufferData(GL_ARRAY_BUFFER, buffer_data_size, nullptr, GL_DYNAMIC_DRAW);
	}

	//Draw batched Line segments
	if (mLineBatches.size() > 0)
	{
		//glBufferData(GL_ARRAY_BUFFER, sizeof(Line) * m_BatchLines.size(), m_BatchLines.data(), GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Line) * mLineBatches.size(), mLineBatches.data());

		if(mLineWidth != 1.0f)
			glLineWidth(mLineWidth);
		glDrawArrays(GL_LINES, 0, GLsizei(mLineBatches.size() * 2));		// <-- x2 as Line as 2 points) 
		mLineBatches.clear();
		if(mLineWidth > 0.0f)
			glLineWidth(1.0f);
	}

	glBindVertexArray(0);
	//unbind shader
	glUseProgram(0);
}

void DebugGizmosRenderer::SetLineWidth(float value)
{
	if (!this)
	{
		DEBUG_LOG_WARNING("Failed to set debug gizmos renderer line width, memory error");
		return;
	}
	mLineWidth = value;
}

void DebugGizmosRenderer::ClearData()
{
	mShader.Clear();

	glDeleteBuffers(1, &mLineSegVertex.VBO);
	glDeleteVertexArrays(1, &mLineSegVertex.VAO);

	mLineBatches.clear();
	DEBUG_LOG_STATUS("Cleared Debug Gizmos Renderer Data");
}
