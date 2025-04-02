#pragma once

#include "../NonCopyable.h"
#include "Shader.h"

//#define QUICK_DEGUB_LINE

class DebugGizmosRenderer : public NonCopyable 
{
public:
	DebugGizmosRenderer() {
		Initialise();
	}
	~DebugGizmosRenderer() = default;

	bool Initialise();

	void DrawLine(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& colour);
	void DrawWireTriangle(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, const glm::vec3& colour);
	//void DrawTriangle(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec4 colour, bool cull_face = true);

	void SendBatchesToGPU(class Camera& cam_ref, float aspect_ratio);

	float GetLineWidth() const { return mLineWidth; }
	void SetLineWidth(float value) {
		if (!this){
			printf("Failed to set debug gizmos renderer line width, memory error\n");
			return;
		}
		mLineWidth = value;
	}

private:
	Shader mShader;
	float mLineWidth = 1.0f;
#pragma region HelperStructure
	struct Line
	{
		float start[3]{ 0.0f }; float startColour[4]{ 0.0f };  // <--- vertex0
		float end[3]{ 0.0f }; float endColour[4]{ 0.0f };	   // <--- vertex1
	};
	struct Triangle
	{
		float v0[3]{ 0.0f }; float v0Colour[4]{ 0.0f };			// <--- vertex0
		float v1[3]{0.0f}; float v1Colour[4]{ 0.0f };			// <--- vertex1
		float v2[3]{0.0f}; float v2Colour[4]{ 0.0f };			// <--- vertex2
	};

	struct LineSegmentVertex
	{
		unsigned int VAO = 0;
		unsigned int VBO = 0;

		unsigned int vertexCount = 1000;
	};

#pragma endregion
	//probably have a pointer to this to have multiple debug giamos renderer but share GPU data{vertex & shader program)
	LineSegmentVertex mLineSegVertex {};
	std::vector<Line> mLineBatches;

};