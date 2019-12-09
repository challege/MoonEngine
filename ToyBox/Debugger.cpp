#include <vector>

#include "Debugger.h"
#include "SceneMgr.h"
#include "Model.h"

namespace MOON {

	void DEBUG::DrawBBox(const Model* object, const Vector4 &color, const float &lineWidth) {
		std::vector<Vector3> corners;
		
		object->bbox_world.GetCorners(&corners);
		DebugLine(corners[0], corners[1], color, lineWidth);
		DebugLine(corners[1], corners[2], color, lineWidth);
		DebugLine(corners[2], corners[3], color, lineWidth);
		DebugLine(corners[3], corners[0], color, lineWidth);

		DebugLine(corners[4], corners[5], color, lineWidth);
		DebugLine(corners[5], corners[6], color, lineWidth);
		DebugLine(corners[6], corners[7], color, lineWidth);
		DebugLine(corners[7], corners[4], color, lineWidth);

		DebugLine(corners[0], corners[6], color, lineWidth);
		DebugLine(corners[5], corners[3], color, lineWidth);
		DebugLine(corners[2], corners[4], color, lineWidth);
		DebugLine(corners[7], corners[1], color, lineWidth);
	}

	void DEBUG::DebugLine(const Vector3 &start, const Vector3 &end, const Vector4 &color, const float &lineWidth) {
		// configure shader
		MOON_ShaderManager::lineShader->use();
		MOON_ShaderManager::lineShader->setVec4("lineColor", color);
		MOON_ShaderManager::lineShader->setMat4("model", Matrix4x4());
		MOON_ShaderManager::lineShader->setMat4("view", MOON_CurrentCamera->view);
		MOON_ShaderManager::lineShader->setMat4("projection", MOON_CurrentCamera->projection);

		std::vector<float> data;
		data.push_back(start[0]); data.push_back(start[1]); data.push_back(start[2]);
		data.push_back(end[0]); data.push_back(end[1]); data.push_back(end[2]);

		// vertex array object
		unsigned int VAO;
		glGenVertexArrays(1, &VAO);
		// vertex buffer object
		unsigned int VBO;
		glGenBuffers(1, &VBO);
		// bind buffers
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		// line width
		glLineWidth(lineWidth);
		// copy data
		glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
		// vertex data format
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glEnableVertexAttribArray(0);
		// unbind buffers
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDrawArrays(GL_LINES, 0, data.size() / 3);
		glBindVertexArray(0);
		// delete buffer object
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
	}

}