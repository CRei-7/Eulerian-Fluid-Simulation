#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>

#include "Window.h"
#include "Shader.h"
#include "Fluid.h"

//For Nvidia GPU
extern "C" {
	__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
}

Window mainWindow;

const unsigned int SCR_WIDTH = 1080;
const unsigned int SCR_HEIGHT = 720;

// Vertex Shader
static const char* vShader = "./shader.vert";
static const char* fShader = "./shader.frag";

std::vector<Shader> shaderList;

const char* glsl_version = "#version 430";

const unsigned int xCount = 16;
const unsigned int yCount = 16;

float density = 1000.0f;
float deltaTime = 1.0f / 60.0f;

Fluid fluid(xCount, yCount, 1.0f / xCount, density, deltaTime);

void CreateShaders() {
	Shader* shaderProgram = new Shader();
	shaderProgram->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shaderProgram);
}

int main() {
	mainWindow = Window(SCR_WIDTH, SCR_HEIGHT);
	mainWindow.initialize();

	CreateShaders();

	float quadVertices[] = {
		-1.0f, -1.0f, 0.0f, 0.0f,
		1.0f, -1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 0.0f, 1.0f
	};

	unsigned int quadIndex[] = { 0, 1, 2, 2, 3, 0 };
	
	unsigned int VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndex), quadIndex, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2*sizeof(float)));
	glEnableVertexAttribArray(1);

	unsigned int solidTex;
	glGenTextures(1, &solidTex);
	glBindTexture(GL_TEXTURE_2D, solidTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	std::vector<unsigned char> solidData(xCount * yCount);
	const bool* solidPtr = fluid.GetSolidData();
	for (unsigned int i = 0; i < xCount * yCount; i++)
		solidData[i] = solidPtr[i] ? 255 : 0; // Stores values for single channel data, in our case Red

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // safety for tight-packed single-channel rows
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, xCount, yCount, 0, GL_RED, GL_UNSIGNED_BYTE, solidData.data());

	while (!mainWindow.getShouldClose()) {
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shaderList[0].UseShader();
		glUniform1i(glGetUniformLocation(shaderList[0].GetShaderID(), "uXCount"), xCount);
		glUniform1i(glGetUniformLocation(shaderList[0].GetShaderID(), "uYCount"), yCount);
		glUniform1f(glGetUniformLocation(shaderList[0].GetShaderID(), "uLineWidth"), 0.02f);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, solidTex);
		glUniform1i(glGetUniformLocation(shaderList[0].GetShaderID(), "uSolidMask"), 0);

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		mainWindow.swapBuffers();
		glfwPollEvents();
	}
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteTextures(1, &solidTex);

	shaderList[0].ClearShader();

	glfwTerminate();
	return 0;
}